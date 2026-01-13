#include "csv_row.h"
#include "cgt_basics.h"
#include "global_options.h"
#include "solver_stats.h"
#include "test_case_enums.h"
#include "utilities.h"
#include "game.h"
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <optional>
#include <cstddef>
#include <cassert>
#include <iostream>


using namespace std;

namespace {

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

string optional_double_to_string(optional<double> val)
{
    if (!val.has_value())
        return CSV_MISSING_TEXT;

    return to_n_digit_mantissa(val.value(), 2);
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
    const test_case_status_enum status =
        evaluate_test_case_status(result, this->expected_result);

    fill_post_test_fields_verbose(result, time_ms, status);
}

void csv_row::fill_post_test_fields_verbose(
    const optional<string>& alt_result, double time_ms,
    test_case_status_enum test_case_status)
{
    assert(has_pre_test_fields());
    assert(!has_post_test_fields());

    this->result = alt_result;
    this->time_ms = time_ms;

    this->status = test_case_status;

    const solver_stats& stats = stats::get_global_stats();

    this->tt_hits = stats.tt_hits;
    this->tt_misses = stats.tt_misses;
    this->tt_hit_rate = stats.get_tt_hit_rate();

    this->db_hits = stats.db_hits;
    this->db_misses = stats.db_misses;
    this->db_hit_rate = stats.get_db_hit_rate();

    this->node_count = stats.search_node_count;
    if (stats.search_node_hashes.has_value())
        this->unique_node_count = stats.search_node_hashes->size();
    this->max_depth = stats.max_search_depth;

    this->initial_subgame_count = stats.initial_subgame_count;
    this->max_subgame_count = stats.max_subgame_count;

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
    return CSV_MISSING_TEXT;
}

string csv_row::get_command_type_string() const
{
    if (command_type.has_value())
        return command_type_to_string(command_type.value());
    return CSV_MISSING_TEXT;
}

string csv_row::get_time_ms_string() const
{
    return optional_double_to_string(time_ms);
}

#ifdef CSV_FIELD
#error CSV_FIELD macro already defined...
#endif

#define CSV_FIELD(field_optional, expr) \
    row_strings.push_back(field_optional.has_value() ? (expr) : CSV_MISSING_TEXT); \
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

    CSV_FIELD(command_type, get_command_type_string());

    CSV_FIELD(tt_hits, to_string(tt_hits.value()));
    CSV_FIELD(tt_misses, to_string(tt_misses.value()));
    CSV_FIELD(tt_hit_rate, optional_double_to_string(tt_hit_rate));

    CSV_FIELD(db_hits, to_string(db_hits.value()));
    CSV_FIELD(db_misses, to_string(db_misses.value()));
    CSV_FIELD(db_hit_rate, optional_double_to_string(db_hit_rate));

    CSV_FIELD(node_count, to_string(node_count.value()));
    CSV_FIELD(unique_node_count, to_string(unique_node_count.value()));
    CSV_FIELD(max_depth, to_string(max_depth.value()));

    CSV_FIELD(initial_subgame_count, to_string(initial_subgame_count.value()));
    CSV_FIELD(max_subgame_count, to_string(max_subgame_count.value()));

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

    header.push_back("Type");

    header.push_back("TT Hits");
    header.push_back("TT Misses");
    header.push_back("TT Hit Rate");

    header.push_back("DB Hits");
    header.push_back("DB Misses");
    header.push_back("DB Hit Rate");

    header.push_back("Node Count");
    header.push_back("Unique Node Count");
    header.push_back("Max Depth");

    header.push_back("Initial Subgames");
    header.push_back("Max Subgames");

    header.push_back("Input hash");

    return header;
}

string csv_row::csv_game_string(const vector<game*>& games)
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

    os << endl;
}

