#include "sumgame.h"
#include "cgt_integer_game.h"
#include <iostream>
#include <cassert>
#include "test_utilities.h"

namespace {
void test_empty()
{
    //std::cout << "test empty sum" << std::endl;
    sumgame g(BLACK);
    assert_solve_sum(g, BLACK, false);
    assert_solve_sum(g, WHITE, false);
}

void test_one_integer(int value, bool resB, bool resW)
{
    //std::cout << "test " << value << std::endl;
    integer_game i1(value);
    sumgame g(BLACK);
    g.add(&i1);
    assert_solve_sum(g, BLACK, resB);
    assert_solve_sum(g, WHITE, resW);
}

void test_two_integers(int value1, int value2, bool resB, bool resW)
{
    //std::cout << "test " << value1 << " + " << value2 << std::endl;
    integer_game i1(value1);
    integer_game i2(value2);
    sumgame g(BLACK);
    g.add(&i1);
    g.add(&i2);
    assert_solve_sum(g, BLACK, resB);
    assert_solve_sum(g, WHITE, resW);
}

void test_three_integers(int value1, int value2, int value3, 
                         bool resB, bool resW)
{
//     std::cout << "test " << value1 << " + " << value2
//               << " + " << value3 << std::endl;
    integer_game i1(value1);
    integer_game i2(value2);
    integer_game i3(value3);
    sumgame g(BLACK);
    g.add(&i1);
    g.add(&i2);
    g.add(&i3);
    assert_solve_sum(g, BLACK, resB);
    assert_solve_sum(g, WHITE, resW);
}

} // namespace

void sumgame_test_integer_all()
{
    test_empty();
    test_one_integer(0, false, false);
    test_two_integers(0, 0, false, false);
    test_one_integer(1, true, false);
    test_one_integer(-1, false, true);
    test_one_integer(2, true, false);
    test_two_integers(1, -1, false, false);
    test_two_integers(-1, 1, false, false);
    test_two_integers(2, -2, false, false);
    test_two_integers(-2, 1, false, true);
    test_two_integers(-10, 10, false, false);
    test_two_integers(-1, -1, false, true);
    test_three_integers(0, 0, 0, false, false);
    test_three_integers(-1, 0, 1, false, false);
    test_three_integers(1, 2, -3, false, false);
    test_three_integers(0, 0, 1, true, false);
    test_three_integers(-1, -1, 0, false, true);
    test_three_integers(-10, -3, 13, false, false);
    test_three_integers(1, -1, -1, false, true);
}
