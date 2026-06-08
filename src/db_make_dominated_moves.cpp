/*
    TODO: are there insufficiencies with the sumgame interface? This file is
    full of hacks...

    TODO this code could be optimized to reduce copying:

    GSM = "generalized_sum_move"

    The initial function call should modify the sumgame passed to it to remove
    all inactive games (the GSMs produced must be compatible
    with both sums which is why the sumgame argument is cloned twice
    instead of once).

    Then clone the games once to make the 2nd sum, and generate all the
    inverse games once.

    Instead of playing the GSMs on the individual sums, maintain a single
    difference sum and manually deactivate/reactivate its games,
    computing the move update using a 4th "helper" sum and adding the games
    to the difference sum which result from playing GSMs
*/
#include "db_make_dominated_moves.h"

#include <memory>
#include <vector>
#include <algorithm>
#include <utility>
#include <sstream>
#include <cassert>
#include <unordered_set>
#include <cstdint>
#include <cstddef>
#include <functional>

#include "ThGraph.h"
#include "cgt_basics.h"
#include "database.h"
#include "dominated_moves.h"
#include "global_database.h"
#include "safe_arithmetic.h"
#include "sumgame.h"
#include "game.h"
#include "throw_assert.h"
#include "warn_on_exit.h"
#include "utilities.h"

// Maximum ratio of nondominated moves to total moves, above which dominated
// moves are stored instead of nondominated ones
#define MAX_NONDOMINATED_RATIO 0.7

using namespace std;


template<>
struct std::hash<std::pair<hash_t, ::move>>
{
    inline size_t operator()(const std::pair<hash_t, ::move>& p) const noexcept
    {
        return p.first ^ p.second;
    }
};

////////////////////////////////////////////////// helpers
namespace {

//////////////////////////////////////// class generalized_sum_move
class generalized_sum_move
{
public:
    inline generalized_sum_move(hash_t subgame_hash, ::move move_db_encoded,
                                const sumgame_move& sm)
        : subgame_hash(subgame_hash),
          move_db_encoded(move_db_encoded),
          sm(sm),
          db_entry(nullptr),
          thermograph(nullptr),
          _is_dominated(false)
    {
    }

    bool operator<(const generalized_sum_move& rhs) const;
    bool operator==(const generalized_sum_move& rhs) const;
    bool operator!=(const generalized_sum_move& rhs) const;

    hash_t subgame_hash;
    ::move move_db_encoded;

    sumgame_move sm;
    const db_entry_partisan* db_entry;
    const ThGraph* thermograph;

    inline bool is_dominated() const
    {
        return _is_dominated;
    }

    inline void mark_dominated()
    {
        _is_dominated = true;
    }

private:
    bool _is_dominated;
};

//////////////////////////////////////// generalized_sum_move methods
inline bool generalized_sum_move::operator<(
    const generalized_sum_move& rhs) const
{
    return (subgame_hash < rhs.subgame_hash) &&     //
           (move_db_encoded < rhs.move_db_encoded); //
}

inline bool generalized_sum_move::operator==(
    const generalized_sum_move& rhs) const
{
    return (subgame_hash == rhs.subgame_hash) &&       //
           (move_db_encoded == rhs.move_db_encoded) && //
           (sm == rhs.sm);                             //
}

inline bool generalized_sum_move::operator!=(const generalized_sum_move& rhs) const
{
    return !(*this == rhs);
}

////////////////////////////////////////
void add_generalized_sum_moves_to_dominated_moves_t(
    db_dom_moves_t& dom, const vector<generalized_sum_move>& sum_moves,
    bw player, db_dom_moves_kind dom_moves_kind)
{
    switch (dom_moves_kind)
    {
        case DB_DOM_MOVES_KIND_NONE:
            assert(false);
        case DB_DOM_MOVES_KIND_DOMINATED:
        {
            for (const generalized_sum_move& gsm : sum_moves)
                if (gsm.is_dominated())
                    dom.add_move(gsm.subgame_hash, player, gsm.move_db_encoded, dom_moves_kind);

            return;
        }
        case DB_DOM_MOVES_KIND_NONDOMINATED:
        {
            for (const generalized_sum_move& gsm : sum_moves)
                if (!gsm.is_dominated())
                    dom.add_move(gsm.subgame_hash, player, gsm.move_db_encoded, dom_moves_kind);

            return;
        }
    }
}

// relation is relative to player to play
relation bools_to_relation_relative(bool black_wins, bool white_wins, bw player)
{
    assert(is_black_white(player));

    relation rel = bools_to_relation(black_wins, white_wins);
    if (player == WHITE)
        rel = flip_relation(rel);

    return rel;
}

bool sum_cloned_properly(const sumgame& sum1, const sumgame& sum2)
{
    stringstream str1;
    stringstream str2;

    sum1.print_simple(str1);
    sum2.print_simple(str2);

    return str1.str() == str2.str();
}

void clone_sumgame(const sumgame& from, sumgame& to)
{
    assert(to.num_total_games() == 0);

    const int n_games = from.num_total_games();

    for (int i = 0; i < n_games; i++)
    {
        const game* sg = from.subgame_const(i);
        if (!sg->is_active())
            continue;

        game* sg_copy = sg->clone();

        to.add(sg_copy);
    }

    to.set_to_play(from.to_play());
    assert(sum_cloned_properly(from, to));
}

// Data to undo the appending of sum2 into sum1
struct subgame_append_info_t
{
    subgame_append_info_t() : sum1_n_total(0), sum2_n_active(0) {}

