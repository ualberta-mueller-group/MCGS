#include "convert_number.h"
#include "cgt_dyadic_rational.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <sstream>

using namespace std;

/*
    TODO asserts for over/underflow
        Need one for multiplication

    For actual implementation, either use dyadic_rational directly, or a simple
        fraction struct. Passing around 4 ints is more error prone than passing around
        2 fractions.
*/

void simplify(int& numerator, int& denominator)
{
    assert(is_power_of_2(denominator));
    assert(denominator > 0);

    while (numerator != 0 && (numerator & 0x1) == 0 && (denominator & 0x1) == 0)
    {
        numerator >>= 1;
        denominator >>= 1;
    }
}

void make_compatible(int& xn, int& xd, int& yn, int& yd)
{
    assert(xd > 0);
    assert(yd > 0);
    assert(is_power_of_2(xd));
    assert(is_power_of_2(yd));
    // powers of 2 have no remainder when divided

    if (xd != yd)
    {
        while (xd < yd)
        {
            xd <<= 1;
            xn <<= 1;
        }

        while (yd < xd)
        {
            yd <<= 1;
            yn <<= 1;
        }
    }

    assert(xd == yd);
}

int power_of_2_mod(const int& x, const int& m)
{
    assert(is_power_of_2(m));
    assert(m > 0);

    return x & (m - 1);
}

// "n" and "d" denote numerator and denominator
string convert_number(int xn, int xd, int yn, int yd)
{
    assert(xd > 0 && yd > 0);
    assert(is_power_of_2(xd));
    assert(is_power_of_2(yd));

    make_compatible(xn, xd, yn, yd);

    assert(is_power_of_2(xd));
    assert(is_power_of_2(yd));

    assert(xd == yd);
    //assert(xn <= yn);

    // Switch game (invalid case)
    if (!(xn <= yn))
    {
        stringstream str;
        str << "FOUND: SWITCH GAME";
        return str.str();
    }

    // 0 case
    if (xn < 0 && yn > 0)
    {
        stringstream str;
        str << "FOUND: 0 (x == y)";
        return str.str();
    }

    // Equal case
    if (xn == yn)
    {
        stringstream str;
        str << "FOUND: " << xn << "/" << xd << " *";
        return str.str();
    }

    bool swapped = false;

    if (xn < 0)
    {
        swapped = true;

        xn = -xn;
        yn = -yn;
        swap(xn, yn);
    }

    // TODO simple special case?
    if (xn + 1 == yn)
    {
        xn <<= 1;
        xd <<= 1;

        yn <<= 1;
        yd <<= 1;
    }

    assert(xd == yd);
    assert(xn + 1 < yn);
    assert(0 <= xn && xn < yn);

    // Don't extract W, it's not necessary
    const int d = xd;
    const int delta = yn - xn;

    int un = d;

    for (int j = 0; ; j++)
    {
        assert(is_power_of_2(un));

        const int xn2 = power_of_2_mod(xn, un);
        const int yn2 = xn2 + delta;

        if (xn2 < un && un < yn2)
        {
            const int offset = un - xn2;

            int zn = xn + offset;
            int zd = d;

            // Simplify result
            simplify(zn, zd);

            if (swapped)
            {
                zn = -zn;
            }

            stringstream str;
            str << "FOUND: " << zn << "/" << zd;

            return str.str();
        }

        if (un == 1)
        {
            assert(false);
        }

        un >>= 1;
    }
}

int rand_range(int low, int high)
{
    assert(low <= high);
    int dist = high - low;

    int val = rand() % dist;
    val += low;

    return val;
}

void test_convert_number()
{
    srand(5);
    const int TEST_COUNT = 200;

    for (int i = 0; i < TEST_COUNT; )
    {
        const int d_init = 1 << rand_range(0, 28);

        int xd = d_init;
        int yd = d_init;

        int xn = rand_range(-xd, xd);
        int yn = rand_range(-yd, yd);

        int rand_roll = rand_range(0, 100);
        if (rand_roll < 30)
        {
            xd <<= 1;
        } else if (rand_roll < 40)
        {
            xn = yn;
        }

        simplify(xn, xd);
        simplify(yn, yd);

        string out_text = convert_number(xn, xd, yn, yd);

        if (out_text.size() != 0)
        {
            cout << "{" << xn << "/" << xd << " | " << yn << "/" << yd << "}" << endl;
            cout << out_text << endl;
            cout << endl;

            i++;
        }
    }
}
