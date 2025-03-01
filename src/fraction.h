#pragma once
#include "cgt_dyadic_rational.h"
#include <cassert>
#include <ostream>

class dyadic_rational;

////////////////////////////////////////
class fraction
{
public:
    inline fraction(int top, int bottom)
    {
        _init(top, bottom);
    }

    inline fraction(const dyadic_rational& rational)
    {
        _init(rational.p(), rational.q());
    }

    dyadic_rational* make_dyadic_rational(); // owned by caller

    void simplify();
    bool raise_denominator(int target_bottom);

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