    int sum1_n_total;
    int sum2_n_active;
};

subgame_append_info_t append_inverse(sumgame& sum1, const sumgame& sum2)
{
    subgame_append_info_t append_info;
    append_info.sum1_n_total = sum1.num_total_games();

    const int n_games = sum2.num_total_games();

    for (int i = 0; i < n_games; i++)
    {
        const game* sg = sum2.subgame_const(i);
        if (!sg->is_active())
            continue;

        append_info.sum2_n_active++;
        game* sg_inverse = sg->inverse();
        sum1.add(sg_inverse);
    }

    return append_info;
}

void pop_inverse(sumgame& sum1, const subgame_append_info_t& append_info)
{
    assert(sum1.num_total_games() ==
           append_info.sum1_n_total + append_info.sum2_n_active);

    const int end = sum1.num_total_games();

    for (int i = 0; i < append_info.sum2_n_active; i++)
    {
        const int idx = end - 1 - i;
        assert(idx >= 0);

        game* g = sum1.subgame(idx);
        sum1.pop(g);
        delete g;
    }
}

void cleanup_sumgame(sumgame& cloned_sum)
{
    const int n_games = cloned_sum.num_total_games();

    for (int i = n_games - 1; i >= 0; i--)
    {
        game* sg = cloned_sum.subgame(i);
        cloned_sum.pop(sg);
        delete sg;
    }

    assert(cloned_sum.num_total_games() == 0);
}

// Relation is relative to `player`'s perspective, with ">" being better
// for `player`
relation compare_sums(sumgame& sum1, const sumgame& sum2, bw player)
{
    assert(is_black_white(player));

    assert_restore_sumgame ars(sum1);
    const bw restore_player = sum1.to_play();

    const subgame_append_info_t append_info = append_inverse(sum1, sum2);

    sum1.set_to_play(BLACK);
    const bool black_wins = sum1.solve();

    sum1.set_to_play(WHITE);
    const bool white_wins = sum1.solve();

    pop_inverse(sum1, append_info);
    sum1.set_to_play(restore_player);

    return bools_to_relation_relative(black_wins, white_wins, player);
}

// Outcome class rank for `player` (higher is better). Must be L/P/R
int get_outcome_ordinal(outcome_class oc, bw player)
{
    assert(is_black_white(player));

    // For BLACK: L > P > R
    switch (oc)
    {
        case outcome_class::R:
            return player == BLACK ? 1 : 3;
        case outcome_class::P:
            return 2;
        case outcome_class::L:
            return player == BLACK ? 3 : 1;
        default:
            assert(false);
    }
}

// true IFF can prune `gsm` (considering outcomes)
bool compare_outcomes(const generalized_sum_move& gsm,
                          int best_ordinal, bw player)
{
    assert(is_black_white(player));
    assert(gsm.db_entry != nullptr);

    const outcome_class oc = gsm.db_entry->outcome;

    if (oc == outcome_class::N)
        return false;

    const int ordinal = get_outcome_ordinal(oc, player);

    if (ordinal < best_ordinal)
        return true;

    return false;
}

// Relation is from `player`'s perspective. Might be REL_UNKNOWN
relation compare_thermographs(const generalized_sum_move& gsm1,
                              const generalized_sum_move& gsm2, bw player)
{
    assert(is_black_white(player));
    assert(gsm1.thermograph != nullptr && gsm2.thermograph != nullptr);

    const ThGraph& th1 = *gsm1.thermograph;
    const ThGraph& th2 = *gsm2.thermograph;

    relation rel = REL_UNKNOWN;

    if (th1.RightStop() > th2.LeftStop())
        rel = REL_GREATER;
    else if (th1.LeftStop() < th2.RightStop())
        rel = REL_LESS;

    if (player == WHITE)
        rel = flip_relation(rel);

    return rel;
}

vector<generalized_sum_move> make_generalized_sum_moves(sumgame& sum, bw player)
{
    assert(is_black_white(player));
    vector<generalized_sum_move> moves;

    assert_restore_sumgame ars(sum);
    const bw restore_player = sum.to_play();

    sum.set_to_play(player);

    /*
        IMPORTANT: Moves must be deduplicated, to avoid duplicates pruning
        themselves
    */
    unordered_set<pair<hash_t, ::move>> move_set;

    assert(sum.to_play() == player);
    unique_ptr<sumgame_move_generator> gen(sum.create_sum_move_generator(player));
    for (; *gen; ++(*gen))
    {
        const sumgame_move sm = gen->gen_sum_move();

        const game* sg = sum.subgame_const(sm.subgame_idx);
        assert(sg->is_active());

        const hash_t subgame_hash = sg->get_local_hash();
        const ::move move_db_encoded = sg->encode_grid_move_to_db(sm.m);

        const auto inserted = move_set.emplace(subgame_hash, move_db_encoded);

        if (!inserted.second)
            continue;
        
        moves.emplace_back(subgame_hash, move_db_encoded, sm);
    }

    std::sort(moves.begin(), moves.end());

    sum.set_to_play(restore_player);
    return moves;
}

void preprocess_generalized_sum_moves(sumgame& sum,
                                      vector<generalized_sum_move>& sum_moves,
                                      bw player, int& best_ordinal,
                                      database& db)
{
    assert(is_black_white(player));
    assert_restore_sumgame ars(sum);

    best_ordinal = -1;

    for (generalized_sum_move& gsm : sum_moves)
    {
        assert(gsm.db_entry == nullptr);

        assert(sum.to_play() == player);
        sum.play_sum(gsm.sm, player);

        gsm.db_entry = db.get_partisan_ptr(sum);
        assert(gsm.db_entry != nullptr);

        const outcome_class oc = gsm.db_entry->outcome;
        assert(oc != outcome_class::U);

        if (oc != outcome_class::N)
            best_ordinal = max(best_ordinal, get_outcome_ordinal(oc, player));

        const std::shared_ptr<const ThGraph> thermograph =
            gsm.db_entry->thermograph;
        THROW_ASSERT(thermograph.get() != nullptr);
        gsm.thermograph = thermograph.get();

        sum.undo_move();
    }
}

// sum1 and sum2 should be 2 clones of the same sumgame
void make_dominated_moves_for(sumgame& sum1, sumgame& sum2, bw player,
                              uint64_t& complexity,
                              vector<generalized_sum_move>& sum_moves,
                              database& db)
{
    complexity = 0;

    assert(is_black_white(player));
    assert_restore_sumgame ars1(sum1);
    assert_restore_sumgame ars2(sum2);

    const bw restore_player1 = sum1.to_play();
    const bw restore_player2 = sum2.to_play();

    sum1.set_to_play(player);
    sum2.set_to_play(player);

    // TODO put this back?
    //assert(sum_moves == make_generalized_sum_moves(sum2, player));

    bool sum1_played = false;
    bool sum2_played = false;

    auto play_gsm_if_not_played = [](sumgame& sum,
                                     const generalized_sum_move& gsm, bw player,
                                     bool& played) -> void
    {
        if (played)
            return;
        played = true;

        assert(sum.to_play() == player);
        sum.play_sum(gsm.sm, player);
    };

    auto undo_gsm_if_played = [](sumgame& sum, bw player, bool& played) -> void
    {
        if (!played)
            return;
        played = false;

        sum.undo_move();
        assert(sum.to_play() == player);
    };

    // Populate sum moves with DB entries, and find best outcome class
    int best_ordinal = -1;
    preprocess_generalized_sum_moves(sum1, sum_moves, player, best_ordinal, db);

    // Compare moves to find dominated ones
    const size_t n_moves = sum_moves.size();
    for (size_t idx1 = 0; idx1 < n_moves; idx1++)
    {
        assert(!sum1_played);
        generalized_sum_move& gsm1 = sum_moves[idx1];

        if (gsm1.is_dominated())
            continue;

        if (compare_outcomes(gsm1, best_ordinal, player))
        {
            gsm1.mark_dominated();
            continue;
        }

        for (size_t idx2 = idx1 + 1; idx2 < n_moves; idx2++)
        {
            assert(!sum2_played);
            generalized_sum_move& gsm2 = sum_moves[idx2];

            if (gsm1.is_dominated())
                break;
            if (gsm2.is_dominated())
                continue;

            if (compare_outcomes(gsm2, best_ordinal, player))
            {
                gsm2.mark_dominated();
                continue;
            }

            relation rel = REL_UNKNOWN;
            rel = compare_thermographs(gsm1, gsm2, player);

            if (rel == REL_UNKNOWN)
            {
                play_gsm_if_not_played(sum1, gsm1, player, sum1_played);
                play_gsm_if_not_played(sum2, gsm2, player, sum2_played);
                rel = compare_sums(sum1, sum2, player);
            }

            /*
                `rel` is relative to the current player, and compares gsm1
                to gsm2. `GREATER` means gsm1 > gsm2 (from the perspective
                of `player`)
            */
            if (rel == REL_LESS)
                gsm1.mark_dominated();
            else if (rel == REL_EQUAL)
            {
                if (gsm1.db_entry->complexity >= gsm2.db_entry->complexity)
                    gsm1.mark_dominated();
                else
                    gsm2.mark_dominated();
            }
            else if (rel == REL_GREATER)
                gsm2.mark_dominated();
            else
                assert(rel == REL_FUZZY);

            undo_gsm_if_played(sum2, player, sum2_played);
        }

        undo_gsm_if_played(sum1, player, sum1_played);
    }

    sum1.set_to_play(restore_player1);
    sum2.set_to_play(restore_player2);

    uint64_t n_immediate_nondom = 0;
    bool add_ok = true;

    for (const generalized_sum_move& gsm : sum_moves)
    {
        if (!gsm.is_dominated())
        {
            n_immediate_nondom++;
            const uint64_t move_complexity = gsm.db_entry->complexity;

            add_ok &= safe_add(complexity, move_complexity);
        }
    }

    add_ok &= safe_add(complexity, n_immediate_nondom);
    if (!add_ok)
        warn_on_exit::on_db_dom_moves_complexity_overflow();
}

} // namespace

