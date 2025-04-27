#pragma once
#include "strip.h"
#include <vector>

void assert_strip_split_result(const strip* g,
                               std::vector<std::string> expected);
void assert_no_split(const game* g);

template <class T>
void test_strip(const std::string& board,
                const std::vector<std::string>& expected, bool no_split = false)
{
    T pos(board);

    if (no_split)
    {
        assert_no_split(&pos);
    }
    else
    {
        assert_strip_split_result(&pos, expected);
    }
}
