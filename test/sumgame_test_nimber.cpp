#include "sumgame.h"
#include "cgt_nimber.h"
#include <iostream>
#include <cassert>
#include "test_utilities.h"

namespace {

void test_one_nimber(int value, bool res)
{
//     std::cout << "test " << value << std::endl;
    nimber i1(value);
    sumgame g(BLACK);
    g.add(&i1);
    assert_solve_sum(g, BLACK, res);
    assert_solve_sum(g, WHITE, res);
}

void test_two_nimbers(int value1, int value2, bool res)
{
//     std::cout << "test " << value1 << " + " << value2 << std::endl;
    nimber i1(value1);
    nimber i2(value2);
    sumgame g(BLACK);
    g.add(&i1);
    g.add(&i2);
    assert_solve_sum(g, BLACK, res);
    assert_solve_sum(g, WHITE, res);
}

void test_three_nimbers(int value1, int value2, int value3, 
                         bool res)
{
//     std::cout << "test " << value1 << " + " << value2
//               << " + " << value3 << std::endl;
    nimber i1(value1);
    nimber i2(value2);
    nimber i3(value3);
    sumgame g(BLACK);
    g.add(&i1);
    g.add(&i2);
    g.add(&i3);
    assert_solve_sum(g, BLACK, res);
    assert_solve_sum(g, WHITE, res);
}

} // namespace

void sumgame_test_nimber_all()
{
    test_one_nimber(0, false);
    test_one_nimber(1, true);
    test_one_nimber(2, true);
    test_one_nimber(7, true);
    test_two_nimbers(0, 0, false);
    test_two_nimbers(1, 1, false);
    test_two_nimbers(2, 2, false);
    test_two_nimbers(2, 1, true);
    test_two_nimbers(5, 5, false);
    test_two_nimbers(5, 4, true);
    test_three_nimbers(0, 0, 0, false);
    test_three_nimbers(1, 0, 1, false);
    test_three_nimbers(1, 2, 3, false);
    test_three_nimbers(0, 0, 1, true);
    test_three_nimbers(1, 2, 0, true);
    test_three_nimbers(2, 3, 4, true);
    test_three_nimbers(1, 1, 1, true);
}
