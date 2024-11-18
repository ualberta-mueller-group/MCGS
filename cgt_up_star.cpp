//---------------------------------------------------------------------------
// Simple combinatorial games - multiples of up, down, with/without star
//---------------------------------------------------------------------------
#include "cgt_up_star.h"

//---------------------------------------------------------------------------
// special cases if -1 <= _value <= 1
void up_star::play(const move& m, bw to_play)
{
    const int delta_v = cgt_move::first(m);
    const bool flip_star = static_cast<bool>(cgt_move::second(m));
    if (_value == 0) // last move from * to 0
    {
        assert(_star);
        assert(flip_star);    
    }
    else
    {
        if (to_play == BLACK)
        {
            if (_value > 0)
                assert(delta_v == -num_ups());
            else
                assert(delta_v == 1);
        } 
        else if (_value > 0)
            assert(delta_v == -1);
        else
            assert(delta_v == -num_ups());
    }
    _value += delta_v;
    if (flip_star)
        _star = ! _star;
    game::play(m, to_play);
}

void up_star::undo_move()
{
    const move m = last_move();
    int flip_star;
    bw to_play;
    const int delta_v = cgt_move::decode3(m, &flip_star, &to_play);
    _value -= delta_v;
    if (flip_star)
        _star = ! _star;
    game::undo_move();
}

void up_star::print(std::ostream& str) const
{
    str << "up_star:" << _value;
    if (has_star())
        str << '*';
}

//---------------------------------------------------------------------------

class up_star_move_generator : public move_generator
{
public:
    up_star_move_generator(const up_star& game, bw to_play);
    void operator++();
    operator bool() const;
    move gen_move() const;
private:
    int compute_num_moves(const up_star& game, bw to_play) const;
    const up_star& _game;
    const int _num_moves;
    int _num_generated;
};

up_star_move_generator::up_star_move_generator(const up_star& game, bw to_play)
    : move_generator(to_play),
      _game(game),
      _num_moves(compute_num_moves(game, to_play)),
      _num_generated(0)
{ }

void up_star_move_generator::operator++()
{
    ++_num_generated;
}

up_star_move_generator::operator bool() const
{
    return _num_moves > _num_generated;
}

int up_star_move_generator::compute_num_moves(
    const up_star& game, bw to_play) const
{
    const int ups = game.num_ups();
    if (ups == 0)
        return game.has_star() ? 1 : 0;
    else if (ups == 1 && game.has_star() && to_play == BLACK)
        return 2;
    else if (ups == -1 && game.has_star() && to_play == WHITE)
        return 2;
    else
        return 1;
}

inline bool is_same_player(int ups, bw to_play)
{
    return (ups > 0) == (to_play == BLACK);
}

inline int sign(int value)
{
    assert(value != 0);
    return (value > 0)? 1 : -1;
}

//value 0: 0, *
// value +1: up = {0 | *}, up* = {0,* | 0}
// value -1: down = {* | 0}, down* = {0 | 0,*}
// value >= 2: up(n) = {0|up(n-1)*}, up(n)* = {0|up(n-1)}
// value <= -2: down(n) = {down(n-1)*|0}, down(n)* = {down(n-1)|0}
move up_star_move_generator::gen_move() const
{
    assert(_num_moves > _num_generated);
    const int ups = _game.num_ups();
    const bool star = _game.has_star();
    move m;
    if (ups == 0) // flip star
        m = cgt_move::two_part_move(0, 1);
    else if (_num_generated == 0) // move to 0 , or flip star
    {
        if (is_same_player(ups, to_play())) // move to 0
            m = cgt_move::two_part_move(-ups, star);
        else // move towards 0 by up_star
            m = cgt_move::two_part_move(sign(-ups), 1);
    }
    else // up* or down* move to *
    {
        assert(ups == 1 || ups == -1);
        m = cgt_move::two_part_move(-ups, 0);
    }
    return m;
}

//---------------------------------------------------------------------------
move_generator* up_star::create_move_generator(bw to_play) const
{
    return new up_star_move_generator(*this, to_play);
}

//---------------------------------------------------------------------------

