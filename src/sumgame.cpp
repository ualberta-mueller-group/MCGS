//---------------------------------------------------------------------------
// Sum of combinatorial games and solving algorithms
//---------------------------------------------------------------------------
#include "sumgame.h"
#include "bounds.h"
#include "cgt_basics.h"
#include "cgt_move.h"
#include "database.h"
#include "global_database.h"
#include "random.h"
#include "type_table.h"
#include "game.h"

#include "cgt_dyadic_rational.h"
#include "cgt_integer_game.h"
#include "cgt_nimber.h"
#include "cgt_switch.h"
#include "cgt_up_star.h"
#include "alternating_move_game.h"
#include "timeout_token.h"

#include <algorithm>
#include <numeric>
#include <utility>
#include <optional>
#include <ctime>
#include <iostream>
#include <memory>

#include <cassert>
#include <unordered_set>
#include <vector>

#include "global_options.h"
#include "hashing.h"
#include "solver_stats.h"
#include "sumgame_change_record.h"
#include "sumgame_undo_stack_unwinder.h"
#include "impartial_game_wrapper.h"
#include "utilities.h"

using std::cout;
using std::endl;
using std::optional;
using sumgame_impl::change_record;

std::shared_ptr<ttable_sumgame> sumgame::_tt(nullptr);

//---------------------------------------------------------------------------

// Helpers
namespace {
inline bool game_is_number(const game* g)
{
    const game_type_t g_type = g->game_type();

    return                                        //
        (g_type == game_type<integer_game>()) ||  //
        (g_type == game_type<dyadic_rational>()); //
}

} // namespace


////////////////////////////////////////////////// Move generators

//////////////////////////////////////// sumgame_move_generator (i.e. v1.5)
#ifndef SUMGAME_MOVE_GENERATOR_1_6

sumgame_move_generator::sumgame_move_generator(const sumgame& game, bw to_play)
    : move_generator(to_play),
      _game(game),
      _num_subgames(game.num_total_games()),
      _skipped_games(),
      _use_skipped_games(false),
      _subgame_idx(0),
      _subgame_generator(nullptr)
{
    // scroll to first move
    next_move(true);
}

sumgame_move_generator::~sumgame_move_generator()
{
    if (_subgame_generator != nullptr)
    {
        delete _subgame_generator;
        _subgame_generator = nullptr;
    }
}

void sumgame_move_generator::operator++()
{
    // scroll to next move
    assert(*this);
    next_move(false);
}

void sumgame_move_generator::next_move(bool init)
{
    // increment existing generator and check for move
    if (_subgame_generator != nullptr)
    {
        ++(*_subgame_generator);

        if (*_subgame_generator)
            return; // we have a move
    }

    // discard generator
    if (_subgame_generator != nullptr)
    {
        delete _subgame_generator;
        _subgame_generator = nullptr;
    }

    // scroll until we have an active subgame AND its generator has a move
    _subgame_idx = init ? 0 : _subgame_idx + 1;
    assert(_subgame_generator == nullptr);

    const int N = _use_skipped_games ? _skipped_games.size() : _num_subgames;

    for (; _subgame_idx < N; _subgame_idx++)
    {
        assert(_subgame_generator == nullptr);
        std::pair<int, const game*> p = _current();
        const game* g = p.second;

        // inactive game
        if (!g->is_active())
            continue;

        // Skip integers and rationals
        if (!_use_skipped_games && game_is_number(g))
        {
            _skipped_games.push_back({_subgame_idx, g});
            continue;
        }

        // Skip already seen games
        if (global::dedupe_movegen())
        {
            const hash_t hash = g->get_local_hash();
            const bool already_seen = !_seen_games.insert(hash).second;

            if (already_seen)
                continue;
        }

        _subgame_generator = g->create_move_generator(to_play());

        if (*_subgame_generator)
            return; // found move
        else
        {
            delete _subgame_generator;
            _subgame_generator = nullptr;
        }
    }

    if (!_use_skipped_games)
    {
        _use_skipped_games = true;
        assert(_subgame_generator == nullptr);
        next_move(true);
    }
}

