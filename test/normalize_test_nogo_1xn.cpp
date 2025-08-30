#include "normalize_test_nogo_1xn.h"
#include "nogo_1xn.h"

#include <vector>
#include <tuple>
#include <string>
#include <cassert>

using namespace std;

void normalize_test_nogo_1xn()
{
    typedef tuple<string, string> test_case_t;
    // input, expected

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {".", "."},
        {"....OO.XXO..X.X.OO.O", "O.O.X.X..OX.O...."},
        {"O.OXXX...X..OX.XX..X", "X..X.XO..X...XO.O"},
        {"..O..OOX..XX.OX.X..O", "O..X.XO.X..XO..O.."},
        {"OO.X.O...OXXX.XXO.XX", "X.OX.XO...O.X.O"},
        {"OOO.XO..X...X.O....O", "O.XO..X...X.O....O"},

    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& input = get<0>(test_case);
        const string& expected = get<1>(test_case);

        nogo_1xn n(input);

        assert(n.board_as_string() == input);
        n.normalize();

        assert(n.board_as_string() == expected);
        n.undo_normalize();
        assert(n.board_as_string() == input);
    }
}
