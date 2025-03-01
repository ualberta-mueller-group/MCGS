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


switch_game::switch_game(dyadic_rational* left, dyadic_rational* right): _left(left->p()), _right(right->p())
{
    cout << "NEW SWITCH: ";
    cout << *left << " " << *right << endl;

    assert(left->q() == 1);
    assert(right->q() == 1);

    delete left;
    delete right;
}

void switch_game::play(const move& m, bw to_play)
{
    if (is_integer())
    {
        assert(m == INTEGER_MOVE_CODE);
        _int_game->play(m, to_play);
    }
    else
    {
        assert(m == SWITCH_MOVE_CODE);
        _int_game.reset(new integer_game(to_play == BLACK ? _left : _right));
    }
    game::play(m, to_play);
}

void switch_game::undo_move()
{
    assert(is_integer());
    const int m = cgt_move::decode(last_move());
    if (m == SWITCH_MOVE_CODE) // back from integer to switch
    {
        _int_game.reset();
    }
    else
    {
        assert(m == INTEGER_MOVE_CODE);
        _int_game->undo_move();
    }
    game::undo_move();
}


split_result switch_game::split_implementation() const
{
    if (!is_integer())
    {
        return split_result();
    } else
    {
        /*
            TODO this returns a new integer_game though we already have one

            maybe ok; not much memory wasted, and switch_game should function
                when split() is disabled by global options
        */
        return split_result({new integer_game(value())});
    }
}

game* switch_game::inverse() const
{
    switch_game* inv = new switch_game(-_right, -_left);
    if (is_integer())
        inv->_int_game.reset(new integer_game(-_int_game->value()));
    return inv;
}

void switch_game::print(std::ostream& str) const
{
    if (is_integer())
        str << "switch:integer:" << value();
    else
        str << "switch:{" << _left << " | " << _right << '}';
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
    if (is_integer())
        return _int_game->create_move_generator(to_play);
    else
        return new switch_move_generator(*this, to_play);
}
//---------------------------------------------------------------------------


