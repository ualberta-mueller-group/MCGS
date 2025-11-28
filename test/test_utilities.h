//---------------------------------------------------------------------------
// Utility functions for unit tests
//---------------------------------------------------------------------------

#pragma once
#include <exception> // IWYU pragma: keep

#include <memory>
#include "cgt_move_new.h"
#include "game.h"
#include "sumgame.h"
#include "file_parser.h"
#include <string>
#include <vector>
#include <cassert>
#include <unordered_set>

std::unordered_set<move> get_generated_moves_for_player(const game* g,
                                                        bw player);

// TODO unit test and document this (and other functions in this file)
void test_moves_as_strings_for_player(
    game* g, bw player, const std::vector<std::string>& exp_move_strings,
    const std::string& game_prefix);

std::unordered_set<std::string> get_generated_moves_as_strings_for_player(
    game* g, bw player);

inline void assert_move(move_generator& mg, int mv)
{
    const move m = mg.gen_move();
    assert(m == mv);
}

inline void assert_num_moves(const game& g, bw to_play, int num_moves)
{
    std::unique_ptr<move_generator> mgp(g.create_move_generator(to_play));
    move_generator& mg(*mgp);
    for (int i = 0; i < num_moves; ++i)
    {
        assert(mg);
        ++mg;
    }
    assert(!mg);
}

inline void assert_two_part_move(move_generator& mg, int from, int to)
{
    move m = mg.gen_move();
    assert(from == cgt_move_new::move2_get_from(m));
    assert(to == cgt_move_new::move2_get_to(m));
}

void assert_solve(game& pos, bw to_play, const bool expected_result);
void assert_solve_sum(sumgame& g, bw to_play, const bool expected_result);

void test_sum(sumgame& sum, bool res_b, bool res_w);
void test_one_game(game& g, bool res_b, bool res_w);
void test_two_games(game& g1, game& g2, bool res_b, bool res_w);
void test_three_games(game& g1, game& g2, game& g3, bool res_b, bool res_w);

inline void test_zero_1(game& g)
{
    test_one_game(g, false, false);
}

inline void test_zero_2(game& g1, game& g2)
{
    test_two_games(g1, g2, false, false);
}

inline void test_zero_3(game& g1, game& g2, game& g3)
{
    test_three_games(g1, g2, g3, false, false);
}

inline void test_inverse(game& g1, game& g2) // g1+g2 == 0
{
    test_zero_2(g1, g2);
}

// void test_equal(game& g1, game& g2); // needs game::inverse()

// These functions take ownership of the games passed to them
void assert_player_sum_outcome(int player, bool expected_outcome,
                               std::vector<game*>&& games,
                               bool delete_games = true);

void assert_player_sum_outcome(int player, bool expected_outcome,
                               std::vector<game*>& games,
                               bool delete_games = true);

void assert_sum_outcomes(bool black_outcome, bool white_outcome,
                         std::vector<game*>&& games);

void assert_sum_outcomes(bool black_outcome, bool white_outcome,
                         std::vector<game*>& games);

void assert_inverse_sum_zero(game* g);

void assert_file_parser_output(file_parser* parser,
                               std::vector<game_case*>& expected_cases);

void assert_file_parser_output_file(const std::string& file_name,
                                    std::vector<game_case*>& expected_cases);

const std::string UNIT_TEST_INPUT_DIR = "test/input/unit_tests/";

void assert_solve_test_file(const std::string& file_name,
                            int expected_case_count);

// static_assert forces a semicolon after usage of this macro
#define ASSERT_DID_THROW(statements)                                           \
    {                                                                          \
        bool threw = false;                                                    \
        try                                                                    \
        {                                                                      \
            statements;                                                        \
        }                                                                      \
        catch (std::exception & exc)                                           \
        {                                                                      \
            threw = true;                                                      \
        }                                                                      \
        assert(threw);                                                         \
    }                                                                          \
    static_assert(true)
