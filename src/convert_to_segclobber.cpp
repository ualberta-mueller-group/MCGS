#include "convert_to_segclobber.h"

#include <cassert>
#include <filesystem>
#include <ostream>
#include <sstream>
#include <system_error>

#include "cgt_basics.h"
#include "clobber_1xn.h"
#include "file_iterator.h"
#include "file_parser_ast.h"
#include "game.h"
#include "test_case.h"
#include "test_case_enums.h"
#include "test_file_iterator.h"
#include "throw_assert.h"

using namespace std;

////////////////////////////////////////////////// Helpers
namespace {
inline char color_to_segclobber_char(int color)
{
    switch (color)
    {
        case EMPTY:
            return '.';
        case BLACK:
            return 'B';
        case WHITE:
            return 'W';
        default:
            THROW_ASSERT(false);
    }
}

inline char segclobber_char_to_color(char c)
{
    switch (c)
    {
        case '.':
            return EMPTY;
        case 'B':
            return BLACK;
        case 'W':
            return WHITE;
        default:
            THROW_ASSERT(false);
    }
}

/*
    Returns string of 'B', 'W', and '.', obtained by concatenating the given
    clobber_1xn boards. If a given game is not of type clobber_1xn, returns no
    value.
*/
optional<string> make_segclobber_game_string(const vector<game*>& games)
{
    string game_string;
    
    auto last_is_empty = [&]() -> bool
    {
        if (game_string.empty())
            return true;

        const char back = game_string.back();
        return segclobber_char_to_color(back) == EMPTY;
    };

    for (const game* g : games)
    {
        if (g->game_type() != game_type<clobber_1xn>())
            return {};

        const clobber_1xn* g_casted = dynamic_cast<const clobber_1xn*>(g);
        assert(g_casted != nullptr);

        const vector<int>& board = g_casted->board_const();

        if (!last_is_empty())
            game_string.push_back(color_to_segclobber_char(EMPTY));

        for (int tile : board)
        {
            const char c = color_to_segclobber_char(tile);
            game_string.push_back(c);
        }
    }

    return game_string;
}

/*
    First: player to play ('B' or 'W')
    Second: expected winner (one of 'B', 'W', or '?')
*/
pair<string, string> extract_player_and_expected_winner(
    const test_case_solve_bw* test_case)
{
    assert(test_case != nullptr);

    pair<string, string> player_and_winner_pair;

    const fp_expr_command_solve_bw& expr = test_case->get_fp_expr();

    const bw player = expr.get_player();
    assert(is_black_white(player));

    const minimax_outcome_enum expected_outcome = expr.get_expected_outcome();

    char expected_winner;
    switch (expected_outcome)
    {
        case MINIMAX_OUTCOME_NONE:
        {
            expected_winner = '?';
            break;
        }
        case MINIMAX_OUTCOME_WIN:
        {
            expected_winner = color_to_segclobber_char(player);
            break;
        }
        case MINIMAX_OUTCOME_LOSS:
        {
            expected_winner = color_to_segclobber_char(opponent(player));
            break;
        }
        default:
            assert(false);
    }

    player_and_winner_pair.first = color_to_segclobber_char(player);
    player_and_winner_pair.second = expected_winner;

    assert(!player_and_winner_pair.first.empty() &&
           !player_and_winner_pair.second.empty());

    return player_and_winner_pair;
}

/*
    Creates directory if it doesn't exist.

    Throws if the given path already exists and is not an empty directory.
*/
void make_output_dir(const string& output_dir_name)
{
    THROW_ASSERT(!output_dir_name.empty());

    const std::filesystem::path output_dir_path(output_dir_name);

    if (std::filesystem::exists(output_dir_path))
    {
        THROW_ASSERT(std::filesystem::is_directory(output_dir_path),
                     "Output path \"" + output_dir_path.string() +
                         "\" already exists and is not a directory!");

        THROW_ASSERT(std::filesystem::is_empty(output_dir_path),
                     "Output directory \"" + output_dir_path.string() +
                         "\" exists and is not empty!");

        return;
    }

    std::filesystem::create_directory(output_dir_path);

    THROW_ASSERT(std::filesystem::is_directory(output_dir_path) &&
                 std::filesystem::is_empty(output_dir_path));
}

// Open a file for writing in the given directory
ofstream open_output_file(const string& output_dir_name,
                          const string& output_file_name)
{
    THROW_ASSERT(!output_dir_name.empty() && !output_file_name.empty());

    const std::filesystem::path output_dir_path(output_dir_name);
    const std::filesystem::path output_file_path(output_dir_path /
                                                 output_file_name);

    THROW_ASSERT(std::filesystem::exists(output_dir_path) &&
                     std::filesystem::is_directory(output_dir_path),
                 "Output path is not a directory!");

    THROW_ASSERT(!std::filesystem::exists(output_file_path),
                 "Output file \"" + output_file_path.string() +
                     "\" already exists!");

    ofstream of(output_file_path);
    THROW_ASSERT(of.is_open());
    return of;
}

// Has value IFF the test case contains only clobber_1xn games
optional<string> convert_test_single(const test_case_solve_bw* test_case)
{
    assert(test_case != nullptr);

    const optional<string> game_string_opt =
        make_segclobber_game_string(test_case->get_games());

    if (!game_string_opt.has_value())
        return {};

    const string& game_string = game_string_opt.value();

    pair<string, string> player_and_winner =
        extract_player_and_expected_winner(test_case);

    assert(!player_and_winner.first.empty() &&
           !player_and_winner.second.empty());

    string test_line;

    test_line += game_string;
    test_line += ' ';

    test_line += player_and_winner.first;
    test_line += ' ';

    test_line += player_and_winner.second ;
    test_line += '\n';

    return test_line;
}

} // namespace

