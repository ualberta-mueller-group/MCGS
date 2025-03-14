#include "sumgame.h"
#include "cgt_integer_game.h"
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

void test_one_integer(int value, bool res_b, bool res_w)
{
    //std::cout << "test " << value << std::endl;
    integer_game i1(value);
    sumgame g(BLACK);
    g.add(&i1);
    assert_solve_sum(g, BLACK, res_b);
    assert_solve_sum(g, WHITE, res_w);
}

void test_two_integers(int value1, int value2, bool res_b, bool res_w)
{
    //std::cout << "test " << value1 << " + " << value2 << std::endl;
    integer_game i1(value1);
    integer_game i2(value2);
    sumgame g(BLACK);
    g.add(&i1);
    g.add(&i2);
    assert_solve_sum(g, BLACK, res_b);
    assert_solve_sum(g, WHITE, res_w);
}

void test_three_integers(int value1, int value2, int value3, 
                         bool res_b, bool res_w)
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
    assert_solve_sum(g, BLACK, res_b);
    assert_solve_sum(g, WHITE, res_w);
}

void test_inverses()
{
    auto ints = {0, 1, -1, 2, 16, -3, 57, 8};
    for (auto i: ints)
    {
        integer_game ig(i);
        std::unique_ptr<game> inv(ig.inverse());
        test_inverse(ig, *inv);
    }
}

} // namespace

void sumgame_test_integer_all()
{
    test_empty();
    test_inverses();
    // can't add same game object to sumgame twice
    integer_game zero1(0);
    integer_game zero2(0);
    integer_game zero3(0);
    integer_game one(1);
    integer_game minus_one(-1);
    test_zero_1(zero1);
    test_zero_2(zero1, zero2);
    test_zero_3(zero1, zero2, zero3);
    test_inverse(zero1, zero2);
    test_inverse(minus_one, one);
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
