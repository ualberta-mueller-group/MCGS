#pragma once

#include <vector>
#include <unordered_set>
#include <cassert>
#include <string>

#include "strip.h"
#include "grid.h"

void assert_strip_split_result(const strip* g,
                               std::vector<std::string> expected);

void assert_grid_split_result(const grid* g, std::vector<std::string> expected);

void assert_no_split(const game* g);

template <class T>
void test_strip_split(const std::string& board,
                      const std::vector<std::string>& expected,
                      bool no_split = false)
{
    T pos(board);
    if (no_split)
        assert_no_split(&pos);
    else
        assert_strip_split_result(&pos, expected);
}

template <class T>
void test_grid_split(const std::string& board,
                     const std::vector<std::string>& expected,
                     bool no_split = false)
{
    T pos(board);
    if (no_split)
        assert_no_split(&pos);
    else
        assert_grid_split_result(&pos, expected);
}

template <class T>
bool game_split_matches(
    const std::string& game_as_string,
    const std::vector<std::string>& exp_split_as_strings)
{
    static_assert(std::is_base_of_v<game, T>);

    T g(game_as_string);

    std::unordered_multiset<std::string> exp_split_printed;
    std::unordered_multiset<std::string> found_split_printed;

    for (const std::string& exp_game_as_string : exp_split_as_strings)
    {
        T g2(exp_game_as_string);
        exp_split_printed.emplace(g2.to_string());
    }

    split_result sr = g.split();
    assert(sr.has_value());

    for (game* g2 : *sr)
    {
        found_split_printed.emplace(g2->to_string());
        delete g2;
    }

    return exp_split_printed == found_split_printed;
}
