//---------------------------------------------------------------------------
// Sum of combinatorial games and solving algorithms
//---------------------------------------------------------------------------
#include <algorithm>
#include <utility>
#include <optional>
#include <ctime>
#include <iostream>
#include <memory>
#include <cassert>
#include <unordered_set>
#include <set>
#include <vector>
#include <cstdint>

#include "sumgame.h"
#include "throw_assert.h"
#include "bounds.h"
#include "db_move_generator.h"
#include "dominated_moves.h"
#include "filtering_move_generator.h"
#include "integral_conversion.h"
#include "global_database.h"
#include "random.h"
#include "safe_arithmetic.h"
#include "search_graph_debug.h"
#include "game.h"
#include "cgt_dyadic_rational.h"
#include "cgt_integer_game.h"
#include "cgt_nimber.h"
#include "cgt_switch.h"
#include "cgt_up_star.h"
#include "alternating_move_game.h"
#include "timeout_token.h"
#include "global_options.h"
#include "solver_stats.h"
#include "sumgame_change_record.h"
#include "sumgame_undo_stack_unwinder.h"
#include "impartial_game_wrapper.h"
#include "utilities.h"
#include "ThGraph.h"
#include "ThValue.h"

constexpr bool PRINT_SUBGAMES = false;

using namespace sumgame_impl;
using namespace std;

shared_ptr<ttable_sumgame> sumgame::_tt(nullptr);


////////////////////////////////////////////////// Helpers
namespace {
unordered_set<game_type_t> basic_cgt_type_set;

inline bool game_is_number(const game* g)
{
    const game_type_t g_type = g->game_type();

    return                                        //
        (g_type == game_type<integer_game>()) ||  //
        (g_type == game_type<dyadic_rational>()); //
}

bool is_simple_cgt(const game* g)
{
    assert(!basic_cgt_type_set.empty());

    const game_type_t type = g->game_type();
    return basic_cgt_type_set.find(type) != basic_cgt_type_set.end();
}

vector<unsigned int> get_oc_indexable_vector()
{
    vector<unsigned int> vec;

    static const outcome_class OC_MAX =
        max({
            outcome_class::U,
            outcome_class::L,
            outcome_class::R,
            outcome_class::P,
            outcome_class::N,
        });

    vec.resize(OC_MAX + 1, 0);
    return vec;
}

/*
    Conclusive:
    - Only Ls
    - Only Rs
    - One N, all others non-negative for to_play
*/
ebw analyze_outcome_count_vector(const vector<unsigned int>& counts,
                                 const bw player)
{
    assert(is_black_white(player));

    const bool has_u = counts[outcome_class::U] > 0;
    if (has_u)
        return EMPTY;

    const bool only_l = counts[outcome_class::L] > 0 &&
                        counts[outcome_class::R] == 0 &&
                        counts[outcome_class::N] == 0;

    if (only_l)
        return BLACK;

    const bool only_r = counts[outcome_class::L] == 0 &&
                        counts[outcome_class::R] > 0 &&
                        counts[outcome_class::N] == 0;

    if (only_r)
        return WHITE;

    const bool one_n = counts[outcome_class::N] == 1;

    const outcome_class negative_class =
        (player == BLACK) ? outcome_class::R : outcome_class::L;

    const bool no_negative = counts[negative_class] == 0;

    if (one_n && no_negative)
        return player;

    return EMPTY;
}

ebw analyze_bounds(bound_t lower_rational, bound_t upper_rational,
                   bound_t lower_ups, bound_t upper_ups)
{
    assert(lower_rational <= upper_rational && //
           lower_ups <= upper_ups              //
    );

    if (lower_rational > 0)
        return BLACK;
    if (upper_rational < 0)
        return WHITE;

    if (lower_rational == 0 && upper_rational == 0)
    {
        if (lower_ups > 0)
            return BLACK;
        if (upper_ups < 0)
            return WHITE;
    }

    return EMPTY;
}

/*
    Puts hottest games first, then games without temperatures.
*/
class game_pair_sort
{
public:
    game_pair_sort(const temperature_vec_t& temperatures)
        : _temperatures(temperatures)
    {
    }

    bool operator()(const pair<int, const game*>& game_pair_1,
                    const pair<int, const game*>& game_pair_2) const
    {
        const optional<ThValue>& temp1 = _temperatures[game_pair_1.first];
        const optional<ThValue>& temp2 = _temperatures[game_pair_2.first];

        if (temp1.has_value())
        {
            if (!temp2.has_value())
                return true;

            return *temp1 > *temp2;
        }

        return false;
    };

private:
    const temperature_vec_t& _temperatures;
};

} // namespace

