//---------------------------------------------------------------------------
// Simple combinatorial games - switches
//---------------------------------------------------------------------------
#ifndef cgt_switch_H
#define cgt_switch_H

#include "cgt_basics.h"
#include "cgt_integer_game.h"
#include "game.h"

//---------------------------------------------------------------------------

class switch_game : public game
{
public:
    switch_game(int left, int right);
    void play(const move& m, bw to_play);
    void undo_move();
    move_generator* create_move_generator(bw to_play) const;
    
    int left() const { return _left;}
    int right() const { return _right;}
    bool is_integer() const { return _is_integer;}
    int value() const
    {
        assert(is_integer());
        return _int_game.value();
    }

private:
    const int _left, _right;
    bool _is_integer; // has root move been played?
    integer_game _int_game; //only meaningful if _is_integer
};

inline switch_game::switch_game(int left, int right) : 
    _left(left), 
    _right(right),
    _is_integer(false),
    _int_game(0)
{
    assert(left > right);
}

//---------------------------------------------------------------------------
#endif // cgt_switch_H
