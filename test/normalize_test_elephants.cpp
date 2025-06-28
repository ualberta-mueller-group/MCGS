#include "normalize_test_elephants.h"
#include "elephants.h"

#include <tuple>
#include <vector>
#include <string>
#include <cassert>

using namespace std;

void normalize_test_elephants()
{
    typedef tuple<string, string> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {"", ""},
        {"...", ""},
        {".", ""},
        {"...O...XO.OX.XX.", "...OXO.OX.XX."},
        {"..O.O.O.XX...X.X", "..O.O.OXX...X."},
        {"..O.X.X...OO.X.X", "..OX.X...OOX."},
        {"..OXXX...OXOOOX.", "..OXXX...OX."},
        {"..X..OOXXX....XO", "X..OOXXX...."},
        {"..XO.XOXO.XO..X.", "X."},
        {"..XOX......XO.O.", "X......XO.O"},
        {"..XX...OOO.XXO.X", "XX...OOO"},
        {".O.XX.XOOO.X.X..", ".OXX.XOX.X.."},
        {".X..OOO..OXXX..X", "X..OOO..OXXX.."},
        {".X.X..XOO...OOOX", "X.X..XO...OOO"},
        {"O..O..OX...X..XO", "..O..OX...X.."},
        {"O..X.XO.X.O.....", "X.XOX.O"},
        {"O..XXX.X..XOO..O", "XXX.X..XO..O"},
        {"O.X.X..XOO...X.O", "X.X..XOX.O"},
        {"O.XO.OX.X.OX...X", ".OX.X.OX..."},
        {"O.XOOO....X.XX.X", "X.XX."},
        {"O.XOX..X....XO..", "X..X...."},
        {"OO.....XX.X.XO.O", "XX.X.XO.O"},
        {"OO....XO...XX..X", "XX.."},
        {"OO.O.....X.XXOX.", ".OX.XOX."},
        {"OO.XX...O...X...", "XX...OX..."},
        {"OOX...X.X.OO.OXX", "X...X.X.OO.O"},
        {"OX....XX.XX.O.O.", "X....XX.XX.O.O"},
        {"OXO..XX.X..O..XO", "XX.X..O"},
        {"X..OO.X..OXO...X", "X..OOX..O"},
        {"X.O.OX.OO.O.XX.X", "X.O.OX.OO.OXX."},
        {"X.X.OO.X..X.OX.O", "X.X.OOX..X.OX.O"},
        {"XO.OXX.....X..O.", ".OXX.....X..O"},
        {"XOO..O...XX.OX.X", "..OXX.OX."}
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& input = get<0>(test_case);
        const string& expected = get<1>(test_case);

        elephants g(input);
        assert(g.board_as_string() == input);

        g.normalize();
        assert(g.board_as_string() == expected);

        g.undo_normalize();
        assert(g.board_as_string() == input);
    }
}
