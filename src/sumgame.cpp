//---------------------------------------------------------------------------
// Sum of combinatorial games and solving algorithms
//---------------------------------------------------------------------------
#include "sumgame.h"
#include "cgt_nimber.h"
#include "obj_id.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iostream>
#include <limits>
#include <memory>

#include <thread>
#include <future>

#include "cli_options.h"
#include <unordered_map>

#include "cgt_up_star.h"

using std::cout;
using std::endl;
using std::optional;


//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
class sumgame_move_generator : public move_generator
{
public:
    sumgame_move_generator(const sumgame& game, bw to_play);
    ~sumgame_move_generator();

    void operator++() override;
    void next_move(bool init);
    operator bool() const override;
    sumgame_move gen_sum_move() const;
    move gen_move() const override {assert(false);}
private:
    const game* _current() const { return _game.subgame(_subgame_idx); }
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
    ///assert(_play_record_stack.empty());
}

void sumgame::add(game* g)
{
    _subgames.push_back(g);
    assert(g->is_active());
    //_game_record.push_back(make_pair(ADD_MARKER, 
    //                      &_subgames.back()));
}

void sumgame::add(std::vector<game*>& gs)
{
    for (game* g : gs)
    {
        add(g);
    }
}

const bool PRINT_SUBGAMES = false;

// Solve combinatorial game - find winner
// Game-independent implementation of boolean minimax,
// plus sumgame simplification
bool sumgame::solve() const
{
    assert_restore_game ar(*this);
    sumgame& sum = 
        const_cast<sumgame&>(*this);

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
optional<solve_result> sumgame::solve_with_timeout(unsigned long long timeout) const
{
    assert_restore_game ar(*this);
    sumgame& sum = const_cast<sumgame&>(*this);

    _should_stop = false;

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
    } else
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
    return future.get();
}

bool sumgame::solve_with_games(std::vector<game*>& gs) const
{
    assert_restore_game ar(*this);

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
    assert_restore_game ar(*this);
    sumgame& sum = const_cast<sumgame&>(*this);

    sum.add(g);

    bool result = solve();

    game* back = sum._pop_game();
    assert(back == g);

    return result;
}

optional<solve_result> sumgame::_solve_with_timeout()
{
    if (_over_time())
    {
        return solve_result::invalid();
    }

    if (PRINT_SUBGAMES)
    {
        cout << "solve sum ";
        print(cout);
    }

    simplify();
    undo_simplify();
    return solve_result(false);

    const bw toplay = to_play();

    std::unique_ptr<sumgame_move_generator>
      mgp(create_sum_move_generator(toplay));

    sumgame_move_generator& mg = *mgp;
    
    for (; mg; ++mg)
    {
        const sumgame_move m = mg.gen_sum_move();
        play_sum(m, toplay);

        solve_result result(false);

        bool found = find_static_winner(result.win);

        if (! found)
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
            undo_simplify();
            return solve_result::invalid();
        }

        if (result.win)
        {
            undo_simplify();
            return result;
        }
    }

    undo_simplify();
    return solve_result(false);
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
    _play_record_stack.push_back(play_record(sm));
    play_record& record = _play_record_stack.back();

    const int subg = sm.subgame_idx;
    const move mv = sm.m;

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
    
    alternating_move_game::play(mv);
}

void sumgame::undo_move()
{
    play_record& record = _play_record_stack.back();

    const sumgame_move sm = record.sm;
    const int subg = sm.subgame_idx;
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

    if(!(sm.m == subm))
    {
        cout << subg << ' ' << sm.m << ' ' << subm << endl;
    }

    assert(sm.m == subm);
    s->undo_move();
    alternating_move_game::undo_move();

    _play_record_stack.pop_back();
}

/*
    TODO this definitely needs to be split into multiple smaller functions...

    there may be a tradeoff between readability and performance
*/
void sumgame::simplify()
{
    if (!do_simplification)
    {
        return;
    }

    cout << "Before simplify:" << endl;
    cout << *this << endl;

    _simplify_record_stack.push_back(simplify_record());
    simplify_record& record = _simplify_record_stack.back();

    // Sort all games by type <-- this should be one function
    std::unordered_map<obj_id_t, std::vector<game*>> game_map;

    const int N = num_total_games();
    for (int i = 0; i < N; i++)
    {
        game* g = subgame(i);

        if (!g->is_active())
        {
            continue;
        }

        const obj_id_t obj_id = g->get_obj_id();
        std::vector<game*>& vec = game_map[obj_id];
        vec.push_back(g);
    }

    // Sum together nimbers
    {
        auto it = game_map.find(get_obj_id<nimber>());

        if (it != game_map.end() && it->second.size() >= 1)
        {
            std::vector<game*>& game_vec = it->second;
            std::vector<int> heap_vec;

            for (game* g : game_vec)
            {
                assert(g->is_active() && g->get_obj_id() == get_obj_id<nimber>());
                assert(dynamic_cast<nimber*>(g) != nullptr);

                nimber* g2 = reinterpret_cast<nimber*>(g);
                heap_vec.push_back(g2->value());

                g2->set_active(false);
                record.deactivated_games.push_back(g2);
            }

            game_vec.clear();

            int sum = nimber::nim_sum(heap_vec);
            assert(sum >= 0);

            // if 0 do nothing
            if (sum == 1)
            {
                up_star* new_game = new up_star(0, true);
                game_map[get_obj_id<up_star>()].push_back(new_game);
                _subgames.push_back(new_game);
                record.added_games.push_back(new_game);
            } else if (sum > 1)
            {
                nimber* new_game = new nimber(sum);
                game_map[get_obj_id<nimber>()].push_back(new_game);
                _subgames.push_back(new_game);
                record.added_games.push_back(new_game);
            }
        }

    }

    // Sum together up_stars <-- this should be one function (and should be less verbose)
    {
        auto it = game_map.find(get_obj_id<up_star>());

        if (it != game_map.end() && it->second.size() >= 2)
        {
            std::vector<game*>& up_star_vec = it->second;

            int ups = 0;
            bool star = false;

            for (game* g : up_star_vec)
            {
                assert(g->is_active() && g->get_obj_id() == get_obj_id<up_star>());
                assert(dynamic_cast<up_star*>(g) != nullptr);

                up_star* g2 = reinterpret_cast<up_star*>(g);

                ups += g2->num_ups();
                star ^= g2->has_star();

                g2->set_active(false);
                record.deactivated_games.push_back(g2);
            }

            up_star_vec.clear();

            if (ups != 0 || star)
            {
                game* new_game = new up_star(ups, star);

                record.added_games.push_back(new_game);
                up_star_vec.push_back(new_game);
                add(new_game);
                cout << "ADDED " << *new_game << endl;
            }

        }
    }


}

void sumgame::undo_simplify()
{
    if (!do_simplification)
    {
        return;
    }

    cout << "After simplify:" << endl;
    cout << *this << endl;

    simplify_record& record = _simplify_record_stack.back();

    for (auto it = record.added_games.rbegin(); it != record.added_games.rend(); it++)
    {
        game* g = *it;

        assert(!_subgames.empty());
        assert(g == _subgames.back());
        assert(g->is_active());

        delete g;
        _subgames.pop_back();
    }

    for (game* g : record.deactivated_games)
    {
        assert(std::find(_subgames.begin(), _subgames.end(), g) != _subgames.end());
        assert(!g->is_active());

        g->set_active(true);
    }

    _simplify_record_stack.pop_back();


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

std::ostream& operator<<(std::ostream& out, const sumgame& s)
{
    s.print(out);
    return out;
}
