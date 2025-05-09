#include "fraction.h"

#include <stdexcept>
#include <string>
#include "cgt_basics.h"
#include "cgt_dyadic_rational.h"
#include "safe_arithmetic.h"
#include <cassert>
#include <type_traits>
#include <algorithm>
#include "utilities.h"

using namespace std;

//////////////////////////////////////// helper functions
namespace {

inline void compute_integral_part(const fraction& frac, int& int_simplified,
                                  int& int_compatible)
{
    int remainder_compatible = frac.top();
    bool success = safe_pow2_mod(remainder_compatible, frac.bottom());
    assert(success);

    int_compatible = frac.top() - remainder_compatible;

    fraction integral(int_compatible, frac.bottom());
    integral.simplify();
    assert(integral.is_simplified());
    int_simplified = integral.top();
}

// common part of two fraction methods
inline bool raise_denominator_common(fraction& frac, int exponent)
{
    assert(exponent >= 0);

    int top_copy = frac.top();
    int bottom_copy = frac.bottom();

    if (!safe_mul2_shift(top_copy, exponent) ||
        !safe_mul2_shift(bottom_copy, exponent))
        return false;

    frac.set(top_copy, bottom_copy);

    return true;
}

} // namespace

//////////////////////////////////////// fraction methods
fraction::fraction(const dyadic_rational& rational)
{
    _init(rational.p(), rational.q());
}

dyadic_rational* fraction::make_dyadic_rational() const
{
    return new dyadic_rational(_top, _bottom);
}

bool fraction::is_simplified() const
{
    return (_top & 0x1) != 0 || (_bottom & 0x1) != 0;
}

void fraction::simplify()
{
    _check_legal();

    static_assert(is_integral_v<decltype(_top)> && is_signed_v<decltype(_top)>);
    static_assert(is_integral_v<decltype(_bottom)> &&
                  is_signed_v<decltype(_bottom)>);

    while (!is_simplified())
    {
        _top >>= 1;
        _bottom >>= 1;
    }
}

int fraction::get_integral_part() const
{
    _check_legal();

    int int_simplified;
    int int_compatible;
    compute_integral_part(*this, int_simplified, int_compatible);

    return int_simplified;
}

int fraction::remove_integral_part()
{
    _check_legal();

    int int_simplified;
    int int_compatible;
    compute_integral_part(*this, int_simplified, int_compatible);

    set_top(top() - int_compatible);
    return int_simplified;
}

bool fraction::operator<(const fraction& rhs) const
{
    return get_relation(*this, rhs) == REL_LESS;
}

bool fraction::operator<=(const fraction& rhs) const
{
    const relation rel = get_relation(*this, rhs);
    return rel == REL_LESS || rel == REL_EQUAL;
}

bool fraction::operator==(const fraction& rhs) const
{
    return get_relation(*this, rhs) == REL_EQUAL;
}

bool fraction::operator!=(const fraction& rhs) const
{
    relation rel = get_relation(*this, rhs);
    return rel == REL_LESS || rel == REL_GREATER;
}

bool fraction::operator>(const fraction& rhs) const
{
    return get_relation(*this, rhs) == REL_GREATER;
}

bool fraction::operator>=(const fraction& rhs) const
{
    const relation rel = get_relation(*this, rhs);
    return rel == REL_GREATER || rel == REL_EQUAL;
}

relation fraction::get_relation(const fraction& lhs, const fraction& rhs)
{
    fraction f1 = lhs;
    fraction f2 = rhs;

    f1._check_legal();
    f2._check_legal();

    int int1 = f1.remove_integral_part();
    int int2 = f2.remove_integral_part();

    if (int1 < int2)
    {
        return REL_LESS;
    }
    if (int1 > int2)
    {
        return REL_GREATER;
    }

    // should always be possible because we removed the integral part
    bool compatible = fraction::make_compatible(f1, f2);
    assert(compatible && f1.bottom() == f2.bottom());

    if (f1.top() < f2.top())
    {
        return REL_LESS;
    }

    if (f1.top() > f2.top())
    {
        return REL_GREATER;
    }

    return REL_EQUAL;
}

relation fraction::get_lexicographic_relation(const fraction& f1, const fraction& f2)
{
    const int& top1 = f1.top();
    const int& top2 = f2.top();

    if (top1 != top2)
        return top1 < top2 ? REL_LESS : REL_GREATER;

    const int& bot1 = f1.bottom();
    const int& bot2 = f2.bottom();

    if (bot1 != bot2)
        return bot1 < bot2 ? REL_LESS : REL_GREATER;

    return REL_EQUAL;
}

fraction fraction::operator-() const
{
    fraction f(*this);
    f.negate();
    return f;
}

void fraction::negate()
{
    _check_legal();
    _top = -_top;
}

bool fraction::raise_denominator_to(int target_bottom)
{
    _check_legal();

    if (target_bottom < bottom() || !is_power_of_2(target_bottom))
        return false;

    int exponent = 0;

    // Find distance to target bottom
    {
        int target_bottom_copy = target_bottom;

        while (target_bottom_copy > bottom())
        {
            assert((target_bottom_copy & 0x1) == 0);
            target_bottom_copy >>= 1;
            exponent += 1;
        }

        assert(target_bottom_copy == bottom());
    }

    bool success = raise_denominator_common(*this, exponent);
    assert(!success || bottom() == target_bottom);

    return success;
}

bool fraction::raise_denominator_by_pow2(int exponent)
{
    _check_legal();

    if (exponent < 0)
        return false;

    return raise_denominator_common(*this, exponent);
}

bool fraction::mul2_bottom(int exponent)
{
    _check_legal();
    return safe_mul2_shift(_bottom, exponent);
}

bool fraction::make_compatible(fraction& f1, fraction& f2)
{
    f1._check_legal();
    f2._check_legal();

    f1.simplify();
    f2.simplify();

    int target = max(f1.bottom(), f2.bottom());

    if (!f1.raise_denominator_to(target) || !f2.raise_denominator_to(target))
        return false;

    return true;
}

bool fraction::safe_add_fraction(fraction& x, fraction& y)
{
    if (!fraction::make_compatible(x, y))
        return false;

    assert(x.bottom() == y.bottom());

    int xtop = x.top();
    int ytop = y.top();

    bool success =
        safe_add(xtop, ytop) && (TOP_MIN <= xtop) && (xtop <= TOP_MAX);

    if (success)
    {
        x.set_top(xtop);
    }

    return success;
}

bool fraction::safe_subtract_fraction(fraction& x, fraction& y)
{
    if (!fraction::make_compatible(x, y))
        return false;

    assert(x.bottom() == y.bottom());

    int xtop = x.top();
    int ytop = y.top();

    bool success =
        safe_subtract(xtop, ytop) && (TOP_MIN <= xtop) && (xtop <= TOP_MAX);

    if (success)
    {
        x.set_top(xtop);
    }

    return success;
}

void fraction::_init(int top, int bottom)
{
    this->_top = top;
    this->_bottom = bottom;

    _check_legal();
}

void fraction::_check_legal() const
{
    if (!(                                       //
            (_bottom > 0) &&                     //
            is_power_of_2(_bottom) &&            //
            (TOP_MIN <= _top && _top <= TOP_MAX) //
            ))                                   //
        throw range_error("Illegal fraction: " + to_string(_top) + "/" +
                          to_string(_bottom));
}
