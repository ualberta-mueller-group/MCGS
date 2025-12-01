#include "split_test_domineering.h"

#include <vector>
#include <string>
#include <tuple>
#include <cassert>

#include "split_test_utils.h"
#include "domineering.h"

using namespace std;

namespace {
////////////////////////////////////////////////// helpers

////////////////////////////////////////////////// main test functions
void test_no_split()
{
    // clang-format off
    vector<string> test_cases =
    {
        "..|..",
        ".....|.....|.....",
        "#.#..|...#.|.#...",
        "..",
        ".|.",
        "...",
        ".|.|.",
        ".#.|.#.|...",
        ".#.|...|.#.",
        "#.#|...|#.#",
    };
    // clang-format on

    for (const string& board : test_cases)
    {
        domineering g(board);
        assert_no_split(&g);
    }
}

void test_split()
{
    /*
       get<0> -- Domineering board string
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
            ".#",
            {
            },
        },

        {
            "##|##",
            {
            },
        },

        {
            "#",
            {
            },
        },

        {
            "##",
            {
            },
        },

        {
            "#|#",
            {
            },
        },

        {
            ".#.#.#|#.#.#.|.#.#.#|#.#.#.|.#.#.#",
            {
            },
        },

        {
            ".#.",
            {
            },
        },

        {
            ".#.#.#|#.#.#.|.#.#.#|#...#.|.#.#.#",
            {
                "#.#|...|#.#",
            },
        },

        {
            "#...|.#..|..#.|...#",
            {
                ".##|..#|...",
                "...|#..|##.",
            },
        },

        {
            "..#.|.#..|#...",
            {
                "##.|#..|...",
                "..|.#",
            },
        },

        {
            ".#..",
            {
                "..",
            },
        },

        {
            ".|#|.|.",
            {
                ".|.",
            },
        },

        {
            ".#...#..",
            {
                "...",
                "..",
            },
        },

    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& board = get<0>(test_case);
        const vector<string>& exp_boards = get<1>(test_case);

        assert(game_split_matches<domineering>(board, exp_boards));
    }
}

} // namespace

//////////////////////////////////////////////////
void split_test_domineering_all()
{
    test_no_split();
    test_split();
}
