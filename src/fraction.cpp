#include "fraction.h"

#include <climits>
#include "cgt_basics.h"
#include "cgt_dyadic_rational.h"
#include "utilities.h"
#include "safe_arithmetic.h"

using namespace std;

//////////////////////////////////////// helper functions
namespace {

inline void compute_integral_part(const fraction& frac, int& int_simplified, int& int_compatible)
{
    assert(frac.is_legal());

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
    assert(frac.is_legal());
    assert(exponent >= 0);

    int top_copy = frac.top();
    int bottom_copy = frac.bottom();

    if (!safe_mul2_shift(top_copy, exponent) || !safe_mul2_shift(bottom_copy, exponent))
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
    assert(is_legal());

    static_assert(is_integral_v<decltype(_top)> && is_signed_v<decltype(_top)>);
    static_assert(is_integral_v<decltype(_bottom)> && is_signed_v<decltype(_bottom)>);

    while (!is_simplified())
    {
        _top >>= 1;
        _bottom >>= 1;
    }
}

int fraction::get_integral_part() const
{
    assert(is_legal());

    int int_simplified;
    int int_compatible;
    compute_integral_part(*this, int_simplified, int_compatible);

    return int_simplified;
}

int fraction::remove_integral_part()
{
    assert(is_legal());

    int int_simplified;
    int int_compatible;
    compute_integral_part(*this, int_simplified, int_compatible);

    set_top(top() - int_compatible);
    return int_simplified;
}

bool fraction::is_legal() const
{
    return (_bottom > 0) && is_power_of_2(_bottom) && !negate_will_wrap(_top);
}

bool fraction::operator<(const fraction& rhs) const
{
    return get_relation(*this, rhs) == REL_LESS;
}

bool fraction::operator>(const fraction& rhs) const
{
    return get_relation(*this, rhs) == REL_GREATER;
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

bool fraction::operator<=(const fraction& rhs) const
{
    const relation rel = get_relation(*this, rhs);
    return rel == REL_LESS || rel == REL_EQUAL;
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

    assert(f1.is_legal() && f2.is_legal());

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

fraction fraction::operator-() const
{
    fraction f(*this);
    f.negate();
    return f;
}

void fraction::negate()
{
    assert(is_legal());
    _top = -_top;
}

bool fraction::raise_denominator_to(int target_bottom)
{
    assert(is_legal());

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
    assert(is_legal());

    if (exponent < 0)
        return false;

    return raise_denominator_common(*this, exponent);
}

bool fraction::make_compatible(fraction& f1, fraction& f2)
{
    assert(f1.is_legal() && f2.is_legal());

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

    return safe_add_negatable(x._top, y._top);
}

bool fraction::safe_subtract_fraction(fraction& x, fraction& y)
{
    if (!fraction::make_compatible(x, y))
        return false;

    assert(x._bottom == y._bottom);

    return safe_subtract_negatable(x._top, y._top);
}

void fraction::_init(int top, int bottom)
{
    this->_top = top;
    this->_bottom = bottom;

    assert(is_legal());
}