////////////////////////////////////////////////// sumgame methods
void sumgame::add(game* g)
{
    _subgames.push_back(g);
    assert(g->is_active());

    if (                                //
        global::simplify_basic_cgt() && //
        !_need_cgt_simplify &&          //
        is_simple_cgt(g)                //
        )                               //
        _need_cgt_simplify = true;
}

void sumgame::play_sum(const sumgame_move& sm, bw to_play)
{
    assert(is_black_white(to_play));

    _push_undo_code(SUMGAME_UNDO_PLAY);

    _play_record_stack.push_back(play_record(sm));
    play_record& record = _play_record_stack.back();

    const int subg = sm.subgame_idx;
    const ::move mv = sm.m;

    game* g = subgame(subg);
    assert(g->is_active());

    g->play(mv, to_play);
    split_result sr;

    if (global::play_split())
        sr = g->split();

    if (sr) // split changed the sum
    {
        assert(global::play_split());
        record.split_g = true;

        // Don't normalize g, it's no longer part of the sum
        g->set_active(false);
        record.deactivated_g = true;

        for (game* gp : *sr)
        {
            if (global::play_normalize())
                gp->normalize();

            add(gp);
            record.add_game(gp); // save these games in the record for debugging
        }
    }
    else
    {
        // TODO has_moves() is maybe slow...
        if (!g->has_moves())
        {
            g->set_active(false);
            record.deactivated_g = true;
        }
        else
        {
            if (global::play_normalize())
                g->normalize();
        }
    }

    alternating_move_game::play(mv);
}

void sumgame::undo_move()
{
    _pop_undo_code(SUMGAME_UNDO_PLAY);

    play_record& record = _play_record_stack.back();

    const sumgame_move& sm = record.sm;
    const int subg = sm.subgame_idx;
    game* s = subgame(subg);

    // undo split (if necessary)
    if (record.split_g)
    {
        assert(global::play_split());
        assert(
            !s->is_active() &&
            record.deactivated_g); // should have been deactivated on last split

        s->set_active(true);

        for (auto it = record.new_games.rbegin(); it != record.new_games.rend();
             it++)
        {
            game const* g = *it;
            assert(g->is_active());

            // No need for g->undo_normalize() -- g is being deleted
            pop(g);
            delete g;
        }
    }
    else
    {
        assert(s->is_active() == !record.deactivated_g);

        if (record.deactivated_g)
            s->set_active(true);
        else if (global::play_normalize())
            s->undo_normalize();
    }

    const ::move subm = cgt_move::remove_color(s->last_move());

    assert(                                                            //
        sm.m == subm ||                                                //
        (                                                              //
            (s->game_type() == game_type<impartial_game_wrapper>()) && //
            (cgt_move::remove_color(sm.m) == subm)                     //
            )                                                          //
    );                                                                 //

    s->undo_move();
    alternating_move_game::undo_move();

    _play_record_stack.pop_back();
}

void sumgame::print(ostream& str) const
{
    str << "sumgame: " << num_total_games() << " total " << num_active_games()
        << " active: ";
    bool first = true;
    for (auto g : _subgames)
        if (g->is_active())
        {
            if (first)
                first = false;
            else
                str << ' ';
            g->print(str);
        }
    str << endl;
}

void sumgame::print_simple(ostream& os) const
{
    os << color_to_player_char(to_play());

    const int n_games = num_total_games();
    for (int i = 0; i < n_games; i++)
    {
        const game* sg = subgame_const(i);
        if (!sg->is_active())
            continue;

        os << " " << *sg;
    }

    os << flush;
}

void sumgame::print_sorted(ostream& str) const
{
    vector<const game*> active_games;

    const int n_games = num_total_games();
    for (int i = 0; i < n_games; i++)
    {
        const game* g = subgame(i);
        if (!g->is_active())
            continue;

        active_games.push_back(g);
    }

    auto sort_fn = [](const game* g1, const game* g2) -> bool
    {
        return g1->get_local_hash() < g2->get_local_hash();
    };

    sort(active_games.begin(), active_games.end(), sort_fn);

    const size_t n_active = active_games.size();
    for (size_t i = 0; i < n_active; i++)
    {
        if (i > 0)
           str << " ";

        const game* g = active_games[i];
        str << *g;
    }

    str << flush;
}

// Solve combinatorial game - find winner
// Game-independent implementation of boolean minimax,
// plus sumgame simplification
bool sumgame::solve() const
{
    // No assert_restore_sumgame; downstream function will do it
    sumgame& sum = const_cast<sumgame&>(*this);

    optional<solve_result> result = sum.solve_with_timeout(0);
    assert(result.has_value());

    return result.value().win;
}

