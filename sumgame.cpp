//---------------------------------------------------------------------------
// Sum of combinatorial games and solving algorithms
//---------------------------------------------------------------------------
#include "sumgame.h"

#include <chrono>
#include <ctime>
#include <iostream>
#include <limits>
#include <memory>

#include <thread>
#include <future>

using std::cout;
using std::endl;


//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
class sumgame_move_generator : public move_generator
{
public:
    sumgame_move_generator(const sumgame& game, bw to_play);
    ~sumgame_move_generator();

    void operator++();
    void next_move(bool init);
    operator bool() const;
    sumgame_move gen_sum_move() const;
    move gen_move() const {assert(false);}
private:
    const game* current() const { return _game.subgame(_subgame_idx); }
    const sumgame& _game;
    const int _num_subgames;
    int _subgame_idx;
    move_generator* _subgame_generator;
};

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

//void sumgame_move_generator::skip_no_move_subgames()
//{
//    assert(_subgame_generator);
//}

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
        const game* g = current();

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
        } else
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

sumgame::~sumgame()
{
// todo delete subgames, or store in vector of std::unique_ptr
    assert(_play_record_stack.empty());
}

void sumgame::add(game* g)
{
    _subgames.push_back(g);
    assert(g->is_active());
    //_game_record.push_back(make_pair(ADD_MARKER, 
    //                      &_subgames.back()));
}

const bool PRINT_SUBGAMES = false;

bool sumgame::solve() const
{
    assert_restore_game ar(*this);
    sumgame& sum = 
        const_cast<sumgame&>(*this);

    solve_result result = sum.solve_with_timeout(0);
    assert(result.timed_out() == NOT_OVER_TIME);

    return result.win();
}

// Solve combinatorial game - find winner
// Game-independent implementation of boolean minimax,
// plus sumgame simplification

/*
bool sumgame::_solve()
{
    if (PRINT_SUBGAMES)
    {
        cout << "solve sum ";
        print(cout);
    }
    const bw toplay = to_play();
    std::unique_ptr<sumgame_move_generator>
      mgp(create_sum_move_generator(toplay));
    sumgame_move_generator& mg = *mgp;
    
    for (; mg; ++mg)
    {
        const sumgame_move m = mg.gen_sum_move();
        play_sum(m, toplay);
        bool success = false;
        bool found = find_static_winner(success);
        if (! found)
            success = not solve();
        undo_move();
        if (success)
            return true;
    }
    return false;
}
*/

/*
    Spawns a thread that runs _solve_with_timeout(), then blocks until
        the thread returns, or the timeout has elapsed. It seems using
        std::chrono or clock() to check timeouts is very slow.

        As of writing this, unit tests take 1s to complete when no
        timeout is implemented, 1s when timeout is implemented with threads,
        ~4s with clock(), and ~11s with chrono...

*/
solve_result sumgame::solve_with_timeout(unsigned long long timeout) const
{
    assert_restore_game ar(*this);
    sumgame& sum = const_cast<sumgame&>(*this);

    should_stop = false;

    // spawn a thread, then wait with a timeout for it to complete
    std::promise<solve_result> promise;
    std::future<solve_result> future = promise.get_future();

    std::thread thr([&]() -> void
    {
        solve_result result = sum._solve_with_timeout();
        promise.set_value(result);
    });

    std::future_status status = std::future_status::ready;

    if (timeout == 0)
    {
        future.wait();
    } else
    {
        status = future.wait_for(std::chrono::milliseconds(timeout));
    }

    if (timeout != 0 && status == std::future_status::timeout)
    {
        // Stop the thread
        should_stop = true;
    }

    thr.join();

    return future.get();
}

solve_result sumgame::_solve_with_timeout()
{
    if (over_time())
    {
        return solve_result::invalid();
    }

    if (PRINT_SUBGAMES)
    {
        cout << "solve sum ";
        print(cout);
    }

    const bw toplay = to_play();

    std::unique_ptr<sumgame_move_generator>
      mgp(create_sum_move_generator(toplay));

    sumgame_move_generator& mg = *mgp;
    
    for (; mg; ++mg)
    {
        const sumgame_move m = mg.gen_sum_move();
        play_sum(m, toplay);

        solve_result result(NOT_OVER_TIME, false);

        bool found = find_static_winner(result.win());

        if (! found)
        {
            solve_result child_result = _solve_with_timeout();

            if (!child_result.timed_out())
            {
                result.win() = not child_result.win(); 
            }
        }

        undo_move();

        if (over_time())
        {
            return solve_result::invalid();
        }

        if (result.win())
        {
            return result;
        }
    }
    return {NOT_OVER_TIME, false};
}

bool sumgame::over_time() const
{
    return should_stop;
};

void sumgame::play_sum(const sumgame_move& m, bw to_play)
{
    play_record record(m);

    const int subg = m._subgame_idx;
    const move mv = m._move;

    game* g = subgame(subg);

    g->play(mv, to_play);
    split_result sr = g->split();

    if (sr) // split changed the sum
    {
        record.did_split = true;

        // g is no longer part of the sum
        g->set_active(false);

        for (game* gp : *sr)
        {
            add(gp);
            record.add_game(gp); // save these games in the record for debugging
        }
    }
    
    _play_record_stack.push_back(record);
    alternating_move_game::play(mv);
}

void sumgame::undo_move()
{
    const play_record& record = last_play_record();

    const sumgame_move m = record.move;
    const int subg = m._subgame_idx;
    game* s = subgame(subg);

    // undo split (if necessary)
    if (record.did_split)
    {
        assert(!s->is_active()); // should have been deactivated on last split

        s->set_active(true);

        for (auto it = record.new_games.rbegin(); it != record.new_games.rend(); it++)
        {
            game const* g = *it;
            assert(g == _subgames.back()); // we're deleting the same game
            assert(_subgames.back()->is_active()); // a previous undo should have reactivated g 

            delete _subgames.back();
            _subgames.pop_back();
        }
    }


    const move subm = cgt_move::decode(s->last_move());

    if(!(m._move == subm))
    {
        cout << subg << ' ' << m._move << ' ' << subm << endl;
    }

    assert(m._move == subm);
    s->undo_move();
    alternating_move_game::undo_move();
    _play_record_stack.pop_back();
}

void sumgame::print(std::ostream& str) const
{
    str << "sumgame: " 
        << num_total_games() << " total "
        << num_active_games() << " active: ";
    bool first = true;
    for (auto g: _subgames)
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

sumgame_move_generator* sumgame::create_sum_move_generator(bw to_play) const
{
    return new sumgame_move_generator(*this, to_play);
}
