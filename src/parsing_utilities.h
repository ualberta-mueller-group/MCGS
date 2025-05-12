#pragma once
#include <string>
#include <vector>
#include <cstddef>
#include "fraction.h"
#include "game_case.h"
#include "search_utils.h"

struct run_command_t
{
    run_command_t():
        player(EMPTY),
        expected_value()
    {
    }

    ebw player;
    search_value expected_value;
};

////////////////////////////////////////////////// parsing functions
std::vector<std::string> get_string_tokens(const std::string& line,
                                 const std::vector<char>& special_chars);

inline bool is_comma(const std::string& str)
{
    return str == ",";
}

inline bool is_slash(const std::string& str)
{
    return str == "/";
}

/*  "get_X()" helper functions

    These functions will:

    Return true on success, false on failure.
    Increment idx as they consume from string_tokens.
    Not cause memory errors (i.e. when idx is past the range of string_tokens).

    NOTE: The following functions may increment "idx" even on failure:
        get_fraction
        get_run_command

    TODO: Fix this?
*/
bool get_star(const std::vector<std::string>& string_tokens, size_t& idx, bool& val);

bool get_int(const std::vector<std::string>& string_tokens, size_t& idx, int& val);

bool get_win_loss(const std::vector<std::string>& string_tokens, size_t& idx, bool& win);

bool get_player(const std::vector<std::string>& string_tokens, size_t& idx, ebw& player);

bool get_fraction(const std::vector<std::string>& string_tokens, size_t& idx,
                  std::vector<fraction>& fracs);

bool consume_optional_comma(const std::vector<std::string>& string_tokens, size_t& idx);

bool consume_mandatory_comma(const std::vector<std::string>& string_tokens, size_t& idx);

bool get_fraction_list(const std::string& line, std::vector<fraction>& fracs);

bool get_run_command(const std::vector<std::string>& string_tokens, size_t& idx, std::vector<run_command_t>& run_commands);

bool get_run_command_list(const std::string& line, std::vector<run_command_t>& commands);