bool sumgame::solve_with_games(game* g) const
{
    assert_restore_sumgame ars(*this);
    sumgame& sum = const_cast<sumgame&>(*this);

    sum.add(g);

    bool result = solve();

    game* back = sum._pop_game();
    assert(back == g);

    return result;
}

bool sumgame::solve_with_games(const vector<game*>& games) const
{
    assert_restore_sumgame ars(*this);
    sumgame& sum = const_cast<sumgame&>(*this);

    for (game* g : games)
        sum.add(g);

    bool result = solve();

    const size_t N = games.size();
    for (size_t i = 0; i < N; i++)
    {
        game* back = sum._pop_game();
        game* g = games[N - 1 - i];

        assert(back == g);
    }

    return result;
}

optional<solve_result> sumgame::solve_with_timeout(
    unsigned long long timeout) const
{
    timeout_source src;
    timeout_token tok = src.get_timeout_token();

    src.start_timeout(timeout);
    optional<solve_result> result =
        solve_with_timeout_token(tok, INITIAL_SEARCH_DEPTH);
    src.cancel_timeout();

    return result;
}

optional<solve_result> sumgame::solve_with_timeout_token(
    const timeout_token& timeout_tok, uint64_t depth) const
{
    assert_restore_sumgame ars(*this);
    sumgame& sum = const_cast<sumgame&>(*this);

    // Set up timeout_token
    assert(!sum._timeout_tok.has_value());
    sum._timeout_tok = timeout_tok;

    sum._pre_solve_pass();

    _need_cgt_simplify = true;

    optional<solve_result> result = sum._solve_impl(depth);

    sum._undo_pre_solve_pass();

    sum._timeout_tok.reset();

    return result;
}

void sumgame::simplify_basic()
{
    if (!global::simplify_basic_cgt())
        return;

    if (!_need_cgt_simplify)
        return;

    _change_record_stack.emplace_back();
    change_record& record = _change_record_stack.back();

#ifdef SUMGAME_DEBUG
    const hash_t hash_before = get_global_hash();
#endif

    record.simplify_basic(*this);
    _need_cgt_simplify = false;

#ifdef SUMGAME_DEBUG
    const hash_t hash_after = get_global_hash();
    assert((hash_before == hash_after) == record.no_change());
#endif

    if (record.no_change())
    {
        _change_record_stack.pop_back();
        return;
    }

    _push_undo_code(SUMGAME_UNDO_SIMPLIFY_BASIC);
}

void sumgame::undo_simplify_basic()
{
    if (!global::simplify_basic_cgt())
        return;

    _pop_undo_code(SUMGAME_UNDO_SIMPLIFY_BASIC);

    _need_cgt_simplify = true;

    assert(!_change_record_stack.empty());
    change_record& record = _change_record_stack.back();
    record.undo_simplify_basic(*this);

    _change_record_stack.pop_back();
}

