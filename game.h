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
    const vector<move>& move_stack() const;
    
    virtual void play(const move& m, bw to_play);
    virtual void undo_move();
    virtual move_generator* create_move_generator(bw to_play) const = 0;

private:
    vector<move> _move_stack;
}; // game

inline game::game() :
    _move_stack()
{ }

inline const vector<move>& game::move_stack() const
{
    return _move_stack;
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



std::ostream& operator<<(std::ostream& out, const game& g);

//---------------------------------------------------------------------------
// a alternating_move_game contains a game, and a player to play first
// 
class alternating_move_game
{
public:
    alternating_move_game(game& game, int color);
    bw to_play() const;
    bw opponent() const;
    void set_to_play(int color);
    bool solve();
    game& game_pos() {return _game;}

    // Default just returns false, a specific game may override
    virtual bool find_static_winner(bool& success) const;
    virtual void play(const move& m);
    virtual void undo_move();
private:
    game& _game;
    bw _to_play;
}; // alternating_move_game

inline alternating_move_game::alternating_move_game(game& game, bw to_play) :
    _game(game),
    _to_play(to_play)
{
    assert_black_white(to_play);
}

inline bw alternating_move_game::to_play() const
{
    return _to_play;
}

inline bw alternating_move_game::opponent() const
{
    return ::opponent(_to_play);
}

inline void alternating_move_game::set_to_play(bw to_play)
{
    assert_black_white(to_play);
    _to_play = to_play;
}

inline void alternating_move_game::play(const move& m)
{
    _game.play(m, _to_play);
    _to_play = ::opponent(_to_play);
}

inline void alternating_move_game::undo_move()
{
    _game.undo_move();
    _to_play = ::opponent(_to_play);
}

inline bool alternating_move_game::find_static_winner(bool& success) const
{
    return false;
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
    bw _to_play;
}; // move_generator

inline move_generator::move_generator(bw to_play) :
    _to_play(to_play)
{ }
#endif // game_H
