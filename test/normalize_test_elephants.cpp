#include "normalize_test_elephants.h"
#include "elephants.h"

#include <vector>
#include <string>
#include <cassert>

using namespace std;

void normalize_test_elephants()
{
    // clang-format off
    vector<string> boards =
    {
        "OX....XX.XX.O.O.",
        "X.O.OX.OO.O.XX.X",
        "OO.....XX.X.XO.O",
        "X.X.OO.X..X.OX.O",
        "O..O..OX...X..XO",
        "...O...XO.OX.XX.",
        "..XX...OOO.XXO.X",
        ".X.X..XOO...OOOX",
        "OOX...X.X.OO.OXX",
        "O..XXX.X..XOO..O",
        "O.XO.OX.X.OX...X",
        "O.X.X..XOO...X.O",
        "..X..OOXXX....XO",
        "OO.XX...O...X...",
        "OXO..XX.X..O..XO",
        "O.XOX..X....XO..",
        "..O.X.X...OO.X.X",
        ".O.XX.XOOO.X.X..",
        "O.XOOO....X.XX.X",
        "XO.OXX.....X..O.",
        "..O.O.O.XX...X.X",
        "X..OO.X..OXO...X",
        "..XOX......XO.O.",
        "..OXXX...OXOOOX.",
        ".X..OOO..OXXX..X",
        "OO.O.....X.XXOX.",
        "XOO..O...XX.OX.X",
        "O..X.XO.X.O.....",
        "OO....XO...XX..X",
        "..XO.XOXO.XO..X.",
        "",
        "...",
        ".",
    };
    // clang-format on

    for (const string& board : boards)
    {
        elephants e(board);

        assert(e.board_as_string() == board);
        e.normalize();
        assert(e.board_as_string() == board);
        e.undo_normalize();
        assert(e.board_as_string() == board);
    }
}
