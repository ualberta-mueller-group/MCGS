#include "csv_row.h"
#include "cgt_basics.h"
#include "global_options.h"
#include "parsing_utilities.h"
#include "search_utils.h"
#include "solver_stats.h"
#include "test_case_enums.h"
#include "utilities.h"
#include <cctype>

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
