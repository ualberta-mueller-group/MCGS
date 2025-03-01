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

    if (top == std::numeric_limits<int>::min())
    {
        return false;
    }

    // i.e. 11000...0 (2 bits to avoid changing sign)
    const int mask = int(0x3) << (sizeof(int) * CHAR_BIT - 2);

    auto left_shift_safe = [&]() -> bool
    {
        if ((mask & top) != 0 || (mask & bottom) != 0)
        {
            return false;
        }

        top <<= 1;
        bottom <<= 1;

        return true;
    };

    const int top_copy = top;
    const int bottom_copy = bottom;

    bool flip_sign = false;

    if (top < 0)
    {
        flip_sign = true;

        assert(abs(top) == abs(-top));
        top = -top;
    }

    while (bottom < target_bottom && left_shift_safe())
    { }

    assert(bottom <= target_bottom);

    if (flip_sign)
    {
        assert(abs(top) == abs(-top));
        top = -top;
    }

    if (bottom == target_bottom)
    {
        return true;
    }

    top = top_copy;
    bottom = bottom_copy;
    return false;
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