/*
    Extracts data from the DB entries of partisan subgames. Also deactivates
    games with outcome_class::P, and uses outcome classes and bounds to solve
    the sum.

    When the sum is not solved, `temperatures` and `dom_objects` may or may not
    be empty. When not empty, they are indexable by subgame index and have size
    `num_total_games()`.

    - Adds up bounds on scales BOUND_SCALE_UP and BOUND_SCALE_DYADIC_RATIONAL.
        The bound sum is invalidated in the following cases:

        1. A partisan game has no DB entry, or its entry has no bounds, or the
            bounds are on a different scale.

        2. Bound addition would overflow.

        3. When an impartial game is active, the UP component of the sum is
            zeroed before use (though the RATIONAL component is still valid).
*/
optional<solve_result> sumgame::db_lookup_pass(temperature_vec_t& temperatures,
                                               dom_object_vec_t& dom_objects)
{
    assert(temperatures.empty() && dom_objects.empty());
    _push_undo_code(SUMGAME_UNDO_DB_LOOKUP_PASS);

    if (!global::use_db())
        return {};

    _change_record_stack.emplace_back();
    sumgame_impl::change_record& cr = _change_record_stack.back();

    const int N_SUBGAMES = num_total_games();

    database& db = get_global_database();

    bound_t lower_bound_ups = 0;
    bound_t upper_bound_ups = 0;

    bound_t lower_bound_rationals = 0;
    bound_t upper_bound_rationals = 0;

    bool bounds_valid = true;
    bool at_least_one_impartial = false;

    vector<unsigned int> oc_counts = get_oc_indexable_vector();

    // Search all active games
    for (int subgame_idx = 0; subgame_idx < N_SUBGAMES; subgame_idx++)
    {
        game* g = subgame(subgame_idx);
        if (!g->is_active())
            continue;

        // Get g's outcome class
        outcome_class oc = outcome_class::U;

        if (g->is_impartial())
        {
            /*
               Because db_replacement_pass() should be called prior to this
               function, one of the following must hold:
                   1. g is a nimber
                   2. g is not a nimber, AND is not in the database
            */
            if (g->game_type() == game_type<nimber>())
            {
                assert(dynamic_cast<nimber*>(g) != nullptr);
                nimber* g_nimber = static_cast<nimber*>(g);

                const int g_nim_value = g_nimber->value();
                assert(g_nim_value >= 0);

                oc = (g_nim_value == 0) ? outcome_class::P : outcome_class::N;
            }

            if (oc != outcome_class::P)
                at_least_one_impartial = true;
        }
        else
        {
            const db_entry_partisan* entry = db.get_partisan_ptr(*g);
            const bool has_value = entry != nullptr;
            stats::report_db_access(has_value);

            if (!has_value)
            {
                bounds_valid = false;
            }
            else
            {
                // Use outcome
                oc = entry->outcome;

                // Use thermograph
                const shared_ptr<ThGraph>& entry_graph = entry->thermograph;

                if (entry_graph)
                {
                    if (temperatures.empty())
                        temperatures.resize(N_SUBGAMES);

                    optional<ThValue>& g_temp = temperatures[subgame_idx];
                    g_temp = entry_graph->Temperature();
                }

                // Use dominated moves
                const shared_ptr<db_dom_moves_t>& entry_dom_object = entry->dominated_moves;

                if (entry_dom_object)
                {
                    if (dom_objects.empty())
                        dom_objects.resize(N_SUBGAMES);

                    shared_ptr<const db_dom_moves_t>& g_dom_obj = dom_objects[subgame_idx];
                    g_dom_obj = entry_dom_object;
                }

                // Use bounds
                if (!(bounds_valid && entry->bounds_data && entry->bounds_data->both_valid()))
                    bounds_valid = false;
                else
                {
                    const game_bounds& bounds = *entry->bounds_data;
                    const bound_scale scale = bounds.get_scale();

                    THROW_ASSERT(bounds.get_lower_relation() == REL_LESS &&
                                 bounds.get_upper_relation() == REL_GREATER);

                    switch (scale)
                    {
                        case BOUND_SCALE_UP:
                        {
                            bounds_valid &= safe_add(lower_bound_ups, bounds.get_lower());
                            bounds_valid &= safe_add(upper_bound_ups, bounds.get_upper());
                            break;
                        }

                        case BOUND_SCALE_DYADIC_RATIONAL:
                        {
                            bounds_valid &= safe_add(lower_bound_rationals, bounds.get_lower());
                            bounds_valid &= safe_add(upper_bound_rationals, bounds.get_upper());
                            break;
                        }

                        case BOUND_SCALE_UP_STAR:
                        {
                            bounds_valid = false;
                            break;
                        }
                    }
                }
            }
        }

        oc_counts[oc]++;

        if (oc == outcome_class::P)
        {
            assert(g->is_active());
            g->set_active(false);
            cr.deactivated_games.push_back(g);
        }
    }

    if (bounds_valid)
    {
        if (at_least_one_impartial)
        {
            // Can't use infinitesimal part of bounds
            lower_bound_ups = 0;
            upper_bound_ups = 0;
        }

        const ebw bounds_winner =
            analyze_bounds(lower_bound_rationals, upper_bound_rationals,
                           lower_bound_ups, upper_bound_ups);

        if (bounds_winner != EMPTY)
            return solve_result(bounds_winner == to_play());
    }

    const ebw outcome_winner = analyze_outcome_count_vector(oc_counts, to_play());

    if (outcome_winner != EMPTY)
        return solve_result(outcome_winner == to_play());

    return {};
}

void sumgame::undo_db_lookup_pass()
{
    _pop_undo_code(SUMGAME_UNDO_DB_LOOKUP_PASS);

    if (!global::use_db())
        return;

    sumgame_impl::change_record& cr = _change_record_stack.back();
    assert(cr.added_games.empty());

    for (game* g : cr.deactivated_games)
    {
        assert(!g->is_active());
        g->set_active(true);
    }

    cr.deactivated_games.clear();
    _change_record_stack.pop_back();
}

