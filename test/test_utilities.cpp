//---------------------------------------------------------------------------
// Utility functions for unit tests
//---------------------------------------------------------------------------
#include "test_utilities.h"
#include "file_parser.h"
#include <string>
#include <vector>
#include <sstream>
#include "game.h"
#include "sumgame.h"
#include "alternating_move_game.h"
#include <cassert>
#include <cstddef>
#include <memory>

using std::vector, std::string, std::stringstream;

void assert_solve(game& pos, bw to_play, const bool expected_result)
{
    assert_restore_game r(pos);
    assert_black_white(to_play);
    alternating_move_game g(pos, to_play);
    const bool result = g.solve();
    assert(result == expected_result);
}

void assert_solve_sum(sumgame& g, bw to_play, const bool expected_result)
{
    assert_black_white(to_play);
    g.set_to_play(to_play);
    const bool result = g.solve();
    assert(result == expected_result);
}

void test_sum(sumgame& sum, bool res_b, bool res_w)
{
    assert_solve_sum(sum, BLACK, res_b);
    assert_solve_sum(sum, WHITE, res_w);
}

void test_one_game(game& g, bool res_b, bool res_w)
{
    //     std::cout << "test " << g << std::endl;
    sumgame sum(BLACK);
    sum.add(&g);
    test_sum(sum, res_b, res_w);
}

void test_two_games(game& g1, game& g2, bool res_b, bool res_w)
{
    //     std::cout << "test " << g1 << " + " << g2 << std::endl;
    sumgame sum(BLACK);
    sum.add(&g1);
    sum.add(&g2);
    test_sum(sum, res_b, res_w);
}

void test_three_games(game& g1, game& g2, game& g3, bool res_b, bool res_w)
{
    //     std::cout << "test " << g1 << " + " << g2 << " + " << g3 <<
    //     std::endl;
    sumgame sum(BLACK);
    sum.add(&g1);
    sum.add(&g2);
    sum.add(&g3);
    test_sum(sum, res_b, res_w);
}

void assert_player_sum_outcome(int player, bool expected_outcome,
                               vector<game*>&& games, bool delete_games)
{
    assert_player_sum_outcome(player, expected_outcome, games, delete_games);
}

void assert_player_sum_outcome(int player, bool expected_outcome,
                               std::vector<game*>& games, bool delete_games)
{
    assert_black_white(player);

    sumgame sum(player);

    for (game* g : games)
    {
        sum.add(g);
    }

    bool outcome = sum.solve();

    assert(outcome == expected_outcome);

    if (delete_games)
    {
        for (game* g : games)
        {
            delete g;
        }
        games.clear();
    }
}

void assert_sum_outcomes(bool black_outcome, bool white_outcome,
                         std::vector<game*>& games)
{

    assert_player_sum_outcome(BLACK, black_outcome, games, false);
    assert_player_sum_outcome(WHITE, white_outcome, games, true);
}

void assert_sum_outcomes(bool black_outcome, bool white_outcome,
                         vector<game*>&& games)
{
    assert_sum_outcomes(black_outcome, white_outcome, games);
}

void assert_inverse_sum_zero(game* g)
{
    for (int i = 0; i <= 1; i++)
    {
        int to_play = i == 0 ? BLACK : WHITE;

        game* g_negative = g->inverse();

        sumgame sum(to_play);
        sum.add(g);
        sum.add(g_negative);

        bool outcome = sum.solve();

        assert(outcome == false);

        delete g_negative;
    }

    delete g;
}

void assert_file_parser_output(file_parser* parser,
                               vector<game_case*>& expected_cases)
{
    game_case gc;
    size_t case_idx = 0;

    while (parser->parse_chunk(gc))
    {
        assert(case_idx < expected_cases.size());

        game_case& expected = *expected_cases[case_idx];
        case_idx++;

        assert(gc.to_play == expected.to_play);
        assert(gc.expected_outcome == expected.expected_outcome);
        assert(gc.games.size() == expected.games.size());

        for (size_t i = 0; i < gc.games.size(); i++)
        {
            string str_got;
            string str_expected;

            {
                stringstream stream;
                gc.games[i]->print(stream);
                str_got = stream.str();
            }

            {
                stringstream stream;
                expected.games[i]->print(stream);
                str_expected = stream.str();
            }

            assert(str_got == str_expected);
        }

        gc.cleanup_games();
    }

    assert(case_idx == expected_cases.size());
}

void assert_file_parser_output_file(const string& file_name,
                                    vector<game_case*>& expected_cases)
{
    file_parser* parser = file_parser::from_file(file_name);
    assert_file_parser_output(parser, expected_cases);
    delete parser;
}

void assert_solve_test_file(const std::string& file_name,
                            int expected_case_count)
{
    assert(expected_case_count >= 0);

    std::unique_ptr<file_parser> fp =
        std::unique_ptr<file_parser>(file_parser::from_file(file_name));

    int case_count = 0;
    game_case gc;

    while (fp->parse_chunk(gc))
    {
        case_count += 1;

        // Should probably define a meaningful expected result for unit tests...
        assert(gc.expected_outcome != TEST_RESULT_UNSPECIFIED);

        sumgame s(gc.to_play);

        for (game* g : gc.games)
        {
            s.add(g);
        }

        bool result = s.solve();
        assert(result == gc.expected_outcome);

        gc.cleanup_games();
    }

    assert(case_count == expected_case_count);
}
