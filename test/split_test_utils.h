#pragma once
#include "strip.h"

void assert_strip_split_result(const strip* g, std::vector<std::string> expected, bool non_empty = false);
void assert_no_split(const game* g, bool non_empty = false);

template <class T>
void test_strip(const std::string& board, const vector<std::string>& expected, bool no_split = false, bool non_empty = false)
{
    T pos(board);

    if (no_split)
    {
        assert_no_split(&pos, non_empty);
    } else
    {
        assert_strip_split_result(&pos, expected, non_empty);
    }

}

template <class T>
void test_strip_non_empty(const std::string& board, const vector<std::string>& expected, bool no_split = false)
{
    test_strip<T>(board, expected, no_split, true);
}

