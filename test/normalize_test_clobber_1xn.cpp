#include "normalize_test_clobber_1xn.h"
#include "clobber_1xn.h"

#include <vector>
#include <tuple>
#include <string>
#include <cassert>

using namespace std;

void normalize_test_clobber_1xn()
{
    typedef tuple<string, string> test_case_t;
    // input, expected

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {"", ""},
        {".", ""},
        {"X", ""},
        {"O", ""},
        {"XO", "XO"},
        {"OX....XO..X.X.O.XOXO", "OXOX.OX.XO"},
        {"XOOXX...XOOOXO...O.X", "XOOXX.XOOOXO"},
        {".O.X.X.OX.OO.X...XOO", "OX.XOO"},
        {"OX.X..XXO....O.OXXOX", "XOXXO.OXX.XO"},
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& input = get<0>(test_case);
        const string& expected = get<1>(test_case);

        clobber_1xn clob(input);

        assert(clob.board_as_string() == input);
        clob.normalize();
        assert(clob.board_as_string() == expected);
        clob.undo_normalize();
        assert(clob.board_as_string() == input);
    }
}
