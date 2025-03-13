#include "cgt_basics.h"
#include "sumgame.h"
#include "sum_of_nimbers.h"
#include "cgt_nimber.h"
#include <iostream>
#include <cassert>
#include <array>
#include "test_utilities.h"

using std::vector;

namespace {

void assert_nim_value(const sumgame& g, int value)
{
    auto heaps = get_nim_heaps(g);
    assert(value == nimber::nim_sum(heaps));
}

// Takes ownership of its games and deletes them
void test_nimbers(bool expected_result, int nim_sum, vector<nimber*> nimbers)
{
    std::array<bw, 2> players({BLACK, WHITE});
    sumgame sum(BLACK);

    for (nimber* n : nimbers)
    {
        sum.add(n);
    }

    for (const bw& player : players)
    {
        assert_black_white(player);
        sum.set_to_play(player);

        assert_solve_sum(sum, player, expected_result);
        assert_nim_value(sum, nim_sum);
    }

    for (nimber* n : nimbers)
    {
        delete n;
    }
}
} // namespace

void sumgame_test_nimber_all()
{
    test_nimbers(false, 0, {
        new nimber(0),
    });

    test_nimbers(true, 1, {
        new nimber(1),
    });

    test_nimbers(true, 2, {
        new nimber(2),
    });

    test_nimbers(true, 7, {
        new nimber(7),
    });

    test_nimbers(true, 10, {
        new nimber(10),
    });

    test_nimbers(false, 0, {
        new nimber(0),
        new nimber(0),
    });

    test_nimbers(true, 1, {
        new nimber(1),
        new nimber(0),
    });

    test_nimbers(false, 0, {
        new nimber(1),
        new nimber(1),
    });

    test_nimbers(false, 0, {
        new nimber(2),
        new nimber(2),
    });

    test_nimbers(true, 3, {
        new nimber(2),
        new nimber(1),
    });

    test_nimbers(false, 0, {
        new nimber(5),
        new nimber(5),
    });

    test_nimbers(true, 1, {
        new nimber(5),
        new nimber(4),
    });

    test_nimbers(false, 0, {
        new nimber(0),
        new nimber(0),
        new nimber(0),
    });

    test_nimbers(false, 0, {
        new nimber(1),
        new nimber(0),
        new nimber(1),
    });

    test_nimbers(false, 0, {
        new nimber(1),
        new nimber(2),
        new nimber(3),
    });

    test_nimbers(true, 1, {
        new nimber(0),
        new nimber(0),
        new nimber(1),
    });

    test_nimbers(true, 3, {
        new nimber(1),
        new nimber(2),
        new nimber(0),
    });

    test_nimbers(true, 5, {
        new nimber(2),
        new nimber(3),
        new nimber(4),
    });

    test_nimbers(true, 1, {
        new nimber(1),
        new nimber(1),
        new nimber(1),
    });

    // test("3 4 5 6", true, 4);
    test_nimbers(true, 4, {
        new nimber(3),
        new nimber(4),
        new nimber(5),
        new nimber(6),
    });

    // test("3 4 5 2, false, 0);
    test_nimbers(false, 0, {
        new nimber(3),
        new nimber(4),
        new nimber(5),
        new nimber(2),
    });

    // test("12 11 3 11 12 1 2", false, 0);
    test_nimbers(false, 0, {
        new nimber(12),
        new nimber(11),
        new nimber(3),
        new nimber(11),
        new nimber(12),
        new nimber(1),
        new nimber(2),
    });

    assert_solve_test_file(UNIT_TEST_INPUT_DIR + "nimber.test", 22);
}

