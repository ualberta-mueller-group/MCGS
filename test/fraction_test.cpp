#include "fraction_test.h"
#include <memory>
#include "cgt_basics.h"
#include "cgt_dyadic_rational.h"
#include "fraction.h"
#include "utilities.h"
#include <sstream>
#include <stdexcept>
#include <cassert>
#include <tuple>
#include <vector>
#include <optional>
#include <functional>

using namespace std;

namespace {
const int TOP_MIN = fraction::TOP_MIN;
const int TOP_MAX = fraction::TOP_MAX;

const int BOTTOM_MAX = (int(1) << (size_in_bits<int>() - 2));

void test_basic()
{
    fraction f1(-7, 8);
    assert(f1.top() == -7);
    assert(f1.bottom() == 8);

    f1.set_top(4);
    assert(f1.top() == 4);
    assert(f1.bottom() == 8);

    f1.set_bottom(16);
    assert(f1.top() == 4);
    assert(f1.bottom() == 16);

    f1.set(-3, 32);
    assert(f1.equals_verbatim({-3, 32}));
    assert(!f1.equals_verbatim({-6, 64}));

    fraction f2(f1);
    fraction f3(0);
    f3 = f2;

    assert(f1.equals_verbatim(f2));
    assert(f3.equals_verbatim(f1));
    assert(f3.equals_verbatim(f1));
}

void test_comparison()
{
    typedef tuple<fraction, fraction, relation, bool, bool, bool, bool, bool,
                  bool>
        test_case_t;
    /*
           One bool per line:
       <
       <=
       ==
       !=
       >
       >=
    */

    // clang-format off
    vector<test_case_t> test_cases
    {
        {{4, 2}, {2}, REL_EQUAL, false, true, true, false, false, true},
		{{3, 4}, {0}, REL_GREATER, false, false, false, true, true, true},
		{{-21, 8}, {53, 256}, REL_LESS, true, true, false, true, false, false},
		{{TOP_MIN}, {TOP_MAX}, REL_LESS, true, true, false, true, false, false},
		{{TOP_MAX}, {TOP_MIN}, REL_GREATER, false, false, false, true, true, true},
		{{TOP_MIN, 2048}, {TOP_MAX, 128}, REL_LESS, true, true, false, true, false, false},
		{{TOP_MIN, 4}, {TOP_MAX, 1024}, REL_LESS, true, true, false, true, false, false},
		{{TOP_MAX, 128}, {TOP_MIN, 2048}, REL_GREATER, false, false, false, true, true, true},
		{{TOP_MAX, 1024}, {TOP_MIN, 4}, REL_GREATER, false, false, false, true, true, true},
		{{0}, {0}, REL_EQUAL, false, true, true, false, false, true},
		{{-2000}, {-25}, REL_LESS, true, true, false, true, false, false},
		{{21, 8}, {5376, 2048}, REL_EQUAL, false, true, true, false, false, true},
    };
    // clang-format on

    for (const test_case_t& test : test_cases)
    {
        const fraction& f1 = get<0>(test);
        const fraction& f2 = get<1>(test);
        const relation& rel_exp = get<2>(test);

        assert(fraction::get_relation(f1, f2) == rel_exp);
        assert((f1 < f2) == get<3>(test));
        assert((f1 <= f2) == get<4>(test));
        assert((f1 == f2) == get<5>(test));
        assert((f1 != f2) == get<6>(test));
        assert((f1 > f2) == get<7>(test));
        assert((f1 >= f2) == get<8>(test));
    }
}

void test_make_dyadic_rational()
{
    auto assert_same_by_text = [](const fraction& frac) -> void
    {
        unique_ptr<dyadic_rational> g1(frac.make_dyadic_rational());
        unique_ptr<dyadic_rational> g2(
            new dyadic_rational(frac.top(), frac.bottom()));

        stringstream str1;
        stringstream str2;

        g1->print(str1);
        g2->print(str2);

        assert(str1.str() == str2.str());
    };

    assert_same_by_text({5});
    assert_same_by_text({-21});
    assert_same_by_text({4, 4});
    assert_same_by_text({9, 2});
    assert_same_by_text({2000, 4});
}

void test_simplification()
{
    {
        fraction f1(8, 2);
        assert(!f1.is_simplified());
        assert(f1.top() == 8 && f1.bottom() == 2);
        f1.simplify();
        assert(f1.is_simplified());
        assert(f1.top() == 4 && f1.bottom() == 1);
    }

    {
        fraction f2(16);
        assert(f2.is_simplified());
        f2.simplify();
        assert(f2.is_simplified());
        assert(f2.top() == 16 && f2.bottom() == 1);
    }

    {
        int x = TOP_MIN % 2 == 0 ? TOP_MIN : TOP_MIN + 1;
        assert(x % 2 == 0);

        fraction f3(x, 2);
        assert(!f3.is_simplified());
        assert(f3.top() == x && f3.bottom() == 2);
        f3.simplify();
        assert(f3.is_simplified());
        assert(f3.top() == x / 2 && f3.bottom() == 1);
    }

    {
        fraction f4(0, 1048576);
        assert(!f4.is_simplified());
        f4.simplify();
        assert(f4.is_simplified());
        assert(f4.top() == 0 && f4.bottom() == 1);
    }

    {
        fraction f5(-1048576, 1048576);
        assert(!f5.is_simplified());
        f5.simplify();
        assert(f5.is_simplified());
        assert(f5.top() == -1 && f5.bottom() == 1);
    }
}

void test_negation()
{
    // clang-format off
    vector<fraction> fractions
    {
        {5},
        {TOP_MIN},
        {TOP_MAX},
        {TOP_MIN, (1 << 20)},
        {TOP_MAX, (1 << 21)},
        {0},
        {0, 8},
    };
    // clang-format on

    for (const fraction& frac : fractions)
    {
        fraction f = frac;

        fraction fn = frac;
        fn.negate();

        assert(-f == fn);
        assert(fn.top() == -f.top());
        assert(fn.bottom() == f.bottom());
    }
}

void test_integral_part()
{
    typedef tuple<fraction, int, fraction> test_case_t;
    /*
       before
       removed integral component
       after
    */

    // clang-format off
    vector<test_case_t> test_cases
    {
        {{5}, 5, {0}},
        {{21, 8}, 2, {5, 8}},
        {{(21 + (1 << 20)), (1 << 17)}, (1 << 3), {21, (1 << 17)}},
        {{-46, 8}, -5, {-3, 4}},
        {{91, 128}, 0, {91, 128}},
        {{TOP_MAX, 2048}, (TOP_MAX / 2048), {TOP_MAX % 2048, 2048}},
        {{0, 2048}, 0, {0, 512}},
        {{917, 256}, 3, {149, 256}},
    };
    // clang-format on

    auto do_test = [](const fraction& before, const int& integral,
                      const fraction& after) -> void
    {
        assert(before.get_integral_part() == integral);
        fraction f = before;
        assert(f.remove_integral_part() == integral);
        assert(f == after);
        assert(f.get_integral_part() == 0);
    };

    for (const test_case_t& test : test_cases)
    {
        const fraction& before = get<0>(test);
        const int& integral = get<1>(test);
        const fraction& after = get<2>(test);

        do_test(before, integral, after);
        do_test(-before, -integral, -after);
    }
}

void test_add_sub()
{
    typedef optional<fraction> input_t;
    typedef optional<fraction> exp_t;

    typedef tuple<fraction, input_t, input_t, exp_t> test_case_t;
    /*
        before
        to add
        to subtract
        result
    */

    // clang-format off
    vector<test_case_t> test_cases
    {
        {{1, 2}, fraction(0), fraction(0), fraction(1, 2)},
        {{1, 2}, fraction(1, 2), fraction(-1, 2), fraction(2, 2)},
        {{TOP_MAX, 1}, fraction(1), fraction(-1), {}},
        {{TOP_MIN, 1}, fraction(-1), fraction(1), {}},
        {{TOP_MAX, 1}, fraction(-1), fraction(1), fraction(TOP_MAX - 1)},
        {{TOP_MIN, 1}, fraction(1), fraction(-1), fraction(TOP_MIN + 1)},

        {{TOP_MAX, 2}, fraction(1, 2), fraction(-1, 2), {}},
        {{TOP_MIN, 2}, fraction(-1, 2), fraction(1, 2), {}},
        {{TOP_MAX, 2}, fraction(-1, 2), fraction(1, 2), fraction(TOP_MAX - 1, 2)},
        {{TOP_MIN, 2}, fraction(1, 2), fraction(-1, 2), fraction(TOP_MIN + 1, 2)},

        {{6, 4}, fraction(1, 2), fraction(-1, 2), fraction(4, 2)},
        {{6, 16}, fraction(4, 32), fraction(-4, 32), fraction(4, 8)},
        {{7, 32}, fraction(7, 4), fraction(-7, 4), fraction(63, 32)},
        {{64, 512}, fraction(4, 32), fraction(-4, 32), fraction(2, 8)},
    };
    // clang-format on

    for (const test_case_t& test : test_cases)
    {
        const fraction& before = get<0>(test);
        const input_t& to_add = get<1>(test);
        const input_t& to_sub = get<2>(test);
        const exp_t& exp = get<3>(test);

        if (to_add.has_value())
        {
            fraction f1(before);
            fraction f2(to_add.value());

            bool success = fraction::safe_add_fraction(f1, f2);
            assert(success == exp.has_value());

            if (success)
                assert(f1.equals_verbatim(exp.value()));
            else
                assert(f1 == before);

            assert(f2 == to_add.value());
        }

        if (to_sub.has_value())
        {
            fraction f1(before);
            fraction f2(to_sub.value());

            bool success = fraction::safe_subtract_fraction(f1, f2);
            assert(success == exp.has_value());

            if (success)
                assert(f1.equals_verbatim(exp.value()));
            else
                assert(f1 == before);

            assert(f2 == to_sub.value());
        }
    }
}

void test_compatibility()
{
    {
        fraction f1(4, 8);
        fraction f2(8, 16);

        assert(fraction::make_compatible(f1, f2));
        assert(f1.top() == 1 && f1.bottom() == 2);
        assert(f2.top() == 1 && f2.bottom() == 2);

        f1.set(5, 8);
        f2.set(21, 128);
        assert(fraction::make_compatible(f1, f2));
        assert(f1.top() == 80 && f1.bottom() == 128);
        assert(f2.top() == 21 && f2.bottom() == 128);

        f1.set(0, 1024);
        f2.set(6, 1);
        assert(fraction::make_compatible(f1, f2));
        assert(f1.top() == 0 && f1.bottom() == 1);
        assert(f2.top() == 6 && f2.bottom() == 1);
    }
}

void test_raise_denominator()
{

    typedef optional<int> input_t;
    typedef optional<fraction> exp_t;

    typedef tuple<fraction, input_t, input_t, exp_t> test_case_t;
    /*
        before
        input of raise_denominator_by_pow2
        input of raise_denominator_to
        expected result
    */

    // clang-format off
    vector<test_case_t> test_cases
    {
        {{TOP_MAX, 1}, 1, 2, {}},
        {{TOP_MIN, 1}, 1, 2, {}},
        {{TOP_MIN / 2, 1}, 1, 2, fraction((TOP_MIN / 2) * 2, 2)},
        {{TOP_MAX / 2, 1}, 1, 2, fraction((TOP_MAX / 2) * 2, 2)},
        {{1, BOTTOM_MAX}, 1, 3, {}},
        {{0, BOTTOM_MAX}, 1, 4, {}},
        {{23, 4}, 4, 64, fraction(368, 64)},
        {{-64, 128}, 3, 1024, fraction(-512, 1024)},
        {{7}, 0, 1, fraction(7, 1)},
        {{0}, 3, 8, fraction(0, 8)},
    };
    // clang-format on

    for (const test_case_t& test : test_cases)
    {
        const fraction& before = get<0>(test);
        const input_t& raise_by = get<1>(test);
        const input_t& raise_to = get<2>(test);
        const exp_t& exp = get<3>(test);

        if (raise_by.has_value())
        {
            fraction f(before);
            bool success = f.raise_denominator_by_pow2(raise_by.value());

            assert(success == exp.has_value());
            if (success)
                assert(f.equals_verbatim(exp.value()));
        }

        if (raise_to.has_value())
        {
            fraction f(before);
            bool success = f.raise_denominator_to(raise_to.value());

            assert(success == exp.has_value());
            if (success)
                assert(f.equals_verbatim(exp.value()));
        }
    }
}

void test_mul2_bottom()
{
    typedef optional<fraction> exp_t;

    typedef tuple<fraction, int, exp_t> test_case_t;
    /*
       before
       input to mul2_bottom
       expected result
    */

    // clang-format off
    vector<test_case_t> test_cases
    {
        {{1}, 1, fraction(1, 2)},
        {{1}, 2, fraction(1, 4)},
        {{1}, 3, fraction(1, 8)},
        {{1}, -1, {}},
        {{1}, -2, {}},
        {{1}, -3, {}},
        {{TOP_MAX}, 4, fraction(TOP_MAX, 16)},
        {{TOP_MIN}, 7, fraction(TOP_MIN, 128)},
        {{4, 8}, 2, fraction(4, 32)},
        {{42}, size_in_bits<int>() - 2, fraction(42, BOTTOM_MAX)},
        {{6, 8}, 0, fraction(6, 8)},
    };
    // clang-format on

    for (const test_case_t& test : test_cases)
    {
        const fraction& before = get<0>(test);
        const int& mul2_amount = get<1>(test);
        const exp_t& exp = get<2>(test);

        fraction f(before);
        bool success = f.mul2_bottom(mul2_amount);
        assert(success == exp.has_value());
        if (success)
            assert(f.equals_verbatim(exp.value()));
    }
}

void test_illegal()
{
    typedef tuple<int, int, bool> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases
    {
        {0, 1, false},
        {0, -1, true},
        {1, -1, true},
        {-4, -4, true},
        {-3, 3, true},
        {0, 3, true},
    };
    // clang-format on

    // helper functions which may throw
    function<void(int, int)> throw1 = [](int top, int bottom) -> void
    { fraction f(top, bottom); };

    function<void(int, int)> throw2 = [](int top, int bottom) -> void
    {
        fraction f(0);
        f.set_top(top);
        f.set_bottom(bottom);
    };

    function<void(int, int)> throw3 = [](int top, int bottom) -> void
    {
        fraction f(0);
        f.set(top, bottom);
    };

    auto test_throw = [](function<void(int, int)>& fn, int top, int bottom,
                         bool should_throw) -> void
    {
        bool did_throw = false;

        try
        {
            fn(top, bottom);
        }
        catch (const range_error& e)
        {
            did_throw = true;
        }

        assert(did_throw == should_throw);
    };

    for (const test_case_t& test : test_cases)
    {
        const int& top = get<0>(test);
        const int& bottom = get<1>(test);
        const bool& should_throw = get<2>(test);

        test_throw(throw1, top, bottom, should_throw);
        test_throw(throw2, top, bottom, should_throw);
        test_throw(throw3, top, bottom, should_throw);
    }
}

} // namespace

void fraction_test_all()
{
    assert(BOTTOM_MAX > 0);
    assert((BOTTOM_MAX << 1) < 0);

    test_basic();
    test_comparison();
    test_make_dyadic_rational();
    test_simplification();
    test_negation();
    test_integral_part();
    test_add_sub();
    test_compatibility();
    test_raise_denominator();
    test_mul2_bottom();
    test_illegal();
}