void sumgame::split_and_normalize()
{
    _push_undo_code(SUMGAME_UNDO_SPLIT_AND_NORMALIZE);
    _change_record_stack.emplace_back();
    sumgame_impl::change_record& cr = _change_record_stack.back();

    const int n_games = num_total_games();
    for (int i = 0; i < n_games; i++)
    {
        game* g = subgame(i);
        if (!g->is_active())
            continue;

        split_result sr = g->split();

        if (sr)
        {
            g->set_active(false);
            cr.deactivated_games.push_back(g);

            for (game* sg : *sr)
            {
                sg->normalize();
                // actually add later
                cr.added_games.push_back(sg);
                assert(sg->has_moves());
            }
        }
        else
        {
            if (!g->has_moves())
            {
                g->set_active(false);
                cr.deactivated_games.push_back(g);
            }
            else
            {
                g->normalize();
                cr.normalized_games.push_back(g);
            }
        }
    }

    add(cr.added_games);
}

void sumgame::undo_split_and_normalize()
{
    _pop_undo_code(SUMGAME_UNDO_SPLIT_AND_NORMALIZE);
    sumgame_impl::change_record& cr = _change_record_stack.back();

    pop(cr.added_games);
    for (game* sg : cr.added_games)
        delete sg;
    cr.added_games.clear();

    for (game* g : cr.deactivated_games)
    {
        assert(!g->is_active());
        g->set_active(true);
    }
    cr.deactivated_games.clear();

    for (game* g : cr.normalized_games)
    {
        assert(g->is_active());
        g->undo_normalize();
    }
    cr.normalized_games.clear();

    _change_record_stack.pop_back();
}

/*
   1. nim-add impartial games which are nimbers or have DB entries
   2. Replace partisan games with their bound games (when they're equal)
*/
void sumgame::db_replacement_pass()
{
    _push_undo_code(SUMGAME_UNDO_DB_REPLACEMENT_PASS);

    if (!global::use_db())
        return;

    database& db = get_global_database();

    // Push record
    _change_record_stack.emplace_back();
    sumgame_impl::change_record& cr = _change_record_stack.back();

    int n_known_imp = 0; // nimbers and impartial_games
    int n_known_imp_non_nimber = 0; // known values which are not nimbers
    int final_nim_sum = 0;
    vector<game*> deactivated_impartial_games; // deactivation is deferred

    const int N_SUBGAMES = num_total_games();
    for (int i = 0; i < N_SUBGAMES; i++)
    {
        game* sg = subgame(i);
        if (!sg->is_active())
            continue;

        if (sg->is_impartial())
        {
            assert(dynamic_cast<impartial_game*>(sg) != nullptr);

            // Don't need to look up nimber
            if (sg->game_type() == game_type<nimber>())
            {
                assert(dynamic_cast<nimber*>(sg) != nullptr);
                nimber* sg_nimber = static_cast<nimber*>(sg);

                n_known_imp++;
                nimber::add_nimber(final_nim_sum, sg_nimber->value());

                // Actually deactivate later
                deactivated_impartial_games.push_back(sg_nimber);
                continue;
            }

            assert(dynamic_cast<nimber*>(sg) == nullptr);

            optional<db_entry_impartial> entry = db.get_impartial(*sg);
            stats::report_db_access(entry.has_value());

            if (!entry.has_value())
                continue;

            n_known_imp++;
            n_known_imp_non_nimber++;

            const int entry_nimber = entry->nim_value;
            assert(entry_nimber >= 0);

            // Actually deactivate later
            deactivated_impartial_games.push_back(sg);
            nimber::add_nimber(final_nim_sum, entry_nimber);
        }
        else
        {
            // sg is partisan
            const db_entry_partisan* entry = db.get_partisan_ptr(*sg);
            stats::report_db_access(entry != nullptr);

            if (entry == nullptr)
                continue;

            if (!entry->bounds_data)
                continue;

            const game_bounds& bounds = *entry->bounds_data;

            if (!bounds.is_equal())
                continue;

            game* sg_replacement = get_scale_game(bounds.get_lower(), bounds.get_scale());
            add(sg_replacement);
            cr.added_games.push_back(sg_replacement);

            sg->set_active(false);
            cr.deactivated_games.push_back(sg);
        }
    }

    if (n_known_imp >= 2 || n_known_imp_non_nimber > 0)
    {
        for (game* sg : deactivated_impartial_games)
        {
            assert(sg->is_active());
            sg->set_active(false);
            cr.deactivated_games.push_back(sg);
        }

        if (final_nim_sum != 0)
        {
            nimber* nim_sum_as_nimber = new nimber(final_nim_sum);

            add(nim_sum_as_nimber);
            cr.added_games.push_back(nim_sum_as_nimber);
        }
    }
}

