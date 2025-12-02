//---------------------------------------------------------------------------
// Sum of combinatorial games and solving algorithms
//---------------------------------------------------------------------------
#include "sumgame.h"
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
#include "throw_assert.h"
#include "cgt_up_star.h"
#include "alternating_move_game.h"

#include <algorithm>
#include <chrono>
#include <utility>
#include <optional>
#include <ctime>
#include <iostream>
#include <memory>

#ifndef __EMSCRIPTEN__
#include <thread>
#include <future>
#endif

#include <cassert>
#include <unordered_set>
#include <vector>

#include "global_options.h"
#include "hashing.h"
#include "solver_stats.h"
#include "sumgame_change_record.h"
#include "sumgame_undo_stack_unwinder.h"
#include "impartial_game_wrapper.h"

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

//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------
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

/*
    Spawns a thread that runs _solve_with_timeout(), then blocks until
        the thread returns, or the timeout has elapsed. It seems using
        std::chrono or clock() to check timeouts is very slow.

        As of writing this, unit tests take 1s to complete when no
        timeout is implemented, 1s when timeout is implemented with threads,
        ~4s with clock(), and ~11s with chrono...

*/
#ifndef __EMSCRIPTEN__
optional<solve_result> sumgame::solve_with_timeout(
    unsigned long long timeout) const
{
    assert_restore_sumgame ars(*this);
    sumgame& sum = const_cast<sumgame&>(*this);

    sum._pre_solve_pass();

    {
        const int active = sum.num_active_games();
        THROW_ASSERT(active >= 0);
        stats::set_n_subgames(static_cast<uint64_t>(active));
    }

    _should_stop = false;
    _need_cgt_simplify = true;

    // spawn a thread, then wait with a timeout for it to complete
    std::promise<optional<solve_result>> promise;
    std::future<optional<solve_result>> future = promise.get_future();

    std::thread thr([&]() -> void
    {
        optional<solve_result> result = sum._solve_with_timeout(0);
        promise.set_value(result);
    });

    std::future_status status = std::future_status::ready;

    if (timeout == 0)
    {
        future.wait();
    }
    else
    {
        status = future.wait_for(std::chrono::milliseconds(timeout));
    }

    if (timeout != 0 && status == std::future_status::timeout)
    {
        // Stop the thread
        _should_stop = true;
    }

    future.wait();
    thr.join();

    assert(future.valid());

    sum._undo_pre_solve_pass();

    return future.get();
}
#else
// TODO emscripten pthreads
optional<solve_result> sumgame::solve_with_timeout(
    unsigned long long timeout) const
{
    assert_restore_sumgame ars(*this);
    sumgame& sum = const_cast<sumgame&>(*this);

    sum._pre_solve_pass();

    {
        const int active = sum.num_active_games();
        THROW_ASSERT(active >= 0);
        stats::set_n_subgames(static_cast<uint64_t>(active));
    }

    _should_stop = false;
    _need_cgt_simplify = true;


    optional<solve_result> result = sum._solve_with_timeout(0);

    sum._undo_pre_solve_pass();

    return result;
}
#endif

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

optional<solve_result> sumgame::_solve_with_timeout(uint64_t depth)
{
#ifdef SUMGAME_DEBUG
    _debug_extra();
    assert_restore_sumgame ars(*this); // must come before the stack unwinder
#endif

    undo_stack_unwinder stack_unwinder(*this);

    if (_over_time())
        return solve_result::invalid();

    depth++;
    stats::inc_node_count();
    stats::update_search_depth(depth);
    if (global::count_sums())
        stats::count_sum(*this);

    if (PRINT_SUBGAMES)
    {
        cout << "solve sum ";
        print(cout);
    }

    //cout << "vvvvvvvvvvvvvvvvvvvv\n";
    //cout << *this;
    simplify_impartial();
    //cout << *this;
    //cout << "^^^^^^^^^^^^^^^^^^^^\n";
    //cout << endl;

    {
        std::optional<solve_result> result = simplify_db();

        if (result.has_value())

            if (result.has_value())
                return result;
    }

    simplify_basic();

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
        create_sum_move_generator(toplay));

    sumgame_move_generator& mg = *mgp;

    for (; mg; ++mg)
    {
        const sumgame_move m = mg.gen_sum_move();
        play_sum(m, toplay);

        solve_result result(false);

        bool found = find_static_winner(result.win);

        if (!found)
        {
            optional<solve_result> child_result = _solve_with_timeout(depth);

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

    std::optional<ttable_sumgame::search_result> sr = _tt->search(current_hash);
    assert(sr.has_value());

    stats::tt_access((*sr).entry_valid());

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
    return _should_stop;
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

    _change_record_stack.emplace_back();
    sumgame_impl::change_record& cr = _change_record_stack.back();
   
    database& db = get_global_database();

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

std::optional<solve_result> sumgame::simplify_db()
{
    _push_undo_code(SUMGAME_UNDO_SIMPLIFY_DB);

    if (!global::use_db())
        return {};

    _change_record_stack.emplace_back();
    sumgame_impl::change_record& cr = _change_record_stack.back();

    database& db = get_global_database();

    std::vector<unsigned int> counts = get_oc_indexable_vector();

    // Search all active games
    const int N = num_total_games();
    for (int i = 0; i < N; i++)
    {
        game* g = subgame(i);
        if (!g->is_active())
            continue;

        outcome_class oc = outcome_class::U;

        std::optional<db_entry_partizan> entry = db.get_partizan(*g);
        stats::db_access(entry.has_value());

        if (entry.has_value())
            oc = entry->outcome;

        counts[oc]++;

        if (oc == outcome_class::P)
        {
            cr.deactivated_games.push_back(g);
            g->set_active(false);
        }
    }

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
    if (invalidate_game_hashes)
        for (game* g : _subgames)
            g->invalidate_hash();

    _sumgame_hash.reset();
    _sumgame_hash.set_to_play(to_play());

    std::vector<game*> active_games;

    {
        const int N = this->num_total_games();

        for (int i = 0; i < N; i++)
        {
            game* g = this->subgame(i);

            if (!g->is_active())
                continue;

            active_games.push_back(g);
        }
    }

    auto compare_fn = [](const game* g1, const game* g2) -> bool
    {
        const hash_t hash1 = g1->get_local_hash();
        const hash_t hash2 = g2->get_local_hash();
        return hash1 < hash2;
    };

    /*
    auto compare_fn = [](const game* g1, const game* g2) -> bool
    {
        // Put larger games first
        return g1->order(g2) == REL_GREATER;
    };
    */

    std::sort(active_games.begin(), active_games.end(), compare_fn);

    {
        const size_t N = active_games.size();

        for (size_t i = 0; i < N; i++)
        {
            game* g = active_games[i];
            assert(g->is_active());
            _sumgame_hash.add_subgame(i, g);
        }
    }

    return _sumgame_hash.get_value();
}

bool sumgame::all_impartial() const
{
    const int N = num_total_games();
    for (int i = 0; i < N; i++)
        if (!subgame(i)->is_impartial())
            return false;

    return true;
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

sumgame_move_generator* sumgame::create_sum_move_generator(bw to_play) const
{
    return new sumgame_move_generator(*this, to_play);
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
