//---------------------------------------------------------------------------
// Utility functions for unit tests
//---------------------------------------------------------------------------
#include "test_utilities.h"

using std::vector;

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

//////////////////////////////////////////////////////////// game_factory tests

void assert_player_sum_outcome(int player, bool expected_outcome, const vector<game_factory_ptr>& factories)
{
    assert_black_white(player);

    sumgame sum(player);
    vector<game*> games; // clean up later...

    for (const game_factory_ptr& factory: factories)
    {
        game* g = factory->new_game();

        sum.add(g);
        games.push_back(g);
    }

    bool outcome = sum.solve();

    assert(outcome == expected_outcome);

    for (game* g : games)
    {
        delete g;
    }
}

void assert_sum_outcomes(bool black_outcome, bool white_outcome, const vector<game_factory_ptr>& factories)
{
    assert_player_sum_outcome(BLACK, black_outcome, factories);
    assert_player_sum_outcome(WHITE, white_outcome, factories);
}

void assert_inverse_sum_zero(const game_factory_ptr& factory)
{

    for (int i = 0; i <= 1; i++)
    {
        int to_play = i == 0 ? BLACK : WHITE;

        game* pos = factory->new_game();
        game* pos_negative = pos->inverse();

        sumgame sum(to_play);
        sum.add(pos);
        sum.add(pos_negative);

        bool outcome = sum.solve();

        assert(outcome == false);

        delete pos;
        delete pos_negative;
    }

}
