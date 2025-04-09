//---------------------------------------------------------------------------
// Implementation of impartial games,
// and algorithms to solve them.
//
// Main changes from game: 
// 1. play(m) does not take a color argument
// 2. move_generator does not take a color argument
// 3. Completely different solving algorithms:
//    Evaluate any impartial game to a nim value
//    - (implemented) brute force mex algorithm
//    - (not yet) Lemoine and Viennot
//    - (not yet) Beling and Rogalski
//---------------------------------------------------------------------------
#pragma once

#include <set>
#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"

//---------------------------------------------------------------------------
class impartial_game : public game
{
public:
    impartial_game();
    // solve single game; store result in _nim_value
    int search_impartial_game() const; 

    // Impartial game interface
    virtual void play(const move& m);
    virtual move_generator* create_move_generator() const = 0;

    // These functions needed by game class interface
    // They also make it possible to include any 
    // impartial game in any (possibly partizan) sum
    void play(const move& m, bw ignore_to_play) override;
    move_generator* create_move_generator(bw ignore_to_play) const override;
    
    bool is_solved() const;
    int nim_value() const; // available after it is solved
    virtual void set_solved(int nim_value);
    
    // Minimum excluded number
    static int mex(const std::set<int>& values);

private:

    using game::play; // avoid compiler warning
    bool _root_is_solved;
    int _nim_value;
};

inline impartial_game::impartial_game() : 
    _root_is_solved(false), _nim_value(0)
{ }

inline bool impartial_game::is_solved() const
{
    return (num_moves_played() == 0) && _root_is_solved;
}

inline int impartial_game::nim_value() const
{
    assert(num_moves_played() == 0);
    assert(_root_is_solved);
    return _nim_value;
}

inline void impartial_game::play(const move& m)
{
    impartial_game::play(m, BLACK);
}

inline void impartial_game::play(const move& m, bw to_play)
{
    game::play(m, to_play);
}

inline move_generator* 
impartial_game::create_move_generator(bw ignore_to_play) const
{
    return create_move_generator();
}
