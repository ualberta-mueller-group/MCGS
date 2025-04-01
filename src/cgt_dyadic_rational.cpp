//---------------------------------------------------------------------------
// Simple combinatorial games - dyadic rationals
//---------------------------------------------------------------------------
#include "cgt_dyadic_rational.h"
#include "cgt_integer_game.h"
#include "safe_arithmetic.h"
#include "utilities.h"

//---------------------------------------------------------------------------
void dyadic_rational::simplify()
{
    assert(is_power_of_2(_q));

    while ((_p & 0x1) == 0 && (_q & 0x1) == 0)
    {
        _p >>= 1;
        _q >>= 1;
    }
}

dyadic_rational::dyadic_rational(int p, int q) : _p(p), _q(q)
{
    init_game_type_info<dyadic_rational>(*this);
    _check_legal();
    simplify();
}

dyadic_rational::dyadic_rational(const fraction& frac)
    : _p(frac.top()), _q(frac.bottom())
{
    init_game_type_info<dyadic_rational>(*this);
    _check_legal();
    simplify();
}

fraction dyadic_rational::get_fraction() const
{
    return fraction(_p, _q);
}

split_result dyadic_rational::_split_impl() const
{
    if (_q != 1)
    {
        return split_result(); // no split
    }
    else
    {
        return split_result({new integer_game(_p)}); // becomes integer
    }
};

void dyadic_rational::_play_impl(const move& m, bw to_play)
{
    assert(m == cgt_move::two_part_move(_p, _q));
    assert(_p != 0);
    if (to_play == BLACK)
        --_p;
    else
        ++_p;
    simplify();
    //game::play(m, to_play);
}

void dyadic_rational::_undo_move_impl()
{
    const move m = last_move();
    int q;
    bw to_play;
    const int p = cgt_move::decode3(m, &q, &to_play);
    _p = p;
    _q = q;
    //game::undo_move();
}

void dyadic_rational::_init_hash(local_hash& hash)
{
    hash.toggle_tile(0, _p);
    hash.toggle_tile(1, _q);
}

game* dyadic_rational::inverse() const
{
    return new dyadic_rational(-_p, _q);
}

void dyadic_rational::print(std::ostream& str) const
{
    str << "dyadic_rational:" << _p << '/' << _q;
}

void dyadic_rational::_check_legal() const
{
    THROW_ASSERT(_q > 0 && is_power_of_2(_q) && negate_is_safe(_p));
}

//---------------------------------------------------------------------------

class dyadic_rational_move_generator : public move_generator
{
public:
    dyadic_rational_move_generator(const dyadic_rational& game, bw to_play);
    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    const dyadic_rational& _game;
    bool _has_move;
};

dyadic_rational_move_generator::dyadic_rational_move_generator(
    const dyadic_rational& game, bw to_play)
    : move_generator(to_play), _game(game), _has_move(true)
{
    // integer
    if (game.q() == 1)
        if (                                           //
            ((to_play == BLACK) && (game.p() <= 0))    //
            || ((to_play == WHITE) && (game.p() >= 0)) //
            )                                          //
            _has_move = false;
}

void dyadic_rational_move_generator::operator++()
{
    assert(_has_move);
    _has_move = false;
}

dyadic_rational_move_generator::operator bool() const
{
    return _has_move;
}

move dyadic_rational_move_generator::gen_move() const
{
    assert(_has_move);
    return cgt_move::two_part_move(_game.p(), _game.q());
}

//---------------------------------------------------------------------------
move_generator* dyadic_rational::create_move_generator(bw to_play) const
{
    return new dyadic_rational_move_generator(*this, to_play);
}
