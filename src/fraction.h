#pragma once
#include "cgt_dyadic_rational.h"
#include <cassert>
#include <ostream>
#include <type_traits>

class dyadic_rational;

////////////////////////////////////////
class fraction
{
public:
    inline fraction(int top, int bottom)
    {
        _init(top, bottom);
    }

    inline fraction(int top)
    {
        _init(top, 1);
    }

    fraction(const dyadic_rational& rational);


    dyadic_rational* make_dyadic_rational(); // owned by caller

    void simplify();
    bool raise_denominator(int target_bottom);
    bool raise_denominator_by_pow2(int exponent);

    void negate();
    fraction operator-() const;

    int remove_integral_part();
    int get_integral_part() const;

    bool operator<(const fraction& rhs) const;
    bool operator>(const fraction& rhs) const;
    bool operator==(const fraction& rhs) const;
    bool operator<=(const fraction& rhs) const;
    bool operator>=(const fraction& rhs) const;

    bool is_simplified() const;

    static bool make_compatible(fraction& f1, fraction& f2);
    static relation get_relation(const fraction& f1, const fraction& f2);

    int top;
    int bottom;

private:
    void _init(int top, int bottom);
};


inline std::ostream& operator<<(std::ostream& os, const fraction& f)
{
    os << f.top << "/" << f.bottom;
    return os;
}

bool safe_add_fraction(fraction& x, fraction& y);
bool safe_subtract_fraction(fraction& x, fraction& y);


