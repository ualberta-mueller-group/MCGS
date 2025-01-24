#include "cgt_basics.h"
#include "game_factory.h"
#include "sumgame.h"
#include "sum_of_nimbers.h"
#include "cgt_nimber.h"
#include <iostream>
#include <cassert>
#include <array>
#include "test_utilities.h"

namespace {

void assert_nim_value(const sumgame& g, int value)
{
    auto heaps = get_nim_heaps(g);
    assert(value == nimber::nim_sum(heaps));
}

/*
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
*/

void test_nimbers(bool expected_result, int nim_sum, const vector<game_factory_ptr>& nimber_factories)
{
    std::array<bw, 2> players({BLACK, WHITE});

    for (const bw& player : players)
    {
        assert_black_white(player);

        sumgame sum(player);
        vector<game*> games;

        for (const game_factory_ptr& factory : nimber_factories)
        {
            game* g = factory->new_game();

            games.push_back(g);
            sum.add(g);
        }

        assert_solve_sum(sum, player, expected_result);
        assert_nim_value(sum, nim_sum);

        for (game* g : games)
        {
            delete g;
        }
    }
}


} // namespace

void sumgame_test_nimber_all()
{
    /*
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
    */

    test_nimbers(false, 0, {
        make_factory<nimber>(0),
    });

    test_nimbers(true, 1, {
        make_factory<nimber>(1),
    });

    test_nimbers(true, 2, {
        make_factory<nimber>(2),
    });

    test_nimbers(true, 7, {
        make_factory<nimber>(7),
    });

    test_nimbers(true, 10, {
        make_factory<nimber>(10),
    });

    test_nimbers(false, 0, {
        make_factory<nimber>(0),
        make_factory<nimber>(0),
    });

    test_nimbers(true, 1, {
        make_factory<nimber>(1),
        make_factory<nimber>(0),
    });

    test_nimbers(false, 0, {
        make_factory<nimber>(1),
        make_factory<nimber>(1),
    });

    test_nimbers(false, 0, {
        make_factory<nimber>(2),
        make_factory<nimber>(2),
    });

    test_nimbers(true, 3, {
        make_factory<nimber>(2),
        make_factory<nimber>(1),
    });

    test_nimbers(false, 0, {
        make_factory<nimber>(5),
        make_factory<nimber>(5),
    });

    test_nimbers(true, 1, {
        make_factory<nimber>(5),
        make_factory<nimber>(4),
    });

    test_nimbers(false, 0, {
        make_factory<nimber>(0),
        make_factory<nimber>(0),
        make_factory<nimber>(0),
    });

    test_nimbers(false, 0, {
        make_factory<nimber>(1),
        make_factory<nimber>(0),
        make_factory<nimber>(1),
    });

    test_nimbers(false, 0, {
        make_factory<nimber>(1),
        make_factory<nimber>(2),
        make_factory<nimber>(3),
    });

    test_nimbers(true, 1, {
        make_factory<nimber>(0),
        make_factory<nimber>(0),
        make_factory<nimber>(1),
    });

    test_nimbers(true, 3, {
        make_factory<nimber>(1),
        make_factory<nimber>(2),
        make_factory<nimber>(0),
    });

    test_nimbers(true, 5, {
        make_factory<nimber>(2),
        make_factory<nimber>(3),
        make_factory<nimber>(4),
    });

    test_nimbers(true, 1, {
        make_factory<nimber>(1),
        make_factory<nimber>(1),
        make_factory<nimber>(1),
    });

    // test("3 4 5 6", true, 4);
    test_nimbers(true, 4, {
        make_factory<nimber>(3),
        make_factory<nimber>(4),
        make_factory<nimber>(5),
        make_factory<nimber>(6),
    });

    // test("3 4 5 2, false, 0);
    test_nimbers(false, 0, {
        make_factory<nimber>(3),
        make_factory<nimber>(4),
        make_factory<nimber>(5),
        make_factory<nimber>(2),
    });

    // test("12 11 3 11 12 1 2", false, 0);
    test_nimbers(false, 0, {
        make_factory<nimber>(12),
        make_factory<nimber>(11),
        make_factory<nimber>(3),
        make_factory<nimber>(11),
        make_factory<nimber>(12),
        make_factory<nimber>(1),
        make_factory<nimber>(2),
    });
}