sumgame_move_generator::operator bool() const
{
    // do we have a move?
    if (_use_skipped_games)
        // TODO sumgame has a few of these casts. Either convince ourselves
        // it's OK, or just use size_t?
        return _subgame_idx < static_cast<int>(_skipped_games.size());
    else
        return _subgame_idx < _num_subgames;
}

sumgame_move sumgame_move_generator::gen_sum_move() const
{
    assert(_subgame_generator);
    if (_use_skipped_games)
        return sumgame_move(_skipped_games[_subgame_idx].first,
                            _subgame_generator->gen_move());
    else
        return sumgame_move(_subgame_idx, _subgame_generator->gen_move());
}


//////////////////////////////////////// sumgame_move_generator (v1.6 changes)
#else

sumgame_move_generator::sumgame_move_generator(
    const sumgame& sum, bw to_play,
    const temp_vec_opt_t& temperatures)
    : move_generator(to_play), _subgame_idx_local(0), _sum(sum)
{
    assert(LOGICAL_IMPLIES(                                               //
        temperatures.has_value(),                                         //
        temperatures->size() >= as_unsigned_unsafe(sum.num_total_games()) //
        ));                                                               //

    std::vector<std::pair<int, const game*>> numbers;

    // TODO pruning duplicate games?

    const int n_games = sum.num_total_games();
    for (int i = 0; i < n_games; i++)
    {
        game* g = sum.subgame(i);
        if (!g->is_active())
            continue;

        if (game_is_number(g))
            numbers.emplace_back(i, g);
        else
            _subgames.emplace_back(i, g);
    }

#ifdef MCGS_USE_THERM
    auto sort_fn = [&](const std::pair<int, const game*>& sg1,
                       const std::pair<int, const game*>& sg2) -> bool
    {
        assert(temperatures.has_value());

        const std::optional<ThValue>& temp1 = (*temperatures)[sg1.first];
        const std::optional<ThValue>& temp2 = (*temperatures)[sg2.first];

        if (temp1.has_value())
        {
            if (!temp2.has_value())
                return true;
                //return false;

            return *temp1 > *temp2;
        }

        return false;
        //return temp2.has_value();
    };

    if (temperatures.has_value())
        std::sort(_subgames.begin(), _subgames.end(), sort_fn);
#endif

    for (const std::pair<int, const game*>& sg : numbers)
        _subgames.push_back(sg);

    _increment(true);
}

sumgame_move_generator::~sumgame_move_generator()
{
}

void sumgame_move_generator::operator++()
{
    assert(*this);
    _increment(false);
}

sumgame_move_generator::operator bool() const
{
    return (_mg && *_mg);
}

sumgame_move sumgame_move_generator::gen_sum_move() const
{
    assert(*this && _mg && *_mg);
    const std::pair<int, const game*>& sg = _subgames[_subgame_idx_local];
    return sumgame_move(sg.first, _mg->gen_move());
}

void sumgame_move_generator::_increment(bool init)
{
    assert(init || *this);

    if (init)
    {
        assert(_subgame_idx_local == 0 && _seen_games.empty());

        if (_subgames.empty())
            return;


        const game* sg_first = _subgames.front().second;

        if (global::dedupe_movegen())
        {
            const hash_t hash = sg_first->get_local_hash();
            _seen_games.insert(hash);
        }

        _mg.reset(sg_first->create_move_generator(to_play()));
    }
    else
    {
        assert(_mg && *_mg);
        ++(*_mg);
    }

    const size_t n_subgames = _subgames.size();

    while (true)
    {
        if (_mg && *_mg)
            return;

        _mg.reset();
        _subgame_idx_local++;

        if (!(_subgame_idx_local < n_subgames))
            return;

        const game* sg = _subgames[_subgame_idx_local].second;

        if (global::dedupe_movegen())
        {
            const hash_t hash = sg->get_local_hash();

            auto inserted = _seen_games.insert(hash);
            if (!inserted.second)
                continue;
        }
        _mg.reset(sg->create_move_generator(to_play()));
    }
}