void sumgame::undo_db_replacement_pass()
{
    _pop_undo_code(SUMGAME_UNDO_DB_REPLACEMENT_PASS);

    if (!global::use_db())
        return;

    sumgame_impl::change_record& cr = _change_record_stack.back();

    for (game* g : cr.added_games)
        assert(g->is_active());

    pop(cr.added_games);
    for (game* g : cr.added_games)
        delete g;

    cr.added_games.clear();

    for (game* g : cr.deactivated_games)
    {
        assert(!g->is_active());
        g->set_active(true);
    }
    cr.deactivated_games.clear();

    _change_record_stack.pop_back();
}

optional<sumgame_move> sumgame::get_winning_or_random_move(
    bw for_player) const
{
    assert(is_black_white(for_player));
    assert_restore_sumgame ars(*this);

    const bw prev_player = to_play();

    sumgame& sum = const_cast<sumgame&>(*this);
    sum.set_to_play(for_player);

    unique_ptr<sumgame_move_generator> gen(
        sum.create_sum_move_generator(for_player));

    vector<sumgame_move> moves;

    while (*gen)
    {
        moves.emplace_back(gen->gen_sum_move());
        const sumgame_move& sm = moves.back();
        ++(*gen);

        assert(sum.to_play() == for_player);
        sum.play_sum(sm, for_player);
        assert(sum.to_play() == ::opponent(for_player));
        bool opp_loss = !sum.solve();
        sum.undo_move();

        if (opp_loss)
        {
            sum.set_to_play(prev_player);
            return sm;
        }
    }

    sum.set_to_play(prev_player);

    if (moves.empty())
        return {};

    // TODO: random_generator should work for arbitrary types...
    const uint32_t choice = get_global_rng().get_u32(0, moves.size() - 1);
    return moves[choice];
}

void sumgame::init_sumgame(size_t index_bits)
{
    assert(basic_cgt_type_set.empty());
    basic_cgt_type_set.insert(game_type<dyadic_rational>());
    basic_cgt_type_set.insert(game_type<integer_game>());
    basic_cgt_type_set.insert(game_type<nimber>());
    basic_cgt_type_set.insert(game_type<switch_game>());
    basic_cgt_type_set.insert(game_type<up_star>());

    // Now initialize ttable
    if (global::tt_sumgame_idx_bits() == 0)
        return;

    assert(_tt.get() == nullptr); // Not already initialized
    _tt.reset(new ttable_sumgame(index_bits, 1));
}

void sumgame::clear_ttable()
{
    assert(global::clear_tt());

    if (_tt.get() == nullptr)
    {
        assert(global::tt_sumgame_idx_bits() == 0);
        return;
    }

    _tt->clear();
}

void sumgame::_pre_solve_pass()
{
    _push_undo_code(SUMGAME_UNDO_PRE_SOLVE_PASS);

    // TODO change records are used in several places, but are kind of messy...
    // make this better
    _change_record_stack.emplace_back();
    sumgame_impl::change_record& cr = _change_record_stack.back();

    const int N = num_total_games();
    for (int i = 0; i < N; i++)
    {
        game* g = subgame(i);

        if (!g->is_active())
            continue;

        split_result sr = g->split();

        if (!sr.has_value())
            g->normalize();
        else
        {
            g->set_active(false);
            cr.deactivated_games.push_back(g);

            for (game* sg : *sr)
            {
                sg->normalize();

                add(sg);
                cr.added_games.push_back(sg);
            }
        }
    }
}

void sumgame::_undo_pre_solve_pass()
{
    _pop_undo_code(SUMGAME_UNDO_PRE_SOLVE_PASS);
    sumgame_impl::change_record& cr = _change_record_stack.back();

    const int N = num_total_games();
    for (int i = 0; i < N; i++)
    {
        game* g = subgame(i);

        if (!g->is_active())
            continue;

        g->undo_normalize();
    }

    for (game* g : cr.deactivated_games)
    {
        assert(!g->is_active());
        g->set_active(true);
    }

    for (auto it = cr.added_games.rbegin(); it != cr.added_games.rend(); it++)
    {
        game* g = *it;
        assert(g->is_active());

        pop(g);
        delete g;
    }

    cr.added_games.clear();
    cr.deactivated_games.clear();

    _change_record_stack.pop_back();
}

