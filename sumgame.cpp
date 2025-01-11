//---------------------------------------------------------------------------
// Sum of combinatorial games and solving algorithms
//---------------------------------------------------------------------------
#include "sumgame.h"

#include <iostream>
#include <memory>
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
    return sum._solve();
}

// Solve combinatorial game - find winner
// Game-independent implementation of boolean minimax,
// plus sumgame simplification
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


solve_result sumgame::solve_with_timeout(const double& timeout) const
{
    assert_restore_game ar(*this);
    sumgame& sum = const_cast<sumgame&>(*this);


    timeout_duration = timeout;
    start_time = sumgame_clock::now();

    return sum._solve_with_timeout();
}

solve_result sumgame::_solve_with_timeout()
{
    if (over_time())
    {
        return solve_result::over_time();
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
        if (over_time())
        {
            return solve_result::over_time();
        }

        const sumgame_move m = mg.gen_sum_move();
        play_sum(m, toplay);

        solve_result result(NOT_OVER_TIME, false);

        bool found = find_static_winner(result.win);

        if (! found)
        {
            solve_result child_result = _solve_with_timeout();
            result.win = not child_result.win;
        }

        undo_move();
        if (result.win)
        {
            return result;
        }
    }
    return {NOT_OVER_TIME, false};
}


bool sumgame::over_time() const
{
    std::chrono::time_point<sumgame_clock> now = sumgame_clock::now();
    std::chrono::duration<double, std::milli> duration = now - start_time;
    return duration.count() >= timeout_duration;
}


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