#endif
//////////////////////////////////////////////////
// Helpers

namespace {

std::unordered_set<game_type_t> basic_cgt_type_set;

bool is_simple_cgt(const game* g)
{
    assert(!basic_cgt_type_set.empty());

    game_type_t type = g->game_type();
    return basic_cgt_type_set.find(type) != basic_cgt_type_set.end();
}

} // namespace

//---------------------------------------------------------------------------

sumgame::~sumgame()
{
    // todo delete subgames, or store in vector of std::unique_ptr
    // assert(_play_record_stack.empty());
}

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

void sumgame::add(std::vector<game*>& gs)
{
    for (game* g : gs)
    {
        add(g);
    }
}

void sumgame::pop(const game* g)
{
    assert(!_subgames.empty());
    assert(_subgames.back() == g);
    _subgames.pop_back();
}

void sumgame::pop(const std::vector<game*>& gs)
{
    for (auto it = gs.rbegin(); it != gs.rend(); it++)
        pop(*it);
}

const bool PRINT_SUBGAMES = false;

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

optional<solve_result> sumgame::solve_with_timeout(
    unsigned long long timeout) const
{
    timeout_source src;
    timeout_token tok = src.get_timeout_token();

    src.start_timeout(timeout);
    optional<solve_result> result = solve_with_timeout_token(tok, INITIAL_SEARCH_DEPTH);
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

    assert(sum.num_active_games() >= 0);

    _need_cgt_simplify = true;

    optional<solve_result> result = sum._solve_impl(depth);

    sum._undo_pre_solve_pass();

    sum._timeout_tok.reset();

    return result;
}

bool sumgame::solve_with_games(std::vector<game*>& gs) const
{
    assert_restore_sumgame ars(*this);
    sumgame& sum = const_cast<sumgame&>(*this);

    for (game* g : gs)
    {
        sum.add(g);
    }

    bool result = solve();

    const size_t N = gs.size();
    for (size_t i = 0; i < N; i++)
    {
        game* back = sum._pop_game();
        game* g = gs[N - 1 - i];

        assert(back == g);
    }

    return result;
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

optional<solve_result> sumgame::_solve_impl(uint64_t depth)
{
#ifdef SUMGAME_DEBUG
    _debug_extra();
    assert_restore_sumgame ars(*this); // must come before the stack unwinder
#endif

    undo_stack_unwinder stack_unwinder(*this);

    if (_over_time())
        return solve_result::invalid();

    stats::report_search_node(*this, to_play(), depth);
    const uint64_t next_depth = depth + 1; // for after a move is played

    if (PRINT_SUBGAMES)
    {
        cout << "solve sum ";
        print(cout);
    }


    temp_vec_opt_t temperatures;

    {
        //simplify_impartial();

        db_replacement_pass();
        simplify_basic();

        std::optional<solve_result> result = simplify_db(temperatures);

        if (result.has_value())
            return result;
    }



    /*      NOTE:
       This is sort of a nested optional. If sumgame's ttable is disabled (i.e.
       it uses 0 index bits), then tt_result doesn't hold a search_result.

       If tt_result holds a search_result, the queried hash may still be a miss.
    */
    std::optional<ttable_sumgame::search_result> tt_result =
        _do_ttable_lookup();

    if (tt_result.has_value() && tt_result->entry_valid())
        return tt_result->get_bool(0);

    const bw toplay = to_play();

    std::unique_ptr<sumgame_move_generator> mgp(
        create_sum_move_generator(toplay, temperatures));

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
            return result;
        }
    }

    if (tt_result.has_value())
    {
        tt_result->init_entry();
        tt_result->set_bool(0, false);
    }

    return solve_result(false);
}

void sumgame::_push_undo_code(sumgame_undo_code code)
{
    _undo_code_stack.push_back(code);
}