optional<solve_result> sumgame::_solve_impl(uint64_t depth)
{
#ifdef SUMGAME_DEBUG
    _debug_extra();
    assert_restore_sumgame ars(*this); // must come before the stack unwinder
#endif

    undo_stack_unwinder stack_unwinder(*this);
    sgraph::push(*this);

    if (_over_time())
        return solve_result::invalid();


    stats::report_search_node(*this, to_play(), depth);
    const uint64_t next_depth = depth + 1; // for after a move is played

    if (PRINT_SUBGAMES)
    {
        cout << "solve sum ";
        print(cout);
    }

    temperature_vec_t temperatures;
    dom_object_vec_t dom_move_objects;

    {
        db_replacement_pass();
        simplify_basic();

        optional<solve_result> result =
            db_lookup_pass(temperatures, dom_move_objects);

        if (result.has_value())
        {
            sgraph::pop_winloss(result->win);
            return result;
        }
    }

    /*      NOTE:
       This is sort of a nested optional. If sumgame's ttable is disabled (i.e.
       it uses 0 index bits), then tt_result doesn't hold a search_result.

       If tt_result holds a search_result, the queried hash may still be a miss.
    */
    optional<ttable_sumgame::search_result> tt_result =
        _do_ttable_lookup();

    if (tt_result.has_value() && tt_result->entry_valid())
    {
        const bool win = tt_result->get_bool(0);
        sgraph::pop_winloss(win);
        return win;
    }

    const bw toplay = to_play();

    unique_ptr<sumgame_move_generator> mgp =
        make_unique<sumgame_move_generator>(*this, toplay, &temperatures,
                                            &dom_move_objects);

    sumgame_move_generator& mg = *mgp;

    for (; mg; ++mg)
    {
        const sumgame_move m = mg.gen_sum_move();
        play_sum(m, toplay);

        solve_result result(false);

        bool found = find_static_winner(result.win);

        if (!found)
        {
            optional<solve_result> child_result = _solve_impl(next_depth);

            if (!child_result.has_value() || _over_time())
                return solve_result::invalid();

            result.win = not child_result.value().win;
        }

        undo_move();

        if (result.win)
        {
            if (tt_result.has_value())
            {
                tt_result->init_entry();
                tt_result->set_bool(0, result.win);
            }

            sgraph::pop_winloss(result.win);
            return result;
        }
    }

    if (tt_result.has_value())
    {
        tt_result->init_entry();
        tt_result->set_bool(0, false);
    }


    sgraph::pop_winloss(false);
    return solve_result(false);
}

optional<ttable_sumgame::search_result> sumgame::_do_ttable_lookup() const
{
    if (global::tt_sumgame_idx_bits() == 0)
        return {};

    assert(_tt != nullptr);

    const hash_t current_hash = get_global_hash();

    ttable_sumgame::search_result sr = _tt->search(current_hash);
    stats::report_tt_access(sr.entry_valid());

    return sr;
}

void sumgame::_debug_extra() const
{
    _assert_games_unique();
}

void sumgame::_assert_games_unique() const
{
    unordered_set<game*> game_set;

    const size_t n_games = num_total_games();
    for (size_t i = 0; i < n_games; i++)
    {
        game* g = subgame(i);
        auto it = game_set.insert(g);
        assert(it.second == true);
    }
}

//////////////////////////////////////////////////
// sumgame_move_generator methods
sumgame_move_generator::sumgame_move_generator(
    const sumgame& sum, bw to_play, temperature_vec_t* temperatures,
    dom_object_vec_t* dom_move_objects)
    : move_generator(to_play),
      //_sum(sum),
      _subgame_idx_local(0),
      _subgame_current(nullptr),
      _mg_current(nullptr)
{
    const int N_SUBGAMES = sum.num_total_games();

    // Steal contents of DB data vectors (if not nullptr)
    temperature_vec_t temperatures_local;

    if (temperatures != nullptr)
    {
        temperatures_local = std::move(*temperatures);

        assert(temperatures_local.empty() ||                               //
               temperatures_local.size() == as_unsigned_unsafe(N_SUBGAMES) //
        );
    }

    if (dom_move_objects != nullptr)
    {
        _dom_objects = std::move(*dom_move_objects);

        assert(_dom_objects.empty() ||                               //
               _dom_objects.size() == as_unsigned_unsafe(N_SUBGAMES) //
        );
    }

    assert(LOGICAL_IMPLIES(!global::use_db(),
                           temperatures_local.empty() && _dom_objects.empty()));

    /*
        Apply ordering to subgames in sumgame_move_generator:

        1. Hottest games
        2. Games without temperatures
        3. integers/rationals
    */
    vector<pair<int, const game*>> numbers;

    size_t n_active = 0;
    for (int i = 0; i < N_SUBGAMES; i++)
    {
        game* g = sum.subgame(i);
        if (!g->is_active())
            continue;
        n_active++;

        if (game_is_number(g))
            numbers.emplace_back(i, g);
        else
            _subgame_pairs.emplace_back(i, g);
    }

    if (!temperatures_local.empty())
    {
        game_pair_sort sorter(temperatures_local);
        sort(_subgame_pairs.begin(), _subgame_pairs.end(), sorter);
    }

    for (const pair<int, const game*>& game_pair : numbers)
        _subgame_pairs.push_back(game_pair);

    assert(_subgame_pairs.size() == n_active);
    _increment(true);
}


