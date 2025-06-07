#pragma once
#include "cgt_dyadic_rational.h"
#include "safe_arithmetic.h"
#include <cassert>
#include <limits>
#include <ostream>

class dyadic_rational;

////////////////////////////////////////
class fraction
{
public:
    inline fraction(int top, int bottom) { _init(top, bottom); }

    inline fraction(int top) { _init(top, 1); }

    /*
       Functions below here will never cause overflow. They are
       always successful.

       However, constructing invalid fractions, or setting invalid values,
       will throw an exception.
    */
    fraction(const dyadic_rational& rational);

    dyadic_rational* make_dyadic_rational() const; // owned by caller

    bool is_simplified() const;
    void simplify();

    int get_integral_part() const;
    int remove_integral_part();

    bool operator<(const fraction& rhs) const;
    bool operator<=(const fraction& rhs) const;
    bool operator==(const fraction& rhs) const;
    bool operator!=(const fraction& rhs) const;
    bool operator>(const fraction& rhs) const;
    bool operator>=(const fraction& rhs) const;
    static relation get_relation(const fraction& f1, const fraction& f2);
    static relation get_lexicographic_relation(const fraction& f1,
                                               const fraction& f2);

    fraction operator-() const;
    void negate();

    inline bool equals_verbatim(const fraction& rhs) const
    {
        return (top() == rhs.top()) && (bottom() == rhs.bottom());
    }

    inline const int& top() const { return _top; }

    inline const int& bottom() const { return _bottom; }

    inline void set_top(const int& top)
    {
        _top = top;
        _check_legal();
    }

    inline void set_bottom(const int& bottom)
    {
        _bottom = bottom;
        _check_legal();
    }

    inline void set(const int& top, const int& bottom)
    {
        _top = top;
        _bottom = bottom;
        _check_legal();
    }

    /*
        Functions below here may fail and return false (due to overflow).
            When false, may leave operands more simplified or less simplified
    */
    bool raise_denominator_to(int target_bottom);
    bool raise_denominator_by_pow2(int exponent);
    bool mul2_bottom(int exponent);
    static bool make_compatible(fraction& f1, fraction& f2);
    static bool safe_add_fraction(fraction& x, fraction& y);
    static bool safe_subtract_fraction(fraction& x, fraction& y);

    static const int TOP_MIN = std::numeric_limits<int>::min() + 1;
    static const int TOP_MAX = std::numeric_limits<int>::max();
    static_assert(TOP_MIN == -TOP_MAX);
    static_assert(negate_is_safe(TOP_MIN));
    static_assert(negate_is_safe(TOP_MAX));

private:
    void _check_legal() const;
    void _init(int top, int bottom);

    int _top;
    int _bottom;
};

inline std::ostream& operator<<(std::ostream& os, const fraction& f)
{
    os << f.top() << "/" << f.bottom();
    return os;
}
