//---------------------------------------------------------------------------
// Utility functions for unit tests
//---------------------------------------------------------------------------
#include "test_utilities.h"

void assert_solve(game& pos, bw to_play,
                  const bool expected_result)
{
    assert_black_white(to_play);
    alternating_move_game g(pos, to_play);
    const bool result = g.solve();
    assert(result == expected_result);
}

void assert_solve_sum(sumgame& g, bw to_play,
                      const bool expected_result)
{
    assert_black_white(to_play);
    g.set_to_play(to_play);
    const bool result = g.solve();
    assert(result == expected_result);
}

void test_sum(sumgame& sum, bool resB, bool resW)
{
    assert_solve_sum(sum, BLACK, resB);
    assert_solve_sum(sum, WHITE, resW);
}

void test_one_game(game& g, bool resB, bool resW)
{
//     std::cout << "test " << g << std::endl;
    sumgame sum(BLACK);
    sum.add(&g);
    test_sum(sum, resB, resW);
}

void test_two_games(game& g1, game& g2, bool resB, bool resW)
{
//     std::cout << "test " << g1 << " + " << g2 << std::endl;
    sumgame sum(BLACK);
    sum.add(&g1);
    sum.add(&g2);
    test_sum(sum, resB, resW);
}

void test_three_games(game& g1, game& g2, game& g3, bool resB, bool resW)
{
//     std::cout << "test " << g1 << " + " << g2 << " + " << g3 << std::endl;
    sumgame sum(BLACK);
    sum.add(&g1);
    sum.add(&g2);
    sum.add(&g3);
    test_sum(sum, resB, resW);
}
