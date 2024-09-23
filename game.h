#ifndef game_H
#define game_H

#include <ostream>
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
    game(int color);
    bool solve();
    int to_play() const;
    int opponent() const;
    void set_to_play(int color);
    const vector<move>& move_stack() const;
    
    // Default just returns false, a specific game may override
    virtual bool find_static_winner(bool& success) const;
    virtual void play(const move& m);
    virtual void undo_move();
    virtual move_generator* create_move_generator() const = 0;

private:
    int _to_play;
    vector<move> _move_stack;
}; // game

inline game::game(int color) :
    _to_play(color),
    _move_stack()
{
    assert_black_white(color);
}

inline const vector<move>& game::move_stack() const
{
    return _move_stack;
}

inline void game::play(const move& m)
{
    _move_stack.push_back(m);
    _to_play = ::opponent(_to_play);
}

inline void game::undo_move()
{
    _move_stack.pop_back();
    _to_play = ::opponent(_to_play);
}

inline int game::to_play() const
{
    return _to_play;
}

inline int game::opponent() const
{
    return ::opponent(_to_play);
}

inline void game::set_to_play(int color)
{
    assert_black_white(color);
    _to_play = color;
}

inline bool game::find_static_winner(bool& success) const
{
    return false;
}

std::ostream& operator<<(std::ostream& out, const game& g);

//---------------------------------------------------------------------------
class move_generator
{
public:
//     move_generator(const game& g) = 0;
    virtual ~move_generator() { }
    virtual void operator++() = 0;
    virtual operator bool() const = 0;
    virtual move gen_move() const = 0;
}; // move_generator

#endif // game_H
