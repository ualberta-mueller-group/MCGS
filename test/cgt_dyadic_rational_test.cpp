#include "cgt_dyadic_rational_test.h"
#include "cgt_dyadic_rational.h"

#include <cassert>
#include "cgt_move.h"
#include "test_utilities.h"
#include "utilities.h"

namespace {
namespace cgt_dyadic_rational_test {
void is_power_of_2_test()
{
    assert(is_power_of_2(1));
    assert(is_power_of_2(2));
    assert(is_power_of_2(4));
    assert(is_power_of_2(8));
    assert(is_power_of_2(16));
    assert(is_power_of_2(32));
    assert(is_power_of_2(64));
    assert(is_power_of_2(1024));
    assert(is_power_of_2(32768));
    assert(!is_power_of_2(3));
    assert(!is_power_of_2(5));
    assert(!is_power_of_2(7));
    assert(!is_power_of_2(32767));
    assert(!is_power_of_2(24));
}

void test_constructor1()
{
    dyadic_rational g(1, 2);
    assert_equal(g.p(), 1);
    assert_equal(g.q(), 2);
    assert(fraction(1, 2).equals_verbatim(g.get_fraction()));
}

void test_constructor2()
{
    dyadic_rational g(2, 4);
    assert_equal(g.p(), 1);
    assert_equal(g.q(), 2);
    assert(fraction(1, 2).equals_verbatim(g.get_fraction()));
}

void test_constructor3()
{
    dyadic_rational g(-1, 4);
    assert_equal(g.p(), -1);
    assert_equal(g.q(), 4);
    assert(fraction(-1, 4).equals_verbatim(g.get_fraction()));
}

void test_constructor4()
{
    dyadic_rational g(-28, 1024);
    assert_equal(g.p(), -7);
    assert_equal(g.q(), 256);
    assert(fraction(-7, 256).equals_verbatim(g.get_fraction()));
}

void test_constructor5()
{
    dyadic_rational g(0, 1);
    assert_equal(g.p(), 0);
    assert_equal(g.q(), 1);
    assert(fraction(0, 1).equals_verbatim(g.get_fraction()));
}

void test_constructor6()
{
    dyadic_rational g(0, 2);
    assert_equal(g.p(), 0);
    assert_equal(g.q(), 1);
    assert(fraction(0, 1).equals_verbatim(g.get_fraction()));
}

void test_constructor7()
{
    dyadic_rational g(0, 64);
    assert_equal(g.p(), 0);
    assert_equal(g.q(), 1);
    assert(fraction(0, 1).equals_verbatim(g.get_fraction()));
}

void test_play(int p, int q, bw color, int p2, int q2)
{
    dyadic_rational g(p, q);
    const move m = cgt_move::move2_create(p, q);
    assert_equal(g.p(), p);
    assert_equal(g.q(), q);
    g.play(m, color);
    assert_equal(g.p(), p2);
    assert_equal(g.q(), q2);
    g.undo_move();
    assert_equal(g.p(), p);
    assert_equal(g.q(), q);
}

void test_play1()
{
    test_play(1, 2, BLACK, 0, 1);
}

void test_play2()
{
    test_play(1, 2, WHITE, 1, 1);
}

void test_play3()
{
    test_play(-1, 2, BLACK, -1, 1);
}

void test_play4()
{
    test_play(-1, 2, WHITE, 0, 1);
}

void test_play5()
{
    test_play(7, 8, BLACK, 3, 4);
}

void test_play6()
{
    test_play(7, 8, WHITE, 1, 1);
}

void test_solve1()
{
    dyadic_rational g(1, 2);
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, false);
}

void test_solve2()
{
    dyadic_rational g(-1, 4);
    assert_solve(g, BLACK, false);
    assert_solve(g, WHITE, true);
}

void test_solve3()
{
    dyadic_rational g(0, 1);
    assert_solve(g, BLACK, false);
    assert_solve(g, WHITE, false);
}

} // namespace cgt_dyadic_rational_test

namespace cgt_dyadic_rational_move_generator_test {

void test1()
{
    dyadic_rational g(3, 2);
    assert_num_moves(g, BLACK, 1);
}

void test2()
{
    dyadic_rational g(3, 2);
    assert_num_moves(g, WHITE, 1);
}

void test3()
{
    dyadic_rational g(3, 1);
    assert_num_moves(g, BLACK, 1);
    assert_num_moves(g, WHITE, 0);
}

void test4()
{
    dyadic_rational g(-7, 1);
    assert_num_moves(g, BLACK, 0);
    assert_num_moves(g, WHITE, 1);
}

void test5()
{
    dyadic_rational g(0, 1);
    assert_num_moves(g, BLACK, 0);
    assert_num_moves(g, WHITE, 0);
}

} // namespace cgt_dyadic_rational_move_generator_test
} // namespace

void cgt_dyadic_rational_test_all()
{
    cgt_dyadic_rational_test::is_power_of_2_test();
    cgt_dyadic_rational_test::test_constructor1();
    cgt_dyadic_rational_test::test_constructor2();
    cgt_dyadic_rational_test::test_constructor3();
    cgt_dyadic_rational_test::test_constructor4();
    cgt_dyadic_rational_test::test_constructor5();
    cgt_dyadic_rational_test::test_constructor6();
    cgt_dyadic_rational_test::test_constructor7();
    cgt_dyadic_rational_test::test_play1();
    cgt_dyadic_rational_test::test_play2();
    cgt_dyadic_rational_test::test_play3();
    cgt_dyadic_rational_test::test_play4();
    cgt_dyadic_rational_test::test_play5();
    cgt_dyadic_rational_test::test_play6();
    cgt_dyadic_rational_test::test_solve1();
    cgt_dyadic_rational_test::test_solve2();
    cgt_dyadic_rational_test::test_solve3();
    cgt_dyadic_rational_move_generator_test::test1();
    cgt_dyadic_rational_move_generator_test::test2();
    cgt_dyadic_rational_move_generator_test::test3();
    cgt_dyadic_rational_move_generator_test::test4();
    cgt_dyadic_rational_move_generator_test::test5();
}