void sumgame::_pop_undo_code(sumgame_undo_code code)
{
    assert(!_undo_code_stack.empty());
    assert(_undo_code_stack.back() == code);
    _undo_code_stack.pop_back();
}

std::optional<ttable_sumgame::search_result> sumgame::_do_ttable_lookup() const
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
    std::unordered_set<game*> game_set;

    const size_t n_games = num_total_games();
    for (size_t i = 0; i < n_games; i++)
    {
        game* g = subgame(i);
        auto it = game_set.insert(g);
        assert(it.second == true);
    }
}

bool sumgame::_over_time() const
{
    if (_timeout_tok.has_value())
        return _timeout_tok->stop_requested();
    return false;
}

void sumgame::_pre_solve_pass()
{
    _push_undo_code(SUMGAME_UNDO_PRE_SOLVE_PASS);

    // TODO change records are used in several places, but are kind of messy...
    // make this better
    _change_record_stack.push_back({});
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

game* sumgame::_pop_game()
{
    assert(!_subgames.empty());

    game* back = _subgames.back();
    _subgames.pop_back();

    return back;
}

void sumgame::play_sum(const sumgame_move& sm, bw to_play)
{
    assert(is_black_white(to_play));

    _push_undo_code(SUMGAME_UNDO_PLAY);

    _play_record_stack.push_back(play_record(sm));
    play_record& record = _play_record_stack.back();

    const int subg = sm.subgame_idx;
    const move mv = sm.m;

    game* g = subgame(subg);

    g->play(mv, to_play);
    split_result sr;

    if (global::play_split())
        sr = g->split();

    if (sr) // split changed the sum
    {
        assert(global::play_split());
        record.did_split = true;

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
        if (global::play_normalize())
        {
            g->normalize();

            // TODO has_moves() is maybe slow...
            if (!g->has_moves())
            {
                g->set_active(false);
                record.deactivated_g = true;
            }
        }

    }

    alternating_move_game::play(mv);
}

void sumgame::undo_move()
{
    _pop_undo_code(SUMGAME_UNDO_PLAY);

    play_record& record = _play_record_stack.back();

    const sumgame_move sm = record.sm;
    const int subg = sm.subgame_idx;
    game* s = subgame(subg);

    // undo split (if necessary)
    if (record.did_split)
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
            // ensure we're deleting the same game that was added earlier
            assert(g == _subgames.back());
            // a previous undo should have reactivated g
            assert(_subgames.back()->is_active());

            // No need for g->undo_normalize() -- g is being deleted
            pop(g);
            delete g;
        }
    }
    else
    {
        if (global::play_normalize())
        {
            if (record.deactivated_g)
                s->set_active(true);

            s->undo_normalize();
        }
    }

    const move subm = cgt_move::remove_color(s->last_move());

    assert(                                                         //
        sm.m == subm ||                                             //
        (                                                           //
            (cgt_move::remove_color(sm.m) == subm) &&                     //
            (s->game_type() == game_type<impartial_game_wrapper>()) //
            )                                                       //
    );                                                              //

    s->undo_move();
    alternating_move_game::undo_move();

    _play_record_stack.pop_back();
}

