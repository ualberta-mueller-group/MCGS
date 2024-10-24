//---------------------------------------------------------------------------
// Simple combinatorial games - switches
// Games of the for { l | r }, with integers l > r.
//---------------------------------------------------------------------------
#include "cgt_switch.h"

#include "cgt_integer_game.h"
#include "cgt_move.h"


#include <iostream>
using std::cout;
using std::endl;

const int SWITCH_MOVE_CODE = 1;

void switch_game::play(const move& m, bw to_play)
{
    if (_is_integer)
    {
        assert(m == INTEGER_MOVE_CODE);
        _int_game.play(m, to_play);
    }
    else
    {
        assert(m == SWITCH_MOVE_CODE);
        _is_integer = true;
        _int_game.set_value(to_play == BLACK ? _left : _right);
    }
    game::play(m, to_play);
}

void switch_game::undo_move()
{
    assert(_is_integer);
    const int m = cgt_move::decode(last_move());
    if (m == SWITCH_MOVE_CODE) // back from integer to switch
    {
        _is_integer = false;
    }
    else
    {
        assert(m == INTEGER_MOVE_CODE);
        _int_game.undo_move();
    }
    game::undo_move();
}

//---------------------------------------------------------------------------
class switch_move_generator : public move_generator
{
public:
    switch_move_generator(const switch_game& game, bw to_play);
    void operator++();
    operator bool() const;
    move gen_move() const;
private:
    bool _generated;
};

switch_move_generator::switch_move_generator(const switch_game& game, bw to_play)
    : move_generator(to_play),
      _generated(false)
{ 
    assert(!game.is_integer());
}

void switch_move_generator::operator++()
{
    _generated = true;
}

switch_move_generator::operator bool() const
{
    return !_generated;
}

move switch_move_generator::gen_move() const
{
    assert(!_generated);
    return SWITCH_MOVE_CODE;
}

//---------------------------------------------------------------------------
move_generator* switch_game::create_move_generator(bw to_play) const
{
    if (_is_integer)
        return _int_game.create_move_generator(to_play);
    else
        return new switch_move_generator(*this, to_play);
}
//---------------------------------------------------------------------------


