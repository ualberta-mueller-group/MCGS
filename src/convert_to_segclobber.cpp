#include "convert_to_segclobber.h"

#include <cassert>
#include <filesystem>
#include <ostream>

#include "cgt_basics.h"
#include "clobber_1xn.h"
#include "file_parser_ast.h"
#include "game.h"
#include "test_case.h"
#include "test_case_enums.h"
#include "test_file_iterator.h"
#include "throw_assert.h"

using namespace std;

////////////////////////////////////////////////// Helpers
namespace {
inline char color_to_segclobber_char(ebw color)
{
    switch (color)
    {
        case EMPTY:
            return '.';
        case BLACK:
            return 'X';
        case WHITE:
            return 'O';
        default:
            THROW_ASSERT(false);
    }
}

inline ebw segclobber_char_to_color(char c)
{
    switch (c)
    {
        case '.':
            return EMPTY;
        case 'X':
            return BLACK;
        case 'O':
            return WHITE;
        default:
            THROW_ASSERT(false);
    }
}

inline char color_to_segclobber_player_char(ebw color)
{
    switch (color)
    {
        case EMPTY:
            return '?';
        case BLACK:
            return 'B';
        case WHITE:
            return 'W';
        default:
            THROW_ASSERT(false);
    }
}

#if false
inline ebw segclobber_player_char_to_color(char c)
{
    switch (c)
    {
        case '?':
            return EMPTY;
        case 'B':
            return BLACK;
        case 'W':
            return WHITE;
        default:
            THROW_ASSERT(false);
    }
}
#endif

/*
    Returns string of 'X', 'O', and '.', obtained by concatenating the given
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

        assert(last_is_empty());
        for (int tile : board)
        {
            const char c = color_to_segclobber_char(tile);
            game_string.push_back(c);
        }
    }

    if (game_string.empty())
        game_string.push_back(color_to_segclobber_char(EMPTY));

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

    ebw expected_winner;
    switch (expected_outcome)
    {
        case MINIMAX_OUTCOME_NONE:
        {
            expected_winner = EMPTY;
            break;
        }
        case MINIMAX_OUTCOME_WIN:
        {
            expected_winner = player;
            break;
        }
        case MINIMAX_OUTCOME_LOSS:
        {
            expected_winner = opponent(player);
            break;
        }
        default:
            assert(false);
    }

    player_and_winner_pair.first = color_to_segclobber_player_char(player);
    player_and_winner_pair.second =
        color_to_segclobber_player_char(expected_winner);

    assert(!player_and_winner_pair.first.empty() &&
           !player_and_winner_pair.second.empty());

    return player_and_winner_pair;
}

// Open a file for writing in the given directory
ofstream open_output_file(const string& output_dir_name,
                          const string& output_file_name)
{
    THROW_ASSERT(!output_dir_name.empty() && !output_file_name.empty());

    const std::filesystem::path output_dir_path(output_dir_name);
    const std::filesystem::path output_file_path(output_dir_path /
                                                 output_file_name);

    const std::filesystem::path output_file_path_parent =
        output_file_path.parent_path();

    THROW_ASSERT(!std::filesystem::exists(output_file_path),
                 "File \"" + output_file_path.string() + "\" already exists!");

    // Make directories if they don't exist
    if (!std::filesystem::exists(output_file_path_parent))
        std::filesystem::create_directories(output_file_path_parent);

    THROW_ASSERT(std::filesystem::exists(output_file_path_parent) &&
                     std::filesystem::is_directory(output_file_path_parent),
                 "Couldn't open directory \"" +
                     output_file_path_parent.string() + "\"!");

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
    assert(!game_string.empty());

    pair<string, string> player_and_winner =
        extract_player_and_expected_winner(test_case);

    assert(!player_and_winner.first.empty() &&
           !player_and_winner.second.empty());

    string test_line;

    test_line += game_string;
    test_line += ' ';

    test_line += player_and_winner.first;
    test_line += ' ';

    test_line += player_and_winner.second;
    test_line += '\n';

    return test_line;
}

// Either the target doesn't exist, or is an empty directory
bool output_dir_is_valid(const string& output_dir_name)
{
    if (!std::filesystem::exists(output_dir_name))
        return true;

    return std::filesystem::is_directory(output_dir_name) &&
           std::filesystem::is_empty(output_dir_name);
}

} // namespace

////////////////////////////////////////////////// Exported functions
void convert_tests_to_segclobber(shared_ptr<file_parser> fp,
                                 const string& output_dir_name)
{
    THROW_ASSERT(output_dir_is_valid(output_dir_name),
                 "Output path \"" + output_dir_name +
                     "\" exists and is not an empty directory!");

    ofstream output_file =
        open_output_file(output_dir_name, "segclobber-tests.txt");
    THROW_ASSERT(output_file.is_open());

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

            const optional<string> test_line =
                convert_test_single(test_case_casted);

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

    THROW_ASSERT(output_dir_is_valid(output_dir_name),
                 "Output path \"" + output_dir_name +
                     "\" exists and is not an empty directory!");

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
        optional<ofstream> output_file;

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

                const optional<string> test_line =
                    convert_test_single(test_case_casted);

                if (!test_line.has_value())
                {
                    n_skipped_tests++;
                    continue;
                }

                if (!output_file.has_value())
                    output_file =
                        open_output_file(output_dir_name, relative_file_path);

                assert(output_file->is_open());
                *output_file << *test_line << flush;
            }
        }
    }

    cout << "Done! ";
    cout << n_skipped_tests
         << " test case(s) skipped due to having non clobber_1xn games.";
    cout << endl;
}
