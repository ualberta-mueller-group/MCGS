//---------------------------------------------------------------------------
// Simple combinatorial games - integers
//---------------------------------------------------------------------------
#ifndef cgt_integer_game_H
#define cgt_integer_game_H

#include "cgt_basics.h"
#include "game.h"

// A move for integer_game encodes just the to_play
// This is needed for undo_move in the case when _value has reached 0
class integer_game : public game
{
public:
    integer_game(int value);
    void play(const move& m, bw to_play);
    void undo_move();
    int value() const { return _value; }
    move_generator* create_move_generator(bw to_play) const;
private:
    int _value;
};

inline integer_game::integer_game(int value) : _value(value)
{ }
//---------------------------------------------------------------------------
#endif // cgt_integer_game_H
