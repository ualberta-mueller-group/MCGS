//---------------------------------------------------------------------------
// Sum of combinatorial games and solving algorithms
//---------------------------------------------------------------------------
#include "sumgame.h"

#include <iostream>
using std::cout;
using std::endl;

//---------------------------------------------------------------------------
class sumgame_move_generator : public move_generator
{
public:
    sumgame_move_generator(const sumgame& game, bw to_play);
    void operator++();
    operator bool() const;
    sumgame_move gen_sum_move() const;
    move gen_move() const {assert(false);}
private:
    const game* current() const { return _game.subgame(_current_subgame); }
    void skip_no_move_subgames(); // no moves for this player
    const sumgame& _game;
    const int _num_subgames;
    int _current_subgame;
    move_generator* _subgame_generator;
};

sumgame_move_generator::sumgame_move_generator(const sumgame& game, bw to_play)
    : move_generator(to_play),
      _game(game),
      _num_subgames(game.num_total_games()),
      _current_subgame(0),
      _subgame_generator(0)
{
    if (_num_subgames > 0)
    {
        _subgame_generator =
            current()->create_move_generator(to_play);
        skip_no_move_subgames();
    }
}

void sumgame_move_generator::skip_no_move_subgames()
{
    assert(_subgame_generator);
    while ((_current_subgame < _num_subgames)
        && !(*_subgame_generator))
    {
        delete _subgame_generator;
        _subgame_generator = 0;
        ++_current_subgame;
        if (  (_current_subgame < _num_subgames)
            && current()->is_active())
            _subgame_generator =
                current()->create_move_generator(to_play());
    }
}

void sumgame_move_generator::operator++()
{
    if (*_subgame_generator)
        ++(*_subgame_generator);
    if (!(*_subgame_generator))
    {
        skip_no_move_subgames();
    }
}

sumgame_move_generator::operator bool() const
{
    return _current_subgame < _num_subgames;
}

sumgame_move sumgame_move_generator::gen_sum_move() const
{
    assert(_subgame_generator);
    return sumgame_move(_current_subgame, _subgame_generator->gen_move());
}
//---------------------------------------------------------------------------

sumgame::~sumgame()
{
// todo delete subgames, or store in vector of std::unique_ptr
    assert(_sumgame_move_stack.empty());
}

void sumgame::add(game* g)
{
    _subgames.push_back(g);
    assert(g->is_active());
    //_game_record.push_back(make_pair(ADD_MARKER, 
    //                      &_subgames.back()));
}

// Solve combinatorial game - find winner
// Game-independent implementation of boolean minimax,
// plus sumgame simplification
const bool PRINT_SUBGAMES = false;

bool sumgame::solve()
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


void sumgame::play_sum(const sumgame_move& m, bw to_play)
{
    const int subg = m._subgame;
    const move mv = m._move;
    subgame(subg)->play(mv, to_play);
    _sumgame_move_stack.push_back(m);
    alternating_move_game::play(mv);
}

void sumgame::undo_move()
{
    const sumgame_move m = last_sumgame_move();
    const int subg = m._subgame;
    game* s = subgame(subg);
    const move subm = cgt_move::decode(s->last_move());
    if(!(m._move == subm))
    {
        cout << subg << ' ' << m._move << ' ' << subm << endl;
    }
    assert(m._move == subm);
    s->undo_move();
    alternating_move_game::undo_move();
    _sumgame_move_stack.pop_back();
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