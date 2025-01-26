#include "cgt_dyadic_rational.h"
#include "cgt_move.h"

#include <cassert>
#include <iostream>
#include "test_utilities.h"

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

namespace cgt_dyadic_rational{

void test_constructor1()
{
    dyadic_rational g(1, 2);
    assert_equal(g.p(), 1);
    assert_equal(g.q(), 2);
}

void test_constructor2()
{
    dyadic_rational g(2, 4);
    assert_equal(g.p(), 1);
    assert_equal(g.q(), 2);
}

void test_constructor3()
{
    dyadic_rational g(-1, 4);
    assert_equal(g.p(), -1);
    assert_equal(g.q(), 4);
}

void test_constructor4()
{
    dyadic_rational g(-28, 1024);
    assert_equal(g.p(), -7);
    assert_equal(g.q(), 256);
}

void test_constructor5()
{
    dyadic_rational g(0, 1);
    assert_equal(g.p(), 0);
    assert_equal(g.q(), 1);
}

void test_constructor6()
{
    dyadic_rational g(0, 2);
    assert_equal(g.p(), 0);
    assert_equal(g.q(), 1);
}

void test_constructor7()
{
    dyadic_rational g(0, 64);
    assert_equal(g.p(), 0);
    assert_equal(g.q(), 1);
}


void test_play(int p, int q, bw color, int p2, int q2)
{
    dyadic_rational g(p, q);
    const move m = cgt_move::two_part_move(p, q);
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

} // namespace cgt_dyadic_rational

namespace cgt_dyadic_rational_move_generator{

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

} // namespace cgt_dyadic_rational_move_generator

void cgt_dyadic_rational_test_all()
{
    is_power_of_2_test();
    cgt_dyadic_rational::test_constructor1();
    cgt_dyadic_rational::test_constructor2();
    cgt_dyadic_rational::test_constructor3();
    cgt_dyadic_rational::test_constructor4();
    cgt_dyadic_rational::test_constructor5();
    cgt_dyadic_rational::test_constructor6();
    cgt_dyadic_rational::test_constructor7();
    cgt_dyadic_rational::test_play1();
    cgt_dyadic_rational::test_play2();
    cgt_dyadic_rational::test_play3();
    cgt_dyadic_rational::test_play4();
    cgt_dyadic_rational::test_play5();
    cgt_dyadic_rational::test_play6();
    cgt_dyadic_rational::test_solve1();
    cgt_dyadic_rational::test_solve2();
    cgt_dyadic_rational::test_solve3();
    cgt_dyadic_rational_move_generator::test1();
    cgt_dyadic_rational_move_generator::test2();
    cgt_dyadic_rational_move_generator::test3();
    cgt_dyadic_rational_move_generator::test4();
    cgt_dyadic_rational_move_generator::test5();
}