//////////////////////////////////////////////////
void db_make_dominated_moves(const sumgame& sum, db_entry_partisan& entry, database& db)
{
    shared_ptr<db_dom_moves_t> dom(new db_dom_moves_t());

    sumgame clone1(BLACK);
    sumgame clone2(BLACK);
    clone_sumgame(sum, clone1);
    clone_sumgame(sum, clone2);

    uint64_t complexity_b = 0;
    uint64_t complexity_w = 0;

    vector<generalized_sum_move> black_moves = make_generalized_sum_moves(clone1, BLACK);
    vector<generalized_sum_move> white_moves = make_generalized_sum_moves(clone1, WHITE);

    {
        assert_restore_sumgame ars1(clone1);
        assert_restore_sumgame ars2(clone2);
        make_dominated_moves_for(clone1, clone2, BLACK, complexity_b,
                                 black_moves, db);
        make_dominated_moves_for(clone1, clone2, WHITE, complexity_w,
                                 white_moves, db);

        const size_t n_total_moves = black_moves.size() + white_moves.size();
        size_t n_dominated_moves = 0;

        for (const generalized_sum_move& gsm : black_moves)
            if (gsm.is_dominated())
                n_dominated_moves++;
        for (const generalized_sum_move& gsm : white_moves)
            if (gsm.is_dominated())
                n_dominated_moves++;

        const size_t n_nondominated_moves = n_total_moves - n_dominated_moves;

        assert(dom->get_kind() == DB_DOM_MOVES_KIND_NONE);
        db_dom_moves_kind moves_kind = DB_DOM_MOVES_KIND_NONDOMINATED;

        if (n_total_moves != 0)
        {
            const double nondominated_ratio = (double) n_nondominated_moves / (double) n_total_moves;

            if (nondominated_ratio > MAX_NONDOMINATED_RATIO)
                moves_kind = DB_DOM_MOVES_KIND_DOMINATED;
            else
                moves_kind = DB_DOM_MOVES_KIND_NONDOMINATED;

        }

        dom->set_kind(moves_kind);
        add_generalized_sum_moves_to_dominated_moves_t(*dom, black_moves, BLACK, moves_kind);
        add_generalized_sum_moves_to_dominated_moves_t(*dom, white_moves, WHITE, moves_kind);
    }

    if (!add_is_safe(complexity_b, complexity_w))
        warn_on_exit::on_db_dom_moves_complexity_overflow();

    entry.complexity = complexity_b + complexity_w;

    cleanup_sumgame(clone1);
    cleanup_sumgame(clone2);

    entry.dominated_moves = dom;
}
