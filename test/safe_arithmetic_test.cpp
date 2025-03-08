#include "safe_arithmetic_test.h"
#include "safe_arithmetic.h"
#include <limits>
#include <cstdint>
#include <iostream>

using namespace std;

namespace {

/*
    Use to simplify writing unit tests for functions which may sometimes not
        return a value.

    Example usage for a function accepting an int32_t, and optionally 
        returning a string:


    typedef exp_result_impl<string> exp_t;
    typedef tuple<int32_t, exp_t> test_case_t;

    vector<test_case_t> test_cases
    {
        {10, {"10"}}, // expected result is "10"
        {-6, {}}, // expected to not return a result
    };
*/
template <class T>
class exp_result_impl
{
public:
    exp_result_impl(const T& val): _has_val(true), _val(val)
    { }

    exp_result_impl(): _has_val(false), _val(0)
    { }

    bool has_value() const
    {
        return _has_val;
    }

    const T& value() const
    {
        assert(_has_val);
        return _val;
    }

private:
    bool _has_val;
    T _val;
};

//////////////////////////////////////// helper functions
template <class T>
void test_add_will_wrap(const T& x, const T& y, bool exp)
{
    assert(add_will_wrap(x, y) == exp);
}

template <class T>
void test_safe_add(T x, T y, const exp_result_impl<T>& exp)
{
    assert(safe_add(x, y) == exp.has_value());
    assert(!exp.has_value() || x == exp.value());
}

template <class T>
void test_safe_add_negatable(T x, T y, const exp_result_impl<T>& exp)
{
    assert(safe_add_negatable(x, y) == exp.has_value());
    assert(!exp.has_value() || x == exp.value());
}

template <class T>
void test_subtract_will_wrap(const T& x, const T& y, bool exp)
{
    assert(subtract_will_wrap(x, y) == exp);
}

template <class T>
void test_safe_subtract(T x, T y, const exp_result_impl<T>& exp)
{
    assert(safe_subtract(x, y) == exp.has_value());
    assert(!exp.has_value() || x == exp.value());
}

template <class T>
void test_safe_subtract_negatable(T x, T y, const exp_result_impl<T>& exp)
{
    assert(safe_subtract_negatable(x, y) == exp.has_value());
    assert(!exp.has_value() || x == exp.value());
}

//////////////////////////////////////// tests
void test_addition_like()
{
    const int32_t& min = numeric_limits<int32_t>::min();
    const int32_t& max = numeric_limits<int32_t>::max();

    typedef exp_result_impl<int32_t> exp_t;
    typedef tuple<int32_t, int32_t, exp_t, exp_t, exp_t, exp_t, exp_t, exp_t> test_case_t;
    /*
       Each line in this comment represents one expected result in the test_case_t

        safe_add, !add_will_wrap
        safe_add_negatable
        safe_subtract(x, y), !subtract_will_wrap(x, y)
        safe_subtract(y, x), !subtract_will_wrap(y, x)

        safe_subtract_negatable(x, y)
        safe_subtract_negatable(y, x)
    */

    vector<test_case_t> test_cases // auto generated from a python script
    {
        {min, -1, {}, {}, {min + 1}, {max}, {min + 1}, {max}},
		{min, 0, {min}, {}, {min}, {}, {}, {}},
		{min, 1, {min + 1}, {min + 1}, {}, {}, {}, {}},

		{max, -1, {max - 1}, {max - 1}, {}, {min}, {}, {}},
		{max, 1, {}, {}, {max - 1}, {min + 2}, {max - 1}, {min + 2}},

		{min, -2, {}, {}, {min + 2}, {max - 1}, {min + 2}, {max - 1}},
		{min, 2, {min + 2}, {min + 2}, {}, {}, {}, {}},

		{max, -2, {max - 2}, {max - 2}, {}, {}, {}, {}},
		{max, 2, {}, {}, {max - 2}, {min + 3}, {max - 2}, {min + 3}},

		{max / 2, max / 2, {max - 1}, {max - 1}, {0}, {0}, {0}, {0}},
		{min / 2, min / 2, {min}, {}, {0}, {0}, {0}, {0}},

		{min, max, {-1}, {-1}, {}, {}, {}, {}},

		{min, min, {}, {}, {0}, {0}, {0}, {0}},
		{max, max, {}, {}, {0}, {0}, {0}, {0}},
		{26, 91, {117}, {117}, {-65}, {65}, {-65}, {65}},
    };

    for (const test_case_t& test : test_cases)
    {
        const int32_t& x = get<0>(test);
        const int32_t& y = get<1>(test);
        const exp_t& exp1 = get<2>(test);
        const exp_t& exp2 = get<3>(test);
        const exp_t& exp3 = get<4>(test);
        const exp_t& exp4 = get<5>(test);
        const exp_t& exp5 = get<6>(test);
        const exp_t& exp6 = get<7>(test);

        test_safe_add(x, y, exp1);
        test_safe_add(y, x, exp1);
        test_add_will_wrap(x, y, !exp1.has_value());
        test_add_will_wrap(y, x, !exp1.has_value());

        test_safe_add_negatable(x, y, exp2);
        test_safe_add_negatable(y, x, exp2);

        test_safe_subtract(x, y, exp3);
        test_subtract_will_wrap(x, y, !exp3.has_value());

        test_safe_subtract(y, x, exp4);
        test_subtract_will_wrap(y, x, !exp4.has_value());

        test_safe_subtract_negatable(x, y, exp5);

        test_safe_subtract_negatable(y, x, exp6);
    }
}

void test_negate()
{
    const int32_t& min = numeric_limits<int32_t>::min();
    const int32_t& max = numeric_limits<int32_t>::max();

    typedef tuple<int32_t, bool> test_case_t;
    /*
       negate_will_wrap(x), !safe_negate(x)
    */

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
        bool safe = safe_negate(y);
        assert(safe == !exp);
        if (safe)
        {
            assert(x == -y && -x == y);
        }
    }
}

