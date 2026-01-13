//---------------------------------------------------------------------------
// Utility functions for unit tests
//---------------------------------------------------------------------------
#include "test_utilities.h"
#include "cgt_basics.h"
#include "csv_row.h"
#include "file_parser.h"
#include <string>
#include <vector>
#include <sstream>
#include <optional>
#include "game.h"
#include "sumgame.h"
#include "alternating_move_game.h"
#include "test_case.h"
#include "test_case_enums.h"
#include <cassert>
#include <cstddef>
#include <unordered_set>
#include <memory>

using std::vector, std::string, std::stringstream;

std::unordered_set<move> get_generated_moves_for_player(const game* g, bw player)
{
    assert(g != nullptr &&        //
           is_black_white(player) //
    );

    std::unordered_set<move> moves;

    move_generator* gen = g->create_move_generator(player);

    while (*gen)
    {
        const ::move m = gen->gen_move();
        ++(*gen);
        auto inserted = moves.insert(m);
        assert(inserted.second);
    }

    delete gen;

    return moves;
}

void test_moves_as_strings_for_player(
    game* g, bw player, const std::vector<std::string>& exp_move_strings,
    const std::string& game_prefix)
{
    assert(g != nullptr &&        //
           is_black_white(player) //
    );

    // Get expected move strings
    std::unordered_set<std::string> exp_move_strings_prefixed;

    for (const std::string& game_string : exp_move_strings)
        exp_move_strings_prefixed.insert(game_prefix + game_string);


    std::unordered_set<std::string> gen_move_strings;

    move_generator* gen = g->create_move_generator(player);

    while (*gen)
    {
        assert_restore_game arg(*g);

        const ::move m = gen->gen_move();
        ++(*gen);

        const string before_string = g->to_string();
        const hash_t before_hash = g->get_local_hash();

        g->play(m, player);
        const string after_string = g->to_string();
        const hash_t after_hash = g->get_local_hash();

        g->undo_move();
        const string undo_string = g->to_string();
        const hash_t undo_hash = g->get_local_hash();

        assert(before_string == undo_string && //
               before_hash == undo_hash        //
        );

        assert(before_string != after_string && //
               before_hash != after_hash        //
        );

        gen_move_strings.insert(after_string);
    }

    delete gen;
    assert(gen_move_strings == exp_move_strings_prefixed);
}

std::unordered_set<std::string> get_generated_moves_as_strings_for_player(
    game* g, bw player)
{
    assert(g != nullptr &&        //
           is_black_white(player) //
    );

    std::unordered_set<std::string> move_strings;

    move_generator* gen = g->create_move_generator(player);

    while (*gen)
    {
        assert_restore_game arg(*g);

        const ::move m = gen->gen_move();
        ++(*gen);

        g->play(m, player);

        stringstream str;
        str << *g;

        auto inserted = move_strings.insert(str.str());
        assert(inserted.second);

        g->undo_move();
    }

    delete gen;

    return move_strings;
}

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

# warning DELETE COMMENT HERE
/*
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
        assert(gc.expected_value == expected.expected_value);
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
*/

namespace file_parser_test {

void assert_file_parser_output(file_parser* parser,
    const std::vector<csv_row*>& expected_output)
{
    const size_t n_expected_cases = expected_output.size();
    size_t n_cases = 0;

    while (parser->parse_chunk())
    {
        const int n_tests_in_chunk = parser->n_test_cases();
        for (int chunk_test_idx = 0; chunk_test_idx < n_tests_in_chunk; chunk_test_idx++)
        {
            assert(n_cases < n_expected_cases);

            std::shared_ptr<i_test_case> test_case =
                parser->get_test_case(chunk_test_idx);

            const csv_row& row_got = test_case->get_csv_row();
            const csv_row& row_expected = *expected_output[n_cases];
            n_cases++;

            assert(row_got.player == row_expected.player);
            assert(row_got.expected_result == row_expected.expected_result);
            assert(row_got.games == row_expected.games);
            assert(row_got.command_type == row_expected.command_type);
        }
    }

    assert(n_cases == n_expected_cases);
}


void assert_file_parser_output_file(const std::string& file_name,
                                    const std::vector<csv_row*>& expected_output)
{
    file_parser* parser = file_parser::from_file(file_name);
    assert_file_parser_output(parser, expected_output);
    delete parser;
}

void add_row(std::vector<csv_row*>& rows, ebw player,
             std::optional<std::string>&& exp_result,
             std::vector<game*>&& games, command_type_enum type)
{
    csv_row* row = new csv_row();

    row->fill_pre_test_fields(games, player, exp_result);
    row->fill_visitor_fields({}, type, "");

    for (game* g : games)
        delete g;
    games.clear();

    rows.push_back(row);
}


} // namespace file_parser_test 


/*
   TODO this function was weird before the January 2026 file_parser refactor,
   and now it's even weirder...

   Do something about it?
*/
void assert_solve_test_file(const std::string& file_name,
                            int expected_case_count)
{
    assert(expected_case_count >= 0);

    std::unique_ptr<file_parser> fp =
        std::unique_ptr<file_parser>(file_parser::from_file(file_name));

    int actual_case_count = 0;

    while (fp->parse_chunk())
    {
        const int n_test_cases = fp->n_test_cases();

        for (int test_idx = 0; test_idx < n_test_cases; test_idx++)
        {
            actual_case_count++;

            // Should probably define a meaningful expected result for unit tests...
            //assert(gc.expected_value.type() == SEARCH_VALUE_TYPE_WINLOSS);

            std::shared_ptr<i_test_case> test_case = fp->get_test_case(test_idx);
            assert(dynamic_cast<test_case_solve_bw*>(test_case.get()) != nullptr);

            const csv_row& row = test_case->get_csv_row();

            assert(!row.player.value().empty());
            const char player_char = row.player.value().at(0);

            const bw to_play = player_char_to_color(player_char);
            const std::vector<game*>& games = fp->get_games();

            sumgame s(to_play);

            for (game* g : games)
                s.add(g);
            bool result = s.solve();
            s.pop(games);

            const std::string result_text = result ? "Win" : "Loss";
            assert(result_text == row.expected_result.value());
        }
    }

    assert(actual_case_count == expected_case_count);
}
