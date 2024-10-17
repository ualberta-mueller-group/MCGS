#include "cgt_switch.h"

#include <iostream>
using std::cout, std::endl;

void switch_game::play(const move& m, bw to_play)
{
    assert(!_is_integer);
    _is_integer = true;
    _int_value = to_play == BLACK ? _left : _right;
    game::play(m, to_play);
}

void switch_game::undo_move()
{
    _is_integer = false;
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
      _generated(game.is_integer())
{ }

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
    return 0;
}

//---------------------------------------------------------------------------
move_generator* switch_game::create_move_generator(bw to_play) const
{
    return new switch_move_generator(*this, to_play);
}
//---------------------------------------------------------------------------


