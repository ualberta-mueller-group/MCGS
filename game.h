//---------------------------------------------------------------------------
// Combinatorial game and move generator
//---------------------------------------------------------------------------

#ifndef game_H
#define game_H

#include <iostream>
#include <string>
#include <vector>
#include "cgt_basics.h"
#include "cgt_move.h"
//---------------------------------------------------------------------------

using std::vector;
//---------------------------------------------------------------------------

class move_generator;
//---------------------------------------------------------------------------

class game
{
public:
    game();
    move last_move() const;
    
    virtual void play(const move& m, bw to_play);
    virtual void undo_move();
    virtual move_generator* create_move_generator(bw to_play) const = 0;
    virtual void print(std::ostream& str) const = 0;

private:
    vector<move> _move_stack;
}; // game

inline game::game() :
    _move_stack()
{ }

inline move game::last_move() const
{
    return _move_stack.back();
}

inline void game::play(const move& m, int to_play)
{
    assert(cgt_move::get_color(m) == 0);
    const move mc = cgt_move::encode(m, to_play);
    _move_stack.push_back(mc);
//     std::cout << "move "<< cgt_move::print(m) << "\n";
//     std::cout << "move + color "<< cgt_move::print(mc) << std::endl;
}

inline void game::undo_move()
{
    _move_stack.pop_back();
}

inline std::ostream& operator<<(std::ostream& out, const game& g)
{
    g.print(out);
    return out;
}

//---------------------------------------------------------------------------
class move_generator
{
public:
    move_generator(bw to_play);
    virtual ~move_generator() { }
    int to_play() const {return _to_play; }
    int opponent() const {return ::opponent(_to_play); }

    virtual void operator++() = 0;
    virtual operator bool() const = 0;
    virtual move gen_move() const = 0;
private:
    const bw _to_play;
}; // move_generator

inline move_generator::move_generator(bw to_play) :
    _to_play(to_play)
{ }
#endif // game_H
