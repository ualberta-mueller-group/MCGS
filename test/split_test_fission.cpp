#include "split_test_fission.h"

#include <vector>
#include <string>
#include <tuple>
#include <iostream>

#include "split_test_utils.h"
#include "fission.h"

using namespace std;

namespace {
////////////////////////////////////////////////// helpers

////////////////////////////////////////////////// main test functions
void test_no_split()
{
    // clang-format off
    vector<string> test_cases =
    {
        "...|.X.|...",
        ".X.",
        ".|X|.",
        "...|.X.",
        "..|.X|..",
    };
    // clang-format on

    for (const string& board : test_cases)
    {
        fission g(board);
        assert_no_split(&g);
    }
}

void test_split()
{
    /*
       get<0> -- Fission board string
       get<1> -- Expected split result (as board strings)
    */
    typedef tuple<string, vector<string>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            ".",
            {
            },
        },

        {
            "#",
            {
            },
        },

        {
            "...|.#.|...",
            {
            },
        },

        {
            ".#.",
            {
            },
        },

        {
            ".|#|.",
            {
            },
        },

        {
            "X.|..",
            {
            },
        },

        {
            "X...X|XX...|.XX..|..XX.",
            {
            },
        },

        {
            "..#..|X.#.X|..###|..#..",
            {
                "..|X.|..|..",
            },
        },

        {
            "X..X|....|X...|....|X..X",
            {
                "#..#|....|X...|....|#..#",
            },
        },

        {
            "X.X.X|.X.XX|X.X.X|.X.X.|X.XXX",
            {
                "#.X.#|.X.X#|X.X.#|.X.X.|#.###",
            },
        },

        {
            "X....X|.X..XX|...XX.|..XX..|.XX.X.|XX....",
            {
                "#####.|####..|###.X.|##....",
                "#....|.X..#|...##|..###|.####",
            },
        },
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& board = get<0>(test_case);
        const vector<string>& exp_boards = get<1>(test_case);

        // TODO uncomment this after implementing fission::_split_impl() !!
        //assert(game_split_matches<fission>(board, exp_boards));

        fission g(board);
        assert_no_split(&g);
    }
}

} // namespace

//////////////////////////////////////////////////
void split_test_fission_all()
{
    cout << "FISSION SPLIT TEST" << endl;
    test_no_split();
    test_split();
}
