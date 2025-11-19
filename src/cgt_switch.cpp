//---------------------------------------------------------------------------
// Simple combinatorial games - switches
// Games of the form { l | r }, with rationals l and r
//---------------------------------------------------------------------------
#include "cgt_switch.h"

#include "cgt_basics.h"
#include "cgt_dyadic_rational.h"
#include "cgt_move.h"
#include "cgt_move_new.h"
#include "safe_arithmetic.h"

#include <iostream>
#include <cassert>

using std::cout;
using std::endl;

void switch_game::play(const move& m, bw to_play)
{
    game::play(m, to_play);

    if (is_rational())
    {
        assert(_move_depth > 0);
        _rational_game->play(m, to_play);
    }
    else
    {
        assert(_move_depth == 0);
        assert(m == SWITCH_MOVE_CODE);
        _rational_game.reset(
            new dyadic_rational(to_play == BLACK ? _left : _right));
    }

    _move_depth++;
}

void switch_game::undo_move()
{
    const int m = cgt_move_new::remove_color(last_move());
    game::undo_move();

    assert(is_rational());
    assert(_move_depth >= 1);

    if (_move_depth == 1) // back from integer to switch
    {
        assert(m == SWITCH_MOVE_CODE);
        _rational_game.reset();
    }
    else
    {
        _rational_game->undo_move();
    }

    _move_depth--;
}

split_result switch_game::_split_impl() const
{
    if (!is_rational())
    {
        return split_result();
    }
    else
    {
        return split_result({new dyadic_rational(value())});
    }
}

void switch_game::_init_hash(local_hash& hash) const
{
    if (!is_rational())
    {
        hash.toggle_value(0, 0);
        hash.toggle_value(1, _left.top());
        hash.toggle_value(2, _left.bottom());

        hash.toggle_value(3, _right.top());
        hash.toggle_value(4, _right.bottom());
    }
    else
    {
        const fraction& f = value();
        hash.toggle_value(0, 1);
        hash.toggle_value(5, f.top());
        hash.toggle_value(6, f.bottom());
    }
}

relation switch_game::_order_impl(const game* rhs) const
{
    const switch_game* other = reinterpret_cast<const switch_game*>(rhs);
    assert(dynamic_cast<const switch_game*>(rhs) == other);

    const switch_kind& kind1 = this->kind();
    const switch_kind& kind2 = other->kind();

    if (kind1 != kind2)
        return kind1 < kind2 ? REL_LESS : REL_GREATER;

    const bool rational = this->is_rational();
    assert(other->is_rational() == rational);

    if (rational)
    {
        const fraction& f1 = this->value();
        const fraction& f2 = other->value();

        return fraction::get_lexicographic_relation(f1, f2);
    }
    else
    {
        const fraction& f1_left = this->left();
        const fraction& f2_left = other->left();

        relation rel_left =
            fraction::get_lexicographic_relation(f1_left, f2_left);

        if (rel_left != REL_EQUAL)
            return rel_left;

        const fraction& f1_right = this->right();
        const fraction& f2_right = other->right();

        relation rel_right =
            fraction::get_lexicographic_relation(f1_right, f2_right);

        return rel_right;
    }

    return REL_EQUAL;
}

game* switch_game::inverse() const
{
    switch_game* inv = new switch_game(-_right, -_left);
    if (is_rational())
        inv->_rational_game.reset(
            new dyadic_rational(-_rational_game->get_fraction()));
    return inv;
}

void switch_game::print(std::ostream& str) const
{
    if (is_rational())
        str << "switch:rational:" << value();
    else
    {
        // Ensure compatibility with old .CSV data:
        // str << "switch:{" << _left << " | " << _right << '}';

        str << "switch:{";

        if (_left.bottom() == 1)
            str << _left.top();
        else
            str << _left;

        str << " | ";

        if (_right.bottom() == 1)
            str << _right.top();
        else
            str << _right;

        str << '}';
    }
}

switch_kind switch_game::kind() const
{
    if (is_rational())
    {
        return SWITCH_KIND_RATIONAL;
    }

    return _kind;
}

fraction switch_game::_init_fraction(const fraction& f) const
{
    fraction f_simplified(f);
    f_simplified.simplify();
    return f_simplified;
}

relation switch_game::_init_relation() const
{
    relation rel = fraction::get_relation(_left, _right);
    assert(rel == REL_EQUAL || rel == REL_LESS || rel == REL_GREATER);
    return rel;
}

switch_kind switch_game::_init_kind() const
{
    assert(_left.is_simplified() && _right.is_simplified());

    if (_rel == REL_LESS || _rel == REL_EQUAL)
    {
        return SWITCH_KIND_CONVERTIBLE_NUMBER;
    }

    assert(negate_is_safe(_right.top()));
    if (_left.bottom() == _right.bottom() && _left.top() == -_right.top())
    {
        return SWITCH_KIND_PROPER_NORMALIZED;
    }

    assert(_rel == REL_GREATER);
    return SWITCH_KIND_PROPER;
}

//---------------------------------------------------------------------------
class switch_move_generator : public move_generator
{
public:
    switch_move_generator(const switch_game& game, bw to_play);
    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    bool _generated;
};

switch_move_generator::switch_move_generator(const switch_game& game,
                                             bw to_play)
    : move_generator(to_play), _generated(false)
{
    assert(!game.is_rational());
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
    if (is_rational())
        return _rational_game->create_move_generator(to_play);
    else
        return new switch_move_generator(*this, to_play);
}

//---------------------------------------------------------------------------