void sumgame::simplify_basic()
{
    if (!global::simplify_basic_cgt())
        return;

    if (!_need_cgt_simplify)
        return;

    _change_record_stack.emplace_back();
    change_record& record = _change_record_stack.back();

    record.simplify_basic(*this);
    _need_cgt_simplify = false;

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

namespace {
std::vector<unsigned int> get_oc_indexable_vector()
{
    std::vector<unsigned int> vec;

    static const outcome_class OC_MAX =
        std::max({
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
ebw analyze_outcome_count_vector(const std::vector<unsigned int>& counts,
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

} // namespace


void sumgame::simplify_impartial()
{
    _push_undo_code(SUMGAME_UNDO_SIMPLIFY_IMPARTIAL);

    if (!global::use_db())
        return;

    database& db = get_global_database();

    // Push record
    _change_record_stack.emplace_back();
    sumgame_impl::change_record& cr = _change_record_stack.back();

    int n_known_values = 0; // nimbers and impartial_games
    int n_known_non_nimbers = 0; // known values which are not nimbers

    int final_nim_sum = 0;

    const int N_SUBGAMES = num_total_games();
    for (int i = 0; i < N_SUBGAMES; i++)
    {
        game* sg = subgame(i);
        if (!sg->is_active() || !sg->is_impartial())
            continue;

        assert(dynamic_cast<impartial_game*>(sg) != nullptr);

        // Don't need to look up nimber
        if (sg->game_type() == game_type<nimber>())
        {
            assert(dynamic_cast<nimber*>(sg) != nullptr);
            nimber* sg_nimber = static_cast<nimber*>(sg);

            n_known_values++;
            nimber::add_nimber(final_nim_sum, sg_nimber->value());

            // Actually deactivate later
            cr.deactivated_games.push_back(sg);
            continue;
        }

        assert(dynamic_cast<nimber*>(sg) == nullptr);

        std::optional<db_entry_impartial> entry = db.get_impartial(*sg);
        stats::report_db_access(entry.has_value());

        if (!entry.has_value())
            continue;

        n_known_values++;
        n_known_non_nimbers++;
        // Actually deactivate later
        cr.deactivated_games.push_back(sg);
        nimber::add_nimber(final_nim_sum, entry.value().nim_value);
    }

    if (n_known_values < 2 && n_known_non_nimbers == 0)
    {
        cr.deactivated_games.clear();
        assert(cr.added_games.empty());
        return;
    }

    for (game* sg : cr.deactivated_games)
    {
        assert(sg->is_active());
        sg->set_active(false);
    }

    if (final_nim_sum != 0)
    {
        nimber* nim_sum_as_nimber = new nimber(final_nim_sum);

        cr.added_games.push_back(nim_sum_as_nimber);
        add(nim_sum_as_nimber);
    }
}

void sumgame::undo_simplify_impartial()
{
    _pop_undo_code(SUMGAME_UNDO_SIMPLIFY_IMPARTIAL);

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

optional<solve_result> sumgame::simplify_db(
    temp_vec_opt_t& temperatures)
{
    assert(!temperatures.has_value());
    _push_undo_code(SUMGAME_UNDO_SIMPLIFY_DB);

    if (!global::use_db())
        return {};

    _change_record_stack.emplace_back();
    sumgame_impl::change_record& cr = _change_record_stack.back();

    database& db = get_global_database();

#ifdef MCGS_USE_THERM
    bool at_least_one_temp = false;
    temperatures.emplace();
    temperatures->resize(num_total_games());
#endif

    std::vector<unsigned int> counts = get_oc_indexable_vector();

    // Search all active games
    const int N_SUBGAMES = num_total_games();
    int n_active_games = 0;

    for (int subgame_idx = 0; subgame_idx < N_SUBGAMES; subgame_idx++)
    {
        game* g = subgame(subgame_idx);
        if (!g->is_active())
            continue;

        n_active_games++;

        // Get g's outcome class
        outcome_class oc = outcome_class::U;

        if (g->is_impartial())
        {
            /*
               Because simplify_impartial() should be called prior to this
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
        }
        else
        {
            std::optional<db_entry_partisan> entry = db.get_partisan(*g);
            stats::report_db_access(entry.has_value());

            if (entry.has_value())
            {
                oc = entry->outcome;

#ifdef MCGS_USE_THERM
                assert(temperatures.has_value());
                optional<ThValue>& temp = (*temperatures)[subgame_idx];
                temp = entry->thermograph->Temperature();
                at_least_one_temp = true;
#endif
            }
        }

        counts[oc]++;

        if (oc == outcome_class::P)
        {
            cr.deactivated_games.push_back(g);
            assert(g->is_active());
            g->set_active(false);
        }
    }

#ifdef MCGS_USE_THERM
    if (!at_least_one_temp)
        temperatures.reset();
#endif

    assert(std::accumulate(counts.begin(), counts.end(), 0) == n_active_games);

    const ebw winner = analyze_outcome_count_vector(counts, to_play());

    if (winner == EMPTY)
        return {};

    return solve_result(winner == to_play());
}

void sumgame::undo_simplify_db()
{
    _pop_undo_code(SUMGAME_UNDO_SIMPLIFY_DB);

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

            std::optional<db_entry_impartial> entry = db.get_impartial(*sg);
            stats::report_db_access(entry.has_value());

            if (!entry.has_value())
                continue;

            n_known_imp++;
            n_known_imp_non_nimber++;

            // Actually deactivate later
            deactivated_impartial_games.push_back(sg);
            nimber::add_nimber(final_nim_sum, entry.value().nim_value);
        }
        else
        {
#ifdef MCGS_USE_BOUNDS
            // sg is partisan
            std::optional<db_entry_partisan> entry = db.get_partisan(*sg);
            stats::report_db_access(entry.has_value());

            if (!entry.has_value())
                continue;

            if (!entry->bounds_data.has_value())
                continue;

            const bound_scale scale = std::get<0>(*entry->bounds_data);
            const game_bounds_ptr bounds = std::get<1>(*entry->bounds_data);

            if (!bounds->is_equal())
                continue;

            game* sg_replacement = get_scale_game(bounds->get_lower(), scale);
            add(sg_replacement);
            cr.added_games.push_back(sg_replacement);

            sg->set_active(false);
            cr.deactivated_games.push_back(sg);
#endif
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

std::optional<solve_result> sumgame::db_analyze_pass()
{

}

void sumgame::undo_db_analyze_pass()
{

}

void sumgame::print(std::ostream& str) const
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
    str << std::endl;
}

hash_t sumgame::get_global_hash(bool invalidate_game_hashes) const
{
    return _sumgame_hash.get_global_hash_value(subgames(), to_play());
}

hash_t sumgame::get_global_hash_for_player(ebw for_player, bool invalidate_game_hashes) const
{
    assert(is_empty_black_white(for_player));
    return _sumgame_hash.get_global_hash_value(subgames(), for_player);;
}

bool sumgame::all_impartial() const
{
    return all_games_impartial(_subgames);
}

std::optional<sumgame_move> sumgame::get_winning_or_random_move(
    bw for_player) const
{
    assert(is_black_white(for_player));
    assert_restore_sumgame ars(*this);

    const bw prev_player = to_play();

    sumgame& sum = const_cast<sumgame&>(*this);
    sum.set_to_play(for_player);

    std::unique_ptr<sumgame_move_generator> gen(
        sum.create_sum_move_generator(for_player));

    std::vector<sumgame_move> moves;

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
    // Call helper once so that its static member is already stored when we
    // use it during search
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
                cr.added_games.push_back(sg);
            }
        }
        else
            g->normalize();
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

    const int n_games = num_total_games();
    for (int i = 0; i < n_games; i++)
    {
        game* g = subgame(i);
        if (g->is_active())
            g->undo_normalize();
    }

    for (game* g : cr.deactivated_games)
    {
        assert(!g->is_active());
        g->set_active(true);
    }
    cr.deactivated_games.clear();

    _change_record_stack.pop_back();
}

sumgame_move_generator* sumgame::create_sum_move_generator(bw to_play) const
{
    return new sumgame_move_generator(*this, to_play);
}

sumgame_move_generator* sumgame::create_sum_move_generator(
    bw to_play, temp_vec_opt_t& temperatures) const
{
#ifdef SUMGAME_MOVE_GENERATOR_1_6
    const unsigned int n_games = as_unsigned_unsafe(num_total_games());

    if (temperatures.has_value() && (n_games > temperatures->size()))
        temperatures->resize(n_games);

    return new sumgame_move_generator(*this, to_play, temperatures);
#else
    return new sumgame_move_generator(*this, to_play);
#endif
}

std::ostream& operator<<(std::ostream& out, const sumgame& s)
{
    s.print(out);
    return out;
}

//---------------------------------------------------------------------------

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
