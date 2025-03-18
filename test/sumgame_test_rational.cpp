#include "sumgame.h"
#include "cgt_dyadic_rational.h"
#include <cassert>
#include "test_utilities.h"

namespace {

void test_one_rational(dyadic_rational value, bool res_b, bool res_w)
{
    // std::cout << "test " << value << std::endl;
    dyadic_rational i1(value);
    sumgame g(BLACK);
    g.add(&i1);
    assert_solve_sum(g, BLACK, res_b);
    assert_solve_sum(g, WHITE, res_w);
}

void test_two_rationals(dyadic_rational value1, dyadic_rational value2,
                        bool res_b, bool res_w)
{
    // std::cout << "test " << value1 << " + " << value2 << std::endl;
    dyadic_rational i1(value1);
    dyadic_rational i2(value2);
    sumgame g(BLACK);
    g.add(&i1);
    g.add(&i2);
    assert_solve_sum(g, BLACK, res_b);
    assert_solve_sum(g, WHITE, res_w);
}

void test_three_rationals(dyadic_rational value1, dyadic_rational value2,
                          dyadic_rational value3, bool res_b, bool res_w)
{
    //     std::cout << "test " << value1 << " + " << value2
    //               << " + " << value3 << std::endl;
    dyadic_rational i1(value1);
    dyadic_rational i2(value2);
    dyadic_rational i3(value3);
    sumgame g(BLACK);
    g.add(&i1);
    g.add(&i2);
    g.add(&i3);
    assert_solve_sum(g, BLACK, res_b);
    assert_solve_sum(g, WHITE, res_w);
}

dyadic_rational r(int p, int q)
{
    return dyadic_rational(p, q);
}

void test_integers()
{
    test_one_rational(r(0, 1), false, false);
    test_two_rationals(r(0, 1), r(0, 1), false, false);
    test_one_rational(r(1, 1), true, false);
    test_one_rational(r(-1, 1), false, true);
    test_one_rational(r(2, 1), true, false);
    test_two_rationals(r(1, 1), r(-1, 1), false, false);
    test_two_rationals(r(-1, 1), r(1, 1), false, false);
    test_two_rationals(r(2, 1), r(-2, 1), false, false);
    test_two_rationals(r(-2, 1), r(1, 1), false, true);
    test_two_rationals(r(-10, 1), r(10, 1), false, false);
    test_two_rationals(r(-1, 1), r(-1, 1), false, true);
    test_three_rationals(r(0, 1), r(0, 1), r(0, 1), false, false);
    test_three_rationals(r(-1, 1), r(0, 1), r(1, 1), false, false);
    test_three_rationals(r(1, 1), r(2, 1), r(-3, 1), false, false);
    test_three_rationals(r(0, 1), r(0, 1), r(1, 1), true, false);
    test_three_rationals(r(-1, 1), r(-1, 1), r(0, 1), false, true);
    test_three_rationals(r(-10, 1), r(-3, 1), r(13, 1), false, false);
    test_three_rationals(r(1, 1), r(-1, 1), r(-1, 1), false, true);
}

void test_fractions()
{
    test_one_rational(r(1, 2), true, false);
    test_one_rational(r(-1, 2), false, true);
    test_one_rational(r(3, 2), true, false);
    test_one_rational(r(-3, 2), false, true);
    test_one_rational(r(5, 16), true, false);
    test_one_rational(r(-5, 16), false, true);

    test_two_rationals(r(1, 2), r(-1, 2), false, false);
    test_two_rationals(r(1, 4), r(-1, 4), false, false);
    test_two_rationals(r(1, 2048), r(-1, 2048), false, false);
    test_two_rationals(r(1, 2), r(-1, 4), true, false);
    test_two_rationals(r(3, 8), r(-1, 2), false, true);

    test_three_rationals(r(5, 8), r(-11, 16), r(1, 16), false, false);
    test_three_rationals(r(5, 8), r(-11, 16), r(1, 32), false, true);
    test_three_rationals(r(5, 8), r(-11, 16), r(1, 8), true, false);
}

void test_inverses()
{
    auto games = {r(0, 1), r(1, 1), r(1, 16), r(-3, 16), r(57, 8)};
    for (auto g : games)
    {
        std::unique_ptr<game> inv(g.inverse());
        test_inverse(g, *inv);
    }
}

} // namespace

void sumgame_test_rational_all()
{
    test_integers();
    test_fractions();
    test_inverses();
}