////////////////////////////////////////////////// Exported functions
void convert_tests_to_segclobber(shared_ptr<file_parser> fp,
                                 const string& output_dir_name)
{
    // Make output dir, open output file
    make_output_dir(output_dir_name);

    ofstream output_file =
        open_output_file(output_dir_name, "segclobber-tests.txt");
    assert(output_file.is_open());

    uint64_t n_skipped_tests = 0;

    // Iterate over tests
    while (fp->parse_chunk())
    {
        const int n_chunk_tests = fp->n_test_cases();

        for (int i = 0; i < n_chunk_tests; i++)
        {
            if (fp->get_test_case_type(i) != COMMAND_TYPE_SOLVE_BW)
                continue;

            shared_ptr<i_test_case> test_case = fp->get_test_case(i);

            const test_case_solve_bw* test_case_casted =
                dynamic_cast<const test_case_solve_bw*>(test_case.get());
            assert(test_case_casted != nullptr);

            const optional<string> test_line = convert_test_single(test_case_casted);

            if (!test_line.has_value())
            {
                n_skipped_tests++;
                continue;
            }

            output_file << test_line.value() << flush;
        }
    }

    cout << "Done! ";
    cout << n_skipped_tests
         << " test case(s) skipped due to having non clobber_1xn games.";
    cout << endl;

    output_file.close();
}

void convert_tests_to_segclobber(const string& test_directory,
                                 const string& output_dir_name)
{
    THROW_ASSERT(!test_directory.empty() && !output_dir_name.empty());
    THROW_ASSERT(std::filesystem::is_directory(test_directory));

    make_output_dir(output_dir_name);

    uint64_t n_skipped_tests = 0;

    for (test_file_iterator iter(test_directory); iter; ++iter)
    {
        const std::filesystem::directory_entry& entry = iter.gen_entry();
        const std::filesystem::path& file_path = entry.path();
        assert(entry.is_regular_file() && file_path.extension() == ".test");

        cout << "New file: " << file_path.string() << endl;

        std::filesystem::path relative_file_path =
            std::filesystem::relative(file_path, test_directory);
        relative_file_path.replace_extension(".txt");

        unique_ptr<file_parser> fp(file_parser::from_file(file_path));

        while (fp->parse_chunk())
        {
            const int n_chunk_tests = fp->n_test_cases();

            optional<ofstream> output_file;

            for (int i = 0; i < n_chunk_tests; i++)
            {
                if (fp->get_test_case_type(i) != COMMAND_TYPE_SOLVE_BW)
                    continue;

                shared_ptr<i_test_case> test_case = fp->get_test_case(i);

                cout << "Reported test case is " << fp->get_test_case_type(i) << endl;
                cout << "Type is " << test_case->get_command_type() << endl;

                const test_case_solve_bw* test_case_casted =
                    dynamic_cast<const test_case_solve_bw*>(test_case.get());
                assert(test_case_casted != nullptr);

                const optional<string> test_line =
                    convert_test_single(test_case_casted);

                if (!test_line.has_value())
                {
                    n_skipped_tests++;
                    continue;
                }

                if (!output_file.has_value())
                    output_file = open_output_file(test_directory, relative_file_path);

                assert(output_file->is_open());
                *output_file << *test_line << flush;
            }

            if (output_file.has_value())
                output_file->close();
        }
    }

    cout << "Done! ";
    cout << n_skipped_tests
         << " test case(s) skipped due to having non clobber_1xn games.";
    cout << endl;

}
