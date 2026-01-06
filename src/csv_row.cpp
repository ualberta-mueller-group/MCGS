#include "csv_row.h"
#include "cgt_basics.h"
#include "global_options.h"
#include "parsing_utilities.h"
#include "search_utils.h"
#include "solver_stats.h"
#include "test_case_enums.h"
#include "utilities.h"
#include <cctype>
#include <string>

using namespace std;

namespace {
string csv_game_string(const vector<game*>& games)
{
    stringstream stream;

    const size_t N = games.size();
    for (size_t i = 0; i < N; i++)
    {
        const game* g = games[i];
        g->print(stream);

        if (i + 1 < N)
        {
            stream << " ";
        }
    }

    string game_string = stream.str();

    return game_string;
}

string csv_comment_string(const vector<string>& comments)
{
    string comment_val;

    bool prev_is_space = true;
    for (const string& comment : comments)
    {
        for (char c : comment)
        {
            bool is_space = std::isspace(c);

            if (prev_is_space && is_space)
                continue;

            if (is_space)
                comment_val.push_back(' ');
            else
                comment_val.push_back(c);

            prev_is_space = is_space;
        }
    }

    while (!comment_val.empty() && comment_val.back() == ' ')
        comment_val.pop_back();

    return comment_val;
}

string csv_player_string(const optional<ebw>& player)
{
    if (!player.has_value())
        return "?";

    if (is_black_white(player.value()))
        return string(1, color_to_player_char(player.value()));

    return "IMP";
}

test_case_status_enum csv_test_case_status(
    const optional<string>& result, const optional<string>& expected_result)
{
    if (!result.has_value())
        return TEST_CASE_STATUS_TIMEOUT;

    if (!expected_result.has_value())
        return TEST_CASE_STATUS_COMPLETED;

    if (result.value() == expected_result.value())
        return TEST_CASE_STATUS_PASS;

    return TEST_CASE_STATUS_FAIL;
}

/*
   Sanitize field contents of a CSV row (for printing to file)
       1. Strip left and right whitespace
       2. Replace double quotes with 2 single quotes
       3. Wrap the result in double quotes
*/
string csv_format_field(const string& field)
{
    // Remove leading/trailing whitespace, replace double quotes with 2 single
    // quotes
    string sanitized_field;

    sanitized_field.push_back('\"');

    bool past_left_whitespace = false;

    const size_t N = field.size();
    for (size_t i = 0; i < N; i++)
    {
        const char& c = field[i];

        if (!past_left_whitespace && std::isspace(c))
            continue;
        else
            past_left_whitespace = true;

        if (c == '\"')
            sanitized_field += "''";
        else
            sanitized_field.push_back(c);
    }

    while (sanitized_field.size() > 0 && std::isspace(sanitized_field.back()))
        sanitized_field.pop_back();

    sanitized_field.push_back('\"');
    return sanitized_field;
}


} // namespace

bool csv_row::has_visitor_fields() const
{
    return                          //
        comments.has_value() &&     //
        command_type.has_value() && //
        input_hash.has_value();     //
}

bool csv_row::has_pre_test_fields() const
{
    return                   //
        games.has_value() && //
        player.has_value();  //

    // expected_result.has_value();
}

bool csv_row::has_post_test_fields() const
{
    return                      //
        status.has_value() &&   //
        time_ms.has_value() &&  //
        node_count.has_value(); //

    // result.has_value() &&   //
}

bool csv_row::has_autotest_fields() const
{
    return file.has_value() && case_number.has_value();
}

void csv_row::fill_visitor_fields(const vector<string>& comments,
                                  command_type_enum command_type,
                                  const string& input_hash)
{
    assert(!has_visitor_fields());

    this->comments = csv_comment_string(comments);
    this->command_type = command_type;
    this->input_hash = input_hash;

    assert(has_visitor_fields());
}

void csv_row::fill_pre_test_fields(
    const std::vector<game*>& games, std::optional<ebw> player,
    const std::optional<std::string>& expected_result)
{
    assert(!has_pre_test_fields());

    this->games = csv_game_string(games);
    this->player = csv_player_string(player);

    this->expected_result = expected_result;

    assert(has_pre_test_fields());
}

void csv_row::fill_post_test_fields(const optional<string>& result,
                                    double time_ms)
{
    assert(has_pre_test_fields());
    assert(!has_post_test_fields());

    this->result = result;
    this->time_ms = time_ms;

    this->status = csv_test_case_status(result, this->expected_result);

    const solver_stats& stats = stats::get_global_stats();
    this->node_count = stats.node_count;

    if (global::count_sums())
    {
        assert(stats.sum_hashes.has_value());
        this->unique_node_count = stats.sum_hashes->size();
    }

    assert(has_post_test_fields());
}

void csv_row::fill_autotest_fields(const std::string& file, int case_number)
{
    assert(has_pre_test_fields() && !has_autotest_fields());
    this->file = file;
    this->case_number = case_number;
}

string csv_row::get_status_string() const
{
    if (status.has_value())
        return test_case_status_to_string(status.value());
    return "?";
}

string csv_row::get_command_type_string() const
{
    if (command_type.has_value())
        return command_type_to_string(command_type.value());
    return "?";
}

string csv_row::get_time_ms_string() const
{
    if (time_ms.has_value())
        return to_n_digit_mantissa(time_ms.value(), 2);
    return "?";
}

#ifdef CSV_FIELD
#error CSV_FIELD macro already defined...
#endif

#define CSV_FIELD(field_optional, expr) \
    row_strings.push_back(field_optional.has_value() ? (expr) : "N/A"); \
    static_assert(true)

vector<string> csv_row::get_row_field_strings() const
{
    vector<string> row_strings;

    CSV_FIELD(file, file.value());
    CSV_FIELD(case_number, to_string(case_number.value()));
    CSV_FIELD(games, games.value());
    CSV_FIELD(player, player.value());
    CSV_FIELD(expected_result, expected_result.value());
    CSV_FIELD(result, result.value());
    CSV_FIELD(time_ms, get_time_ms_string());
    CSV_FIELD(status, get_status_string());
    CSV_FIELD(comments, comments.value());

    //CSV_FIELD(command_type, get_command_type_string());

    CSV_FIELD(node_count, to_string(node_count.value()));
    CSV_FIELD(unique_node_count, to_string(unique_node_count.value()));
    CSV_FIELD(input_hash, input_hash.value());

    return row_strings;
}

#undef CSV_FIELD

vector<string> csv_row::get_header_field_strings()
{
    vector<string> header;

    header.push_back("File");
    header.push_back("Case");
    header.push_back("Games");
    header.push_back("Player");
    header.push_back("Expected Result");
    header.push_back("Result");
    header.push_back("Time (ms)");
    header.push_back("Status");
    header.push_back("Comments");
    header.push_back("Node Count");
    header.push_back("Unique Sum Count");
    header.push_back("Input hash");

    return header;
}


////////////////////////////////////////////////// utility functions
void write_csv_field_strings(ostream& os, const vector<string>& fields)
{
    const size_t N = fields.size();

    for (size_t i = 0; i < N; i++)
    {
        os << csv_format_field(fields[i]);

        if (i + 1 < N)
            os << ',';
    }

    os << '\n';
}

