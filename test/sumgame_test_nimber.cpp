#include "sumgame.h"
#include "sum_of_nimbers.h"
#include "cgt_nimber.h"
#include <iostream>
#include <cassert>
#include "test_utilities.h"

namespace {

void assert_nim_value(const sumgame& g, int value)
{
    auto heaps = get_nim_heaps(g);
    assert(value == nimber::nim_sum(heaps));
}

void test_one_nimber(int value, bool res, int sum)
{
//     std::cout << "test " << value << std::endl;
    nimber i1(value);
    sumgame g(BLACK);
    g.add(&i1);
    assert_solve_sum(g, BLACK, res);
    assert_solve_sum(g, WHITE, res);
    assert_nim_value(g, sum);
}

void test_two_nimbers(int value1, int value2,
                      bool res, int sum)
{
//     std::cout << "test " << value1 << " + " << value2 << std::endl;
    nimber i1(value1);
    nimber i2(value2);
    sumgame g(BLACK);
    g.add(&i1);
    g.add(&i2);
    assert_solve_sum(g, BLACK, res);
    assert_solve_sum(g, WHITE, res);
    assert_nim_value(g, sum);
}

void test_three_nimbers(int value1, int value2, int value3, 
                        bool res, int sum)
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
    assert_nim_value(g, sum);
}


} // namespace

void sumgame_test_nimber_all()
{
    test_one_nimber(0, false, 0);
    test_one_nimber(1, true, 1);
    test_one_nimber(2, true, 2);
    test_one_nimber(7, true, 7);
    test_one_nimber(10, true, 10);
    test_two_nimbers(0, 0, false, 0);
    test_two_nimbers(1, 0, true, 1);
    test_two_nimbers(1, 1, false, 0);
    test_two_nimbers(2, 2, false, 0);
    test_two_nimbers(2, 1, true, 3);
    test_two_nimbers(5, 5, false, 0);
    test_two_nimbers(5, 4, true, 1);
    test_three_nimbers(0, 0, 0, false, 0);
    test_three_nimbers(1, 0, 1, false, 0);
    test_three_nimbers(1, 2, 3, false, 0);
    test_three_nimbers(0, 0, 1, true, 1);
    test_three_nimbers(1, 2, 0, true, 3);
    test_three_nimbers(2, 3, 4, true, 5);
    test_three_nimbers(1, 1, 1, true, 1);
// TO DO
// test("3 4 5 6", true, 4);
// test("3 4 5 2, false, 0);
// test("12 11 3 11 12 1 2", false, 0);
}

