#include "split_test_elephants.h"
#include <tuple>
#include <string>
#include <vector>

#include "elephants.h"
#include "split_test_utils.h"

using namespace std;

void split_test_elephants_all()
{
    /*
       elephants board
       split result
    */
    typedef tuple<string, vector<string>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            ".......X.....O..X.....O.......",
            {
                "X.....O",
                "X.....O",
            },
        },

        {
            "....OXO.....X......X.XOX..O...O...O.XXO.",
            {
                "....O",
                "X......X.",
                "X..O...O...O",
            },
        },

        {
            ".OXO...O.O..O.X.X......X...X..OO..X....X",
            {
                ".O",
                "...O.O..O",
                "X.X......X...X..OO",
                "X....",
            },
        },

        {
            "X.......O.....XO.O.....X..OO.X.O.OOXO...",
            {
                "X.......O",
                ".O",
                "X..OO",
                "X.O.OO",
            },
        },

        {
            "X...XX.O...X.....OOOO..X...O.....O.O...O",
            {
                "X...XX.O",
                "X.....OOOO",
                "X...O.....O.O...O",
            },
        },

        {
            "X..X........O....X....O...O.......XO.X..",
            {
                "X..X........O",
                "X....O...O",
                "X..",
            },
        },

        {
            "XX...XO.O.X..X.O..O.O.X.O.X..X..X.....O..OOO",
            {
                "XX...",
                ".O",
                "X..X.O..O.O",
                "X.O",
                "X..X..X.....O..OOO",
            },
        },

    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& input = get<0>(test_case);
        const vector<string>& expected = get<1>(test_case);

        elephants g(input);
        assert_strip_split_result(&g, expected);
    }

    vector<string> no_split = {
        "X..X.X..X..O...OOO",  //
        "X..X.X.X..X...O.O..O" //
    };

    for (const string& board : no_split)
    {
        elephants g(board);
        assert_no_split(&g);
    }
}