void test_mul2_shift()
{
    const int32_t& min = numeric_limits<int32_t>::min();
    const int32_t& max = numeric_limits<int32_t>::max();

    typedef exp_result_impl<int32_t> exp_t;
    typedef tuple<int32_t, int16_t, exp_t> test_case_t;
    /*
        safe_mul2_shift(x, y)
    */

    vector<test_case_t> test_cases
    {
        {min, 0, {}},
        {min + 1, 0, {min + 1}},
        {min + 1, 1, {}},
        {max, 0, {max}},
        {max, 1, {}},

        {max >> 1, 1, {(max >> 1) * 2}},
        {(min + 1), 0, {min + 1}},
        {(min + 1) / 2, 1, {((min + 1) / 2) * 2}},
        {(min + 1) / 2, 2, {}},
        {(min) / 2, 1, {}},
        {(min) / 2, 2, {}},

        {0x3FFFFFFF, 1, {0x3FFFFFFF * 2}}, // 0011...
        {0x30000000, 1, {0x30000000 * 2}},
        {0x30000000, 2, {}},

        {0x4FFFFFFF, 1, {}}, // 0100...
        {0x40000000, 1, {}},
        {0x40000000, 2, {}},

        {0x0FFFFFFF, 3, {0x0FFFFFFF * 8}}, // 0000 11...
        {0x0FFFFFFF, 4, {}},

        {0, 31, {0}},
        {0, 32, {}},
        {0, -1, {}},

        {1, 30, {1073741824}},
        {1, 31, {}},
        {1, 32, {}},
        {1, 33, {}},
    };

    for (const test_case_t& test : test_cases)
    {
        const int32_t& x = get<0>(test);
        const int16_t& y = get<1>(test);
        const exp_t& exp1 = get<2>(test);

        {
            int32_t x2 = x;
            assert(safe_mul2_shift(x2, y) == exp1.has_value());
            assert(!exp1.has_value() || x2 == exp1.value());

            // test negated case if possible
            if (x != min)
            {
                x2 = x;
                assert(safe_negate(x2));
                assert(x2 == -x);
                assert(safe_mul2_shift(x2, y) == exp1.has_value());
                assert(!exp1.has_value() || x2 == -exp1.value());
            }
        }
    }
}

void test_mod()
{
    const int32_t& min = numeric_limits<int32_t>::min();
    const int32_t& max = numeric_limits<int32_t>::max();

    typedef exp_result_impl<int32_t> exp_t;
    typedef tuple<int32_t, int32_t, exp_t> test_case_t;
    /*
        safe_pow2_mod(x, y)
    */

    vector<test_case_t> test_cases
    {
        {21, 0, {}},
        {21, 1, {0}},
        {min, 1, {0}},
        {min, 2, {min % 2}},
        {min, 4, {min % 4}},

        {4, 3, {}},

        {max, 1, {0}},
        {max, 2, {max % 2}},
        {max, 4, {max % 4}},
        {0, 2, {0}}
    };

    for (const test_case_t& test : test_cases)
    {
        const int32_t& x = get<0>(test);
        const int32_t& mod = get<1>(test);
        const exp_t& exp = get<2>(test);

        {
            int32_t x2 = x;
            assert(safe_pow2_mod(x2, mod) == exp.has_value());
            assert(!exp.has_value() || x2 == exp.value());

            // try negative case
            if (x != min)
            {
                x2 = x;
                assert(safe_negate(x2));
                assert(x2 == -x);
                assert(safe_pow2_mod(x2, mod) == exp.has_value());
                assert(!exp.has_value() || x2 == -exp.value());
            }
        }
    }
}

} // namespace

void safe_arithmetic_test_all()
{
    test_addition_like();
    test_negate();
    test_mul2_shift();
    test_mod();
}

