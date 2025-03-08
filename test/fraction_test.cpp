#include "fraction_test.h"
#include <iostream>
#include <limits>
#include <memory>
#include "cgt_basics.h"
#include "cgt_dyadic_rational.h"
#include "fraction.h"
#include <sstream>

using namespace std;

const int MIN = numeric_limits<int>::min();
const int MAX = numeric_limits<int>::max();

const int TOP_MIN = fraction::TOP_MIN;
const int TOP_MAX = fraction::TOP_MAX;

namespace {

void test_comparison()
{
    typedef tuple<fraction, fraction, relation, bool, bool, bool, bool, bool, bool> test_case_t;
    /*
           One bool per line:
       <
       <=
       ==
       !=
       >
       >=
    */

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
        unique_ptr<dyadic_rational> g2(new dyadic_rational(frac.top(), frac.bottom()));

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


    for (const fraction& frac : fractions)
    {
        fraction f = frac;

        fraction fn = frac;
        fn.negate();

        assert(-f == fn);
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

    auto do_test = [](const fraction& before, const int& integral, const fraction& after) -> void
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

void test_multiplication_like()
{

}


} // namespace



/*
        comparison operators
        dyadic rational constructor
        simplification
        negation
        integral part
    add/sub
        compatibility
    multiplication-like

    illegal operations

*/


void fraction_test_all()
{
    cout << "FRACTION TEST" << endl;
    test_comparison();
    test_make_dyadic_rational();
    test_simplification();
    test_negation();
    test_integral_part();
    test_add_sub();
    test_compatibility();
    test_multiplication_like();


}
