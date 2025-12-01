/*
    Helper types/functions for parsing.
*/
#pragma once
#include <string>
#include <vector>
#include <cstddef>
#include "fraction.h"
#include "game_case.h"
#include "search_utils.h"

struct run_command_t
{
    run_command_t() : player(EMPTY), expected_value() {}

    ebw player;
    search_value expected_value;
};

////////////////////////////////////////////////// parsing functions

/*
    Calls split_string() on the line, after surrounding special characters with
    spaces
*/
std::vector<std::string> get_string_tokens(
    const std::string& line, const std::vector<char>& special_chars);

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

        TODO: Fix this later? OK for now
*/
bool get_star(const std::vector<std::string>& string_tokens, size_t& idx,
              bool& val);

bool get_int(const std::vector<std::string>& string_tokens, size_t& idx,
             int& val);

bool get_win_loss(const std::vector<std::string>& string_tokens, size_t& idx,
                  bool& win);

// EMPTY for impartial game, i.e. "{N 5}" in .test file
bool get_player(const std::vector<std::string>& string_tokens, size_t& idx,
                ebw& player);

// also matches ints
bool get_fraction(const std::vector<std::string>& string_tokens, size_t& idx,
                  std::vector<fraction>& fracs);

// succeeds IFF no comma, or comma with input afterward
bool consume_optional_comma(const std::vector<std::string>& string_tokens,
                            size_t& idx);

bool consume_mandatory_comma(const std::vector<std::string>& string_tokens,
                             size_t& idx);

/*
    Also matches empty list

   Spaces or commas can separate list items, but commas cannot be at the
   end of the list

   i.e.
       "1, 1/2, 3/ 4, 3 / 4"
       "1  /  4  4"
       " 3 1 / 4 6 "
       ""
    are all valid
*/
bool get_fraction_list(const std::string& line, std::vector<fraction>& fracs);

// like get_fraction_list() but for ints
bool get_int_list(const std::string& line, std::vector<int>& ints);

bool get_run_command(const std::vector<std::string>& string_tokens, size_t& idx,
                     std::vector<run_command_t>& run_commands);

bool get_run_command_list(const std::string& line,
                          std::vector<run_command_t>& commands);
