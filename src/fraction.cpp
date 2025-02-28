#include "fraction.h"

#include <climits>
#include <cstdint>
#include <iostream>
#include <limits>
#include "cgt_dyadic_rational.h"
#include "utilities.h"

using namespace std;

static_assert(int32_t(-1) == int32_t(0xFFFFFFFF), "Not two's complement");
static_assert(numeric_limits<int>::min() < 0);

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


void fraction::_init(int top, int bottom)
{
    assert(bottom > 0);
    assert(is_power_of_2(bottom));

    this->top = top;
    this->bottom = bottom;
}

bool safe_add_fraction(fraction& x, fraction& y)
{
    x.simplify();
    y.simplify();

    int target_bottom = max(x.bottom, y.bottom);

    if (!x.raise_denominator(target_bottom) || !y.raise_denominator(target_bottom))
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



