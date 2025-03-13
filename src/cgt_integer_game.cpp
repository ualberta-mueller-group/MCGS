//---------------------------------------------------------------------------
// Simple combinatorial games - integers
//---------------------------------------------------------------------------
#include "cgt_integer_game.h"

void integer_game::play(const move& m, bw to_play)
{
    assert(m == INTEGER_MOVE_CODE);
    assert(_value != 0);
    if (to_play == BLACK)
    {
        assert(_value > 0);
        _value -= 1;
    }
    else
    {
        assert(_value < 0);
        _value += 1;
    }
    game::play(INTEGER_MOVE_CODE, to_play);
}

void integer_game::undo_move()
{
    const move m = last_move();
    const bw to_play = cgt_move::get_color(m);
    if (to_play == BLACK)
    {
        assert(_value >= 0);
        _value += 1;
    }
    else
    {
        assert(_value <= 0);
        _value -= 1;
    }
    game::undo_move();
}

game* integer_game::inverse() const
{
    return new integer_game(-_value);
}

void integer_game::print(std::ostream& str) const
{
    str << "integer:" << _value;
}

//---------------------------------------------------------------------------

class integer_move_generator : public move_generator
{
public:
    integer_move_generator(const integer_game& game, bw to_play);
    void operator++() override;
    operator bool() const override;
    move gen_move() const override;
private:
    bool _has_move;
};

integer_move_generator::integer_move_generator(const integer_game& game, bw to_play)
    : move_generator(to_play),
      _has_move(true)
{
    if (((to_play == BLACK) && (game.value() <= 0))
     || ((to_play == WHITE) && (game.value() >= 0)))
        _has_move = false;
}

void integer_move_generator::operator++()
{
    assert(_has_move);
    _has_move = false;
}

integer_move_generator::operator bool() const
{
    return _has_move;
}

move integer_move_generator::gen_move() const
{
    assert(_has_move);
    return INTEGER_MOVE_CODE;
}

//---------------------------------------------------------------------------

move_generator* integer_game::create_move_generator(bw to_play) const
{
    return new integer_move_generator(*this, to_play);
}
