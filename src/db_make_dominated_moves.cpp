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
#include <algorithm>
#include <utility>
#include <sstream>

#include "ThGraph.h"
#include "cgt_basics.h"
#include "database.h"
#include "global_database.h"
#include "safe_arithmetic.h"
#include "sumgame.h"

using namespace std;

#ifdef MCGS_USE_DOMINANCE

template<>
struct std::hash<std::pair<hash_t, ::move>>
{
    inline uint64_t operator()(const std::pair<hash_t, ::move>& p) const noexcept
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
                                const sumgame_move& sm, db_entry_partisan* db_entry)
        : subgame_hash(subgame_hash), move_db_encoded(move_db_encoded), sm(sm), db_entry(db_entry), thermograph(nullptr)
    {
    }

    bool operator<(const generalized_sum_move& rhs) const;
    bool operator==(const generalized_sum_move& rhs) const;
    bool operator!=(const generalized_sum_move& rhs) const;

    hash_t subgame_hash;
    ::move move_db_encoded;

    sumgame_move sm;
    db_entry_partisan* db_entry;
    const ThGraph* thermograph;
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
//inline void play_generalized_sum_move(sumgame& sum, const generalized_sum_move& gsm, bw player)
//{
//    assert(sum.to_play() == player);
//    sum.play_sum(gsm.sm, player);
//}

inline void add_generalized_sum_move_to_dominated_moves_t(
    dominated_moves_t& dom, const generalized_sum_move& gsm,
    bw player)
{
    dom.add_move(gsm.subgame_hash, gsm.move_db_encoded, player);
}

// relation is relative to player to play
relation bools_to_relation(bool black_wins, bool white_wins, bw player)
{
    assert(is_black_white(player));

    if (!black_wins && !white_wins) // 00
        return REL_EQUAL;
    if (!black_wins && white_wins) // 01
        return (player == BLACK) ? REL_LESS : REL_GREATER;
    if (black_wins && !white_wins) // 10
        return (player == BLACK) ? REL_GREATER: REL_LESS;
    if (black_wins && white_wins) // 11
        return REL_FUZZY;

    assert(false);
}

bool sum_cloned_properly(const sumgame& sum1, const sumgame& sum2)
{
    std::stringstream str1;
    std::stringstream str2;

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

    return bools_to_relation(black_wins, white_wins, player);
}

int get_outcome_ordinal(outcome_class oc, bw player)
{
    assert(is_black_white(player));

    // L > P > R
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

bool compare_outcomes(const generalized_sum_move& gsm,
                          int best_ordinal, bw player)
{
    assert(is_black_white(player));
    const outcome_class oc = gsm.db_entry->outcome;

    if (oc == outcome_class::N)
        return false;

    const int ordinal = get_outcome_ordinal(oc, player);

    if (ordinal < best_ordinal)
        return true;

    return false;
}

// Might be REL_UNKNOWN
relation compare_thermographs(const generalized_sum_move& gsm1, const generalized_sum_move& gsm2, bw player)
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

    if (rel != REL_UNKNOWN && player == WHITE)
        rel = (rel == REL_GREATER) ? REL_LESS : REL_GREATER;

    return rel;
}

vector<generalized_sum_move> make_generalized_sum_moves(const sumgame& sum, bw player)
{
    assert(is_black_white(player));
    vector<generalized_sum_move> moves;

    assert(sum.to_play() == player);

    /*
        IMPORTANT: Moves must be deduplicated, to avoid duplicates pruning
        themselves
    */
    std::unordered_set<std::pair<hash_t, ::move>> move_set;

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
        
        moves.emplace_back(subgame_hash, move_db_encoded, sm, nullptr);
    }

    std::sort(moves.begin(), moves.end());

    return moves;
}