sumgame_move sumgame_move_generator::gen_sum_move() const
{
    assert(*this);
    assert(_mg_current && *_mg_current);

    const pair<int, const game*>& sg = _subgame_pairs[_subgame_idx_local];
    return sumgame_move(sg.first, _mg_current->gen_move());
}

void sumgame_move_generator::_increment(bool init)
{
    assert(init || *this);

    bool has_subgame = !init;
    bool has_move = !init;

    while (true)
    {
        // Try to increment move
        if (has_subgame && _increment_move(!has_move))
        {
            has_move = true;
            assert(*this);
            return;
        }

        has_move = false;

        // Try to increment subgame
        if (_increment_subgame(!has_subgame))
        {
            has_subgame = true;
            continue;
        }

        has_subgame = false;
        assert(!*this);
        return;
    }
}

bool sumgame_move_generator::_increment_subgame(bool init)
{
    if (init)
        assert(_subgame_idx_local == 0);
    else
        _subgame_idx_local++;

    while (_subgame_idx_local < _subgame_pairs.size())
    {
        const pair<int, const game*>& game_pair =
            _subgame_pairs[_subgame_idx_local];

        const int subgame_idx_nonlocal = game_pair.first;
        _subgame_current = game_pair.second;

        if (_should_skip_game(*_subgame_current))
        {
            _subgame_idx_local++;
            continue;
        }
        
        const db_dom_moves_t* dom_obj = nullptr;

        if (!_dom_objects.empty())
            dom_obj = _dom_objects[subgame_idx_nonlocal].get();

        _mg_current.reset(
            _make_subgame_move_generator(*_subgame_current, dom_obj));

        return true;
    }

    return false;
}

bool sumgame_move_generator::_increment_move(bool init)
{
    assert(_mg_current.get() != nullptr);
    assert(init || *_mg_current);

    if (!init)
        ++(*_mg_current);

    if (!(*_mg_current))
    {
        _mg_current.reset();
        return false;
    }
    
    return true;
}

bool sumgame_move_generator::_should_skip_game(const game& g)
{
    if (global::dedupe_movegen())
    {
        const hash_t hash = g.get_local_hash();
        const auto inserted = _seen_local_hashes.insert(hash);

        if (!inserted.second)
            return true;
    }

    return false;
}

move_generator* sumgame_move_generator::_make_subgame_move_generator(
    const game& g, const db_dom_moves_t* dom_moves_object) const
{
    if (dom_moves_object == nullptr ||                         //
        dom_moves_object->get_kind() == DB_DOM_MOVES_KIND_NONE //
    )
        return g.create_move_generator(to_play());

    switch (dom_moves_object->get_kind())
    {
        case DB_DOM_MOVES_KIND_NONE:
            assert(false);
        case DB_DOM_MOVES_KIND_DOMINATED:
        {
            const set<::move>* dom_set = dom_moves_object->get_dominated_moves(
                g.get_local_hash(), to_play());

            if (dom_set == nullptr)
                break;

            return new filtering_move_generator(g, to_play(), *dom_set);
        }
        case DB_DOM_MOVES_KIND_NONDOMINATED:
        {
            const vector<::move>* nondom_vec =
                dom_moves_object->get_nondominated_moves(g.get_local_hash(),
                                                         to_play());

            if (nondom_vec == nullptr)
                break;

            return new db_move_generator(g, to_play(), *nondom_vec);
        }
    }

    return g.create_move_generator(to_play());
}

////////////////////////////////////////////////// assert_restore_sumgame methods

#ifdef ASSERT_RESTORE_DEBUG
assert_restore_sumgame::assert_restore_sumgame(const sumgame& sgame)
    : assert_restore_alternating_game(sgame),
      _sgame(sgame),
      _global_hash(sgame.get_global_hash()),
      _total_subgames(sgame.num_total_games()),
      _undo_code_stack_size(sgame._undo_code_stack.size()),
      _play_record_stack_size(sgame._play_record_stack.size()),
      _change_record_stack_size(sgame._change_record_stack.size())
{
}

assert_restore_sumgame::~assert_restore_sumgame()
{
    assert(_global_hash == _sgame.get_global_hash());
    assert(_total_subgames == _sgame.num_total_games());
    assert(_undo_code_stack_size == _sgame._undo_code_stack.size());
    assert(_play_record_stack_size == _sgame._play_record_stack.size());
    assert(_change_record_stack_size == _sgame._change_record_stack.size());
}

#endif
