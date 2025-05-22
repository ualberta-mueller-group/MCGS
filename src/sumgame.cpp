//---------------------------------------------------------------------------
// Sum of combinatorial games and solving algorithms
//---------------------------------------------------------------------------
#include "sumgame.h"
#include "cgt_move.h"
#include "game.h"
#include "game_type.h"

#include "cgt_dyadic_rational.h"
#include "cgt_integer_game.h"
#include "cgt_nimber.h"
#include "cgt_switch.h"
#include "cgt_up_star.h"
#include "alternating_move_game.h"

#include <algorithm>
#include <chrono>
#include <optional>
#include <ctime>
#include <iostream>
#include <memory>

#include <thread>
#include <future>

#include <cassert>
#include <unordered_set>
#include <vector>

#include "global_options.h"
#include "hashing.h"
#include "sumgame_undo_stack_unwinder.h"
#include "impartial_game_wrapper.h"

using std::cout;
using std::endl;
using std::optional;
using sumgame_impl::change_record;

std::shared_ptr<ttable_sumgame> sumgame::_tt(nullptr);

//---------------------------------------------------------------------------

sumgame_move_generator::sumgame_move_generator(const sumgame& game, bw to_play)
    : move_generator(to_play),
      _game(game),
      _num_subgames(game.num_total_games()),
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
    next_move(false);
}

void sumgame_move_generator::next_move(bool init)
{
    // increment existing generator and check for move
    if (_subgame_generator != nullptr)
    {
        ++(*_subgame_generator);

        if (*_subgame_generator)
        {
            // we have a move
            return;
        }
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
    for (; _subgame_idx < _num_subgames; _subgame_idx++)
    {
        assert(_subgame_generator == nullptr);
        const game* g = _current();

        // inactive game
        if (!g->is_active())
        {
            continue;
        }

        _subgame_generator = g->create_move_generator(to_play());

        if (*_subgame_generator)
        {
            // found move
            return;
        }
        else
        {
            delete _subgame_generator;
            _subgame_generator = nullptr;
        }
    }
}

sumgame_move_generator::operator bool() const
{
    // do we have a move?
    return _subgame_idx < _num_subgames;
}

sumgame_move sumgame_move_generator::gen_sum_move() const
{
    assert(_subgame_generator);
    return sumgame_move(_subgame_idx, _subgame_generator->gen_move());
}

//---------------------------------------------------------------------------
// Helpers

namespace {

std::unordered_set<game_type_t> get_cgt_type_set()
{
    std::unordered_set<game_type_t> cgt_set;

    cgt_set.insert(game_type<dyadic_rational>());
    cgt_set.insert(game_type<integer_game>());
    cgt_set.insert(game_type<nimber>());
    cgt_set.insert(game_type<switch_game>());
    cgt_set.insert(game_type<up_star>());

    return cgt_set;
}

bool is_simple_cgt(game* g)
{
    // TODO this should be initialized before time is counted
    static const std::unordered_set<game_type_t> CGT_TYPE_SET =
        get_cgt_type_set();

    game_type_t type = g->game_type();
    return CGT_TYPE_SET.find(type) != CGT_TYPE_SET.end();
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
optional<solve_result> sumgame::solve_with_timeout(
    unsigned long long timeout) const
{
    assert_restore_sumgame ars(*this);
    sumgame& sum = const_cast<sumgame&>(*this);

    _should_stop = false;
    _need_cgt_simplify = true;
    for (game* g : _subgames)
        g->normalize();

    // spawn a thread, then wait with a timeout for it to complete
    std::promise<optional<solve_result>> promise;
    std::future<optional<solve_result>> future = promise.get_future();

    std::thread thr([&]() -> void
    {
        optional<solve_result> result = sum._solve_with_timeout();
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

    for (game* g : _subgames)
        g->undo_normalize();
    return future.get();
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

optional<solve_result> sumgame::_solve_with_timeout()
{
#ifdef SUMGAME_DEBUG
    _debug_extra();
    assert_restore_sumgame ars(*this); // must come before the stack unwinder
#endif

    undo_stack_unwinder stack_unwinder(*this);

    if (_over_time())
    {
        return solve_result::invalid();
    }

    if (PRINT_SUBGAMES)
    {
        cout << "solve sum ";
        print(cout);
    }

    simplify_basic();

    // TODO this is quite ugly...
    std::optional<ttable_sumgame::search_result> tt_result = _do_ttable_lookup();
    if (tt_result)
    {
        if (tt_result->entry_valid())
            return tt_result->get_bool(0);
    }

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
            optional<solve_result> child_result = _solve_with_timeout();

            // TODO make a macro to check this and return?
            if (child_result)
            {
                result.win = not child_result.value().win;
            }
        }

        undo_move();

        if (_over_time())
        {
            /// undo_simplify_basic();
            return solve_result::invalid();
        }

        if (result.win)
        {
            /// undo_simplify_basic();

            if (tt_result)
            {
                assert(tt_result.has_value());

                tt_result->init_entry();
                tt_result->set_bool(0, result.win);
            }
            return result;
        }
    }

    /// undo_simplify_basic();
    if (tt_result)
    {
        assert(tt_result.has_value());

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
        return std::optional<ttable_sumgame::search_result>();

    assert(_tt != nullptr);

    const hash_t current_hash = get_global_hash();
    return _tt->search(current_hash);
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
};

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
    if (global::subgame_split())
        sr = g->split();

    if (sr) // split changed the sum
    {
        assert(global::subgame_split());
        record.did_split = true;

        // g is no longer part of the sum
        g->set_active(false);

        for (game* gp : *sr)
        {
            add(gp);             // no need to normalize, add() will do it
            record.add_game(gp); // save these games in the record for debugging
        }
    }
    else
    {
        if (global::tt_sumgame_idx_bits() > 0)
            g->normalize();
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
        assert(global::subgame_split());
        assert(!s->is_active()); // should have been deactivated on last split

        s->set_active(true);

        for (auto it = record.new_games.rbegin(); it != record.new_games.rend();
             it++)
        {
            game const* g = *it;
            // ensure we're deleting the same game that was added earlier
            assert(g == _subgames.back());
            // a previous undo should have reactivated g
            assert(_subgames.back()->is_active());

            pop(g);
            delete g;
        }
    }
    else
    {
        if (global::tt_sumgame_idx_bits() > 0)
            s->undo_normalize();
    }

    const move subm = cgt_move::decode(s->last_move());

    assert(                                                         //
        sm.m == subm ||                                             //
        (                                                           //
            (cgt_move::decode(sm.m) == subm) &&                     //
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
    {
        for (game* g : _subgames)
            g->invalidate_hash();
    }

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

    /*
    auto compare_fn = [](const game* g1, const game* g2) -> bool
    {
        const hash_t hash1 = g1->get_local_hash();
        const hash_t hash2 = g2->get_local_hash();
        return hash1 < hash2;
    };
    */

    auto compare_fn = [](const game* g1, const game* g2) -> bool
    {
        // Put larger games first
        return g1->order(g2) == REL_GREATER;
    };

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

void sumgame::init_ttable(size_t index_bits)
{
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