//std::vector<bool> get_oc_indexable_vector()
//{
//    std::vector<bool> vec;
//
//    static const outcome_class OC_MAX =
//        std::max({
//            outcome_class::U,
//            outcome_class::L,
//            outcome_class::R,
//            outcome_class::P,
//            outcome_class::N,
//        });
//
//    vec.resize(OC_MAX + 1, false);
//    return vec;
//}
//
void make_dominated_moves_for(sumgame& sum1, sumgame& sum2, bw player,
                              dominated_moves_t& dom, uint64_t& complexity)
{
    complexity = 0;
    database& db = get_global_database();

    assert(is_black_white(player));
    assert_restore_sumgame ars1(sum1);
    assert_restore_sumgame ars2(sum2);

    const bw restore_player1 = sum1.to_play();
    const bw restore_player2 = sum2.to_play();

    sum1.set_to_play(player);
    sum2.set_to_play(player);

    vector<generalized_sum_move> sum_moves = make_generalized_sum_moves(sum1, player);
    const size_t N_MOVES = sum_moves.size();

    vector<bool> dominance_mask(N_MOVES, false);

    // TODO put this back?
    //assert(sum_moves == make_generalized_sum_moves(sum2, player));

    auto is_dominated = [&dominance_mask](size_t move_idx) -> bool
    {
        return dominance_mask[move_idx];
    };

    auto mark_dominated = [&dominance_mask](size_t move_idx) -> void
    {
        dominance_mask[move_idx] = true;
    };

    bool sum1_played = false;
    bool sum2_played = false;

    auto play_gsm_if_not_played = [&](sumgame& sum, generalized_sum_move& gsm, bw player, bool& played) -> void
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

    int best_ordinal = -1;

    for (generalized_sum_move& gsm : sum_moves)
    {
        assert(gsm.db_entry == nullptr);

        assert(sum1.to_play() == player);
        sum1.play_sum(gsm.sm, player);

        gsm.db_entry = db.get_partisan_ptr(sum1);
        assert(gsm.db_entry != nullptr);

        const outcome_class oc = gsm.db_entry->outcome;
        assert(oc != outcome_class::U);

        if (oc != outcome_class::N)
            best_ordinal = max(best_ordinal, get_outcome_ordinal(oc, player));

        const std::shared_ptr<const ThGraph> thermograph = gsm.db_entry->thermograph;
        THROW_ASSERT(thermograph.get() != nullptr);
        gsm.thermograph = thermograph.get();

        sum1.undo_move();
    }

    for (size_t idx1 = 0; idx1 < N_MOVES; idx1++)
    {
        if (is_dominated(idx1))
            continue;

        generalized_sum_move& gsm1 = sum_moves[idx1];

        {
            const bool is_dom = compare_outcomes(gsm1, best_ordinal, player);
            if (is_dom)
            {
                mark_dominated(idx1);
                continue;
            }
        }

        for (size_t idx2 = idx1 + 1; idx2 < N_MOVES; idx2++)
        {
            if (is_dominated(idx1))
                break;
            if (is_dominated(idx2))
                continue;

            generalized_sum_move& gsm2 = sum_moves[idx2];

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
                of `player`
            */
            if (rel == REL_LESS)
                mark_dominated(idx1);
            else if (rel == REL_EQUAL)
            {
                if (gsm1.db_entry->complexity >= gsm2.db_entry->complexity)
                    mark_dominated(idx1);
                else
                    mark_dominated(idx2);
            }
            else if (rel == REL_GREATER)
                mark_dominated(idx2);
            else
                assert(rel == REL_FUZZY);

            undo_gsm_if_played(sum2, player, sum2_played);
        }

        undo_gsm_if_played(sum1, player, sum1_played);
    }

    sum1.set_to_play(restore_player1);
    sum2.set_to_play(restore_player2);

    uint64_t n_immediate_nondom = 0;
    bool no_complexity_overflow = true;

    for (size_t i = 0; i < N_MOVES; i++)
    {
        const generalized_sum_move& gsm = sum_moves[i];

        if (!is_dominated(i))
        {
            n_immediate_nondom++;
            const uint64_t move_complexity = gsm.db_entry->complexity;

            no_complexity_overflow &= safe_add(complexity, move_complexity);

            continue;
        }

        add_generalized_sum_move_to_dominated_moves_t(dom, gsm,
                                                      player);
    }

#warning TODO make this a warning instead of a throw
    no_complexity_overflow &= safe_add(complexity, n_immediate_nondom);
    THROW_ASSERT(no_complexity_overflow);

}

} // namespace

//////////////////////////////////////////////////
void db_make_dominated_moves(const sumgame& sum, db_entry_partisan& entry)
{
    shared_ptr<dominated_moves_t> dom(new dominated_moves_t());

    sumgame clone1(BLACK);
    sumgame clone2(BLACK);
    clone_sumgame(sum, clone1);
    clone_sumgame(sum, clone2);

    uint64_t complexity_b = 0;
    uint64_t complexity_w = 0;

    {
        assert_restore_sumgame ars1(clone1);
        assert_restore_sumgame ars2(clone2);
        make_dominated_moves_for(clone1, clone2, BLACK, *dom, complexity_b);
        make_dominated_moves_for(clone1, clone2, WHITE, *dom, complexity_w);


    }

#warning TODO make this a warning instead of a throw
    THROW_ASSERT(add_is_safe(complexity_b, complexity_w));
    entry.complexity = complexity_b + complexity_w;

    cleanup_sumgame(clone1);
    cleanup_sumgame(clone2);

    entry.dominated_moves = dom;
}
#else
shared_ptr<dominated_moves_t> db_make_dominated_moves(const sumgame& sum)
{
    assert(false);
}
#endif
