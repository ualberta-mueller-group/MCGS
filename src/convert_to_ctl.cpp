#include "convert_to_ctl.h"

#include <cassert>
#include <optional>
#include <vector>
#include <cctype>
#include <filesystem>
#include <ostream>
#include <utility>
#include <fstream>
#include <memory>
#include <cstddef>
#include <cstdint>
#include <string>
#include <iostream>

#include "cgt_basics.h"
#include "clobber_1xn.h"
#include "file_parser_ast.h"
#include "test_filter.h"
#include "game.h"
#include "grid.h"
#include "test_case.h"
#include "test_case_enums.h"
#include "test_file_iterator.h"
#include "throw_assert.h"
#include "file_parser.h"

using namespace std;

////////////////////////////////////////////////// Helpers
namespace {

inline void append_string_with_space(string& dst_string, const string& src_string)
{
    if (!dst_string.empty() && !std::isspace(dst_string.back()))
        dst_string.push_back(' ');

    dst_string += src_string;
}

inline void append_char_with_space(string& dst_string, char c)
{ 
    if (!dst_string.empty() && !std::isspace(dst_string.back()))
        dst_string.push_back(' ');

    dst_string.push_back(c);
}

optional<string> get_strip_game_string(const game* g_maybe_strip)
{
    const strip* g_strip = dynamic_cast<const strip*>(g_maybe_strip);
    if (g_strip == nullptr)
        return {};

    return g_strip->board_as_string();
}

optional<string> get_grid_game_string(const game* g_maybe_grid)
{
    const grid* g_grid = dynamic_cast<const grid*>(g_maybe_grid);

    if (g_grid == nullptr)
        return {};

    if (g_grid->get_grid_type() != GRID_TYPE_COLOR)
        return {};

    return g_grid->board_as_string();
}

#ifdef TRY_APPEND_GAME_STRING
#error Macro already defined
#endif

#define TRY_APPEND_GAME_STRING(string_optional)                                \
    {                                                                          \
        if (string_optional.has_value())                                       \
        {                                                                      \
            append_char_with_space(result, '(');                               \
            result += *string_optional;                                        \
            result.push_back(')');                                             \
                                                                               \
            continue;                                                          \
        }                                                                      \
    }                                                                          \
    static_assert(true)

optional<string> extract_game_string(const i_test_case* test_case)
{
    const std::vector<game*>& games = test_case->get_games();
    const std::vector<string>& game_types = test_case->get_game_types();
    assert(games.size() == game_types.size());

    const string* current_game_title = nullptr;
    string result;

    const size_t n_games = games.size();
    for (size_t i = 0; i < n_games; i++)
    {
        const game* g = games[i];
        const string& g_type = game_types[i];
        assert(!g_type.empty());

        if (current_game_title == nullptr || (g_type != *current_game_title))
        {
            current_game_title = &g_type;

            append_char_with_space(result, '[');
            result += g_type;
            result.push_back(']');
        }

        const optional<string> strip_string = get_strip_game_string(g);
        TRY_APPEND_GAME_STRING(strip_string);

        const optional<string> grid_string = get_grid_game_string(g);
        TRY_APPEND_GAME_STRING(grid_string);

        // Game is not a strip or color grid
        return {};
    }

    return std::move(result);
}

#undef TRY_APPEND_GAME_STRING

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

inline string minimax_outcome_to_ctl_string(minimax_outcome_enum oc)
{
    switch (oc)
    {
        case MINIMAX_OUTCOME_NONE:
            return "?";
        case MINIMAX_OUTCOME_WIN:
            return "win";
        case MINIMAX_OUTCOME_LOSS:
            return "loss";
        default:
            assert(false);
    }
}

optional<string> convert_test_solve_bw(const i_test_case* test_case)
{
    const test_case_solve_bw* test_case_casted =
        dynamic_cast<const test_case_solve_bw*>(test_case);
    assert(test_case_casted != nullptr);

    optional<string> result_string_opt = extract_game_string(test_case);
    if (!result_string_opt.has_value())
        return {}; // Some game can't be printed: implement it later

    const fp_expr_command_solve_bw& expr = test_case_casted->get_fp_expr();
    const bw player = expr.get_player();
    assert(is_black_white(player));
    const char player_char = color_to_player_char(player);

    const minimax_outcome_enum exp_outcome = expr.get_expected_outcome();
    const string outcome_string = minimax_outcome_to_ctl_string(exp_outcome);

    string& result_string = *result_string_opt;
    append_string_with_space(result_string, "solve");
    append_char_with_space(result_string, player_char);
    append_string_with_space(result_string, outcome_string);

    assert(result_string_opt.has_value());
    return result_string_opt;
}

optional<string> convert_test_single(const i_test_case* test_case)
{
    assert(test_case != nullptr);

    const command_type_enum command_type = test_case->get_command_type();

    switch (command_type)
    {
        case COMMAND_TYPE_SOLVE_BW:
            return convert_test_solve_bw(test_case);
        default:
            return {}; // Unimplemented
    }
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
void convert_tests_to_ctl_format(shared_ptr<file_parser> fp,
                                 const string& output_dir_name,
                                 test_filter_enum filter_type)
{
    THROW_ASSERT(output_dir_is_valid(output_dir_name),
                 "Output path \"" + output_dir_name +
                     "\" exists and is not an empty directory!");

    ofstream output_file =
        open_output_file(output_dir_name, "ctl-tests.txt");
    THROW_ASSERT(output_file.is_open());

    uint64_t n_filtered_tests = 0;
    uint64_t n_unimplemented_tests = 0;

    // Iterate over tests
    while (fp->parse_chunk())
    {
        const int n_chunk_tests = fp->n_test_cases();

        for (int i = 0; i < n_chunk_tests; i++)
        {
            std::shared_ptr<i_test_case> test_case = fp->get_test_case(i);

            if (!test_filter_permits_test_case(filter_type, *test_case))
            {
                n_filtered_tests++;
                continue;
            }

            const optional<string> test_line_opt = convert_test_single(test_case.get());

            if (!test_line_opt.has_value())
            {
                n_unimplemented_tests++;
                continue;
            }

            const string& test_line = *test_line_opt;
            assert(!test_line.empty());

            output_file << test_line << endl;
        }
    }

    cout << "Done!" << endl;

    cout << n_filtered_tests << " test case(s) skipped by the test filter.";
    cout << endl;

    if (n_unimplemented_tests > 0)
    {
        cout << "WARNING: " << n_unimplemented_tests
             << " test case(s) skipped due to being unimplemented." << endl;
    }
    cout << endl;

    output_file.close();
}

void convert_tests_to_ctl_format(const string& test_directory,
                                 const string& output_dir_name,
                                 test_filter_enum filter_type)
{
    THROW_ASSERT(!test_directory.empty() && !output_dir_name.empty());
    THROW_ASSERT(std::filesystem::is_directory(test_directory));

    THROW_ASSERT(output_dir_is_valid(output_dir_name),
                 "Output path \"" + output_dir_name +
                     "\" exists and is not an empty directory!");

    uint64_t n_filtered_tests = 0;
    uint64_t n_unimplemented_tests = 0;

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
                shared_ptr<i_test_case> test_case = fp->get_test_case(i);

                if (!test_filter_permits_test_case(filter_type, *test_case))
                {
                    n_filtered_tests++;
                    continue;
                }

                const optional<string> test_line_opt =
                    convert_test_single(test_case.get());

                if (!test_line_opt.has_value())
                {
                    n_unimplemented_tests++;
                    continue;
                }

                const string& test_line = *test_line_opt;
                assert(!test_line.empty());

                if (!output_file.has_value())
                    output_file =
                        open_output_file(output_dir_name, relative_file_path);

                assert(output_file->is_open());
                *output_file << test_line << endl;
            }
        }
    }

    cout << "Done!" << endl;

    cout << n_filtered_tests << " test case(s) skipped by the test filter.";
    cout << endl;

    if (n_unimplemented_tests > 0)
    {
        cout << "WARNING: " << n_unimplemented_tests
             << " test case(s) skipped due to being unimplemented." << endl;
    }
    cout << endl;
}
