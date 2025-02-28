#pragma once
#include "cgt_dyadic_rational.h"
#include <cassert>
#include <ostream>

class fraction
{
public:
    inline fraction(int top, int bottom): top(top), bottom(bottom)
    {
        assert(bottom > 0);
        assert(is_power_of_2(bottom));
    }

    void simplify();
    bool raise_denominator(int target_bottom);

    int top;
    int bottom;
};


inline std::ostream& operator<<(std::ostream& os, const fraction& f)
{
    os << f.top << "/" << f.bottom;
    return os;
}


bool safe_add_fraction(fraction& x, fraction& y);
