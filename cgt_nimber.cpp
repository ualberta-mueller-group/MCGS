#include "cgt_nimber.h"

void nimber::play(const move& m, bw to_play)
{
    const int number = cgt_move::second(m);
    assert(number > 0);
    assert(number <= _value);
    _value -= number;
    game::play(m, to_play);
}


void nimber::undo_move()
{
    const move m = cgt_move::decode(move_stack().back());
    const int number = cgt_move::second(m);
    assert(number > 0);
    _value += number;
    game::undo_move();
}

//---------------------------------------------------------------------------
class nimber_move_generator : public move_generator
{
public:
    nimber_move_generator(const nimber& game, bw to_play);
    void operator++();
    operator bool() const;
    move gen_move() const;
private:
    const nimber& _game;
    int _current_number;
};

nimber_move_generator::nimber_move_generator(const nimber& game, bw to_play)
    : move_generator(to_play),
      _game(game),
      _current_number(1)
{ }

void nimber_move_generator::operator++()
{
    if (_current_number <= _game.value())
        ++_current_number;
}

nimber_move_generator::operator bool() const
{
    return _current_number <= _game.value();
}

move nimber_move_generator::gen_move() const
{
    return cgt_move::two_part_move(0, _current_number);
}

//---------------------------------------------------------------------------
move_generator* nimber::create_move_generator(bw to_play) const
{
    return new nimber_move_generator(*this, to_play);
}
//---------------------------------------------------------------------------

