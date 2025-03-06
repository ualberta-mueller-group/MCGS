#include "safe_arithmetic_test.h"
#include "safe_arithmetic.h"

using namespace std;

namespace {

//////////////////////////////////////// helper functions
template <class T>
void test_add_will_wrap(const T& x, const T& y, bool exp)
{
    assert(add_will_wrap(x, y) == exp);
}

template <class T>
void test_safe_add(T x, T y, bool exp)
{
    assert(safe_add(x, y) == exp);
}

template <class T>
void test_safe_add_negatable(T x, T y, bool exp)
{
    assert(safe_add_negatable(x, y) == exp);
}

template <class T>
void test_subtract_will_wrap(const T& x, const T& y, bool exp)
{
    assert(subtract_will_wrap(x, y) == exp);
}

template <class T>
void test_safe_subtract(T x, T y, bool exp)
{
    assert(safe_subtract(x, y) == exp);
}

template <class T>
void test_safe_subtract_negatable(T x, T y, bool exp)
{
    assert(safe_subtract_negatable(x, y) == exp);
}



//////////////////////////////////////// tests
void test_addition_like()
{
    const int32_t& min = numeric_limits<int32_t>::min();
    const int32_t& max = numeric_limits<int32_t>::max();

    typedef tuple<int32_t, int32_t, bool, bool, bool, bool, bool, bool> test_case_t;
    /*
            New bool on each line:

        add_will_wrap, !safe_add
        safe_add_negatable
        subtract_will_wrap(x, y), !safe_subtract(x, y)
        subtract_will_wrap(y, x), !safe_subtract(y, x)
        safe_subtract_negatable(x, y)
        safe_subtract_negatable(y, x)
    */

    vector<test_case_t> test_cases
    {
		{min, -1, true, false, false, false, true, true},
		{min, 0, false, false, false, true, false, false},
		{min, 1, false, true, true, true, false, false},

		{max, -1, false, true, true, false, false, false},
		{max, 1, true, false, false, false, true, true},

		{min, -2, true, false, false, false, true, true},
		{min, 2, false, true, true, true, false, false},

		{max, -2, false, true, true, true, false, false},
		{max, 2, true, false, false, false, true, true},

		{max / 2, max / 2, false, true, false, false, true, true},
		{min / 2, min / 2, false, false, false, false, true, true},

		{min, max, false, true, true, true, false, false},

		{min, min, true, false, false, false, true, true},
		{max, max, true, false, false, false, true, true},
		{26, 91, false, true, false, false, true, true},
    };


    for (const test_case_t& test : test_cases)
    {
        const int32_t& x = get<0>(test);
        const int32_t& y = get<1>(test);
        const bool& exp1 = get<2>(test);
        const bool& exp2 = get<3>(test);
        const bool& exp3 = get<4>(test);
        const bool& exp4 = get<5>(test);
        const bool& exp5 = get<6>(test);
        const bool& exp6 = get<7>(test);

        test_add_will_wrap(x, y, exp1);
        test_add_will_wrap(y, x, exp1);
        test_safe_add(x, y, !exp1);
        test_safe_add(y, x, !exp1);

        test_safe_add_negatable(x, y, exp2);
        test_safe_add_negatable(y, x, exp2);

        test_subtract_will_wrap(x, y, exp3);
        test_safe_subtract(x, y, !exp3);

        test_subtract_will_wrap(y, x, exp4);
        test_safe_subtract(y, x, !exp4);
        
        test_safe_subtract_negatable(x, y, exp5);

        test_safe_subtract_negatable(y, x, exp6);

    }
}

void test_negate()
{
    const int32_t& min = numeric_limits<int32_t>::min();
    const int32_t& max = numeric_limits<int32_t>::max();

    typedef tuple<int32_t, bool> test_case_t;

    vector<test_case_t> test_cases
    {
        {min, true},
        {min + 1, false},
        {max, false},
        {0, false},
        {1244, false},
    };

    for (const test_case_t& test : test_cases)
    {
        const int32_t& x = get<0>(test);
        const bool& exp = get<1>(test);

        assert(negate_will_wrap(x) == exp);
        int32_t y = x;
        assert(safe_negate(y) == !exp);
    }
}


void test_mul2()
{
}

void test_mod()
{
}

} // namespace

void safe_arithmetic_test_all()
{
    test_addition_like();
    test_negate();
    test_mul2();
    test_mod();
}

