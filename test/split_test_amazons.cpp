#include "split_test_amazons.h"

#include <vector>
#include <cassert>
#include <iostream>
#include <string>
#include <tuple>

#include "split_test_utils.h"
#include "amazons.h"


using namespace std;

namespace {
////////////////////////////////////////////////// helpers

////////////////////////////////////////////////// main test functions
void test_no_split()
{
    // clang-format off
    vector<string> test_cases =
    {
        "X..#|..#.|.#..|#...",
        "...#|..#.|.#..|#.O.",
        "...|.X.|...",
        ".....|O....|X####|X....|..O..",
    };
    // clang-format on

    for (const string& board : test_cases)
    {
        amazons g(board);
        assert_no_split(&g);
    }
}

void test_split()
{
    /*
       get<0> -- Amazons board string
       get<1> -- Expected split result (as board strings)
    */
    typedef tuple<string, vector<string>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            "...|...|...",
            {
            },
        },

        {
            "...|.#.|...",
            {
            },
        },

        {
            "X",
            {
            },
        },

        {
            "XO",
            {
            },
        },

        {
            "X#.|##.|...",
            {
            },
        },

        {
            "X#X|##O|.##",
            {
            },
        },


        {
            "X.#.O",
            {
                "X.",
                ".O",
            },
        },

        {
            "..#.|.X#.|..#.",
            {
                "..|.X|..",
            },
        },

        {
            "..#.|.X#O|..##",
            {
                "..|.X|..",
                ".|O",
            },
        },

        {
            "...#|.O#.|####|X.#.",
            {
                "...#|.O#.",
                "X.",
            },
        },

        {
            "...#|.###|##..|#.O.",
            {
                "#..|.O.",
            },
        },

        {
            ".#.X|###.|O.#.|..##",
            {
                ".X|#.|#.",
                "O.|..",
            },
        },

        {
            "..#O.|O.X#.|X####|X..#.|..O#.",
            {
                "..#O.|O.X#.|X####|X..##|..O##",
            },
        },
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& board = get<0>(test_case);
        const vector<string> exp_boards = get<1>(test_case);

        assert(game_split_matches<amazons>(board, exp_boards));
    }
}


} // namespace

//////////////////////////////////////////////////
void split_test_amazons_all()
{
    cout << "AMAZONS SPLIT TEST" << endl;
    test_no_split();
    test_split();
}
