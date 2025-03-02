#include "fraction.h"

#include <climits>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include "cgt_dyadic_rational.h"
#include "utilities.h"

using namespace std;

static_assert(int32_t(-1) == int32_t(0xFFFFFFFF), "Not two's complement");
static_assert(numeric_limits<int>::min() < 0);

//////////////////////////////////////// helper functions

namespace {
bool less_than(const fraction& lhs, const fraction& rhs, bool or_equal)
{
    fraction f1 = lhs;
    fraction f2 = rhs;

    int int1 = f1.remove_integral_part();
    int int2 = f2.remove_integral_part();

    if (int1 < int2)
    {
        return true;
    }
    if (int1 > int2)
    {
        return false;
    }

    // should always be possible because we removed the integral part
    bool compatible = fraction::make_compatible(f1, f2);
    assert(compatible && f1.bottom == f2.bottom);

    return (f1.top < f2.top) || (or_equal && f1.top == f2.top);
}

void compute_integral_part(const fraction& frac, int& int_simplified, int& int_compatible)
{
    int remainder_compatible = pow2_mod(frac.top, frac.bottom);

    int_compatible = frac.top - remainder_compatible;

    fraction integral(int_compatible, frac.bottom);
    integral.simplify();
    assert(integral.bottom == 1);
    int_simplified = integral.top;
}
} // namespace

////////////////////////////////////////

fraction::fraction(const dyadic_rational& rational)
{
    _init(rational.p(), rational.q());
}

dyadic_rational* fraction::make_dyadic_rational() // owned by caller
{
    return new dyadic_rational(top, bottom);
}

void fraction::simplify()
{
    assert(bottom >= 1);
    assert(is_power_of_2(bottom));

    // right shift OK because operands are signed
    static_assert(is_integral_v<decltype(top)> && is_signed_v<decltype(top)>);
    static_assert(is_integral_v<decltype(bottom)> && is_signed_v<decltype(bottom)>);

    while ((top & 0x1) == 0 && (bottom & 0x1) == 0)
    {
        top >>= 1;
        bottom >>= 1;
    }
}


// TODO should have a static_assert to check that abs(INT_MIN) == INT_MAX + 1
bool fraction::raise_denominator(int target_bottom)
{
    assert(target_bottom >= bottom);

    assert(bottom > 0);
    assert(is_power_of_2(bottom));

    assert(target_bottom > 0);
    assert(is_power_of_2(target_bottom));

    int exponent = 0;

    {
        int target_bottom_copy = target_bottom;

        while (target_bottom_copy > bottom)
        {
            assert((target_bottom_copy & 0x1) == 0);
            target_bottom_copy >>= 1;
            exponent += 1;
        }

        assert(target_bottom_copy == bottom);
    }

    int top_copy = top;
    int bottom_copy = bottom;

    if (!safe_mul2_shift(top_copy, exponent) || !safe_mul2_shift(bottom_copy, exponent))
    {
        return false;
    }

    assert(bottom_copy == target_bottom);

    top = top_copy;
    bottom = bottom_copy;

    return true;
}

bool fraction::raise_denominator_by_pow2(int exponent)
{
    int target = bottom;

    if (left_shift_will_wrap(target, exponent))
    {
        return false;
    }

    target <<= exponent;
    return raise_denominator(target);
}

void fraction::negate()
{
    assert(top != std::numeric_limits<decltype(top)>::min());
    top = -top;
}

fraction fraction::operator-() const
{
    fraction f(*this);
    f.negate();
    return f;
}

int fraction::remove_integral_part()
{
    int int_simplified;
    int int_compatible;
    compute_integral_part(*this, int_simplified, int_compatible);

    top -= int_compatible;
    return int_simplified;
}

int fraction::get_integral_part() const
{
    int int_simplified;
    int int_compatible;
    compute_integral_part(*this, int_simplified, int_compatible);

    return int_simplified;
}

bool fraction::operator<(const fraction& rhs) const
{
    return less_than(*this, rhs, false);
}

bool fraction::operator>(const fraction& rhs) const
{
    return less_than(rhs, *this, false);
}

bool fraction::operator==(const fraction& rhs) const
{
    fraction f1 = *this;
    fraction f2 = rhs;

    if (!make_compatible(f1, f2))
    {
        return false;
    }

    assert(f1.bottom == f2.bottom);

    return f1.top == f2.top;
}

bool fraction::operator<=(const fraction& rhs) const
{
    return less_than(*this, rhs, true);
}

bool fraction::operator>=(const fraction& rhs) const
{
    return less_than(rhs, *this, true);
}

bool fraction::make_compatible(fraction& f1, fraction& f2)
{
    f1.simplify();
    f2.simplify();

    int target = max(f1.bottom, f2.bottom);

    if (!f1.raise_denominator(target) || !f2.raise_denominator(target))
    {
        return false;
    }

    return true;
}

void fraction::_init(int top, int bottom)
{
    assert(bottom > 0);
    assert(is_power_of_2(bottom));

    this->top = top;
    this->bottom = bottom;
}

bool safe_add_fraction(fraction& x, fraction& y)
{
    if (!fraction::make_compatible(x, y))
    {
        return false;
    }

    assert(x.bottom == y.bottom);

    if (addition_will_wrap(x.top, y.top))
    {
        return false;
    }

    x.top += y.top;
    return true;
}



bool safe_subtract_fraction(fraction& x, fraction& y)
{
    fraction y_negative = y;

    if (!safe_negate(y_negative.top))
        return false;

    return safe_add_fraction(x, y_negative);
}
