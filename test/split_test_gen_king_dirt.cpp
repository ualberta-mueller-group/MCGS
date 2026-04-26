#include "split_test_gen_king_dirt.h"

#include "gen_king_dirt.h"
#include "split_test_utils.h"

#include <iostream>
#include <vector>
#include <tuple>
#include <string>

using namespace std;


////////////////////////////////////////////////// Utilities
namespace {

void test_game_split(
    const game& g,
    const std::vector<std::string>& exp_split_as_strings)
{
    std::unordered_multiset<std::string> exp_split_printed;
    std::unordered_multiset<std::string> found_split_printed;

    for (const std::string& exp_game_as_string : exp_split_as_strings)
        exp_split_printed.emplace(exp_game_as_string);

    split_result sr = g.split();
    assert(sr.has_value());

    for (game* g2 : *sr)
    {
        found_split_printed.emplace(g2->to_string());
        delete g2;
    }

    assert(exp_split_printed == found_split_printed);
}

////////////////////////////////////////////////// Main test functions
void test_no_split()
{
    // clang-format off
    typedef tuple<vector<int>, string> test_case_t;

    const vector<test_case_t> test_cases =
    {
        //////////////////// No stones to place
        {
            {0, 0, 0},
            "..X",
        },

        {
            {0, 0, 0},
            "..#..|..#..|..#X.|..#..|...#.",
        },

        //////////////////// Stones to place
        {
            {1, 1, 1},
            "..#.|..#.|..#.|..#.",
        },
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const vector<int>& params = get<0>(test_case);
        const string& board = get<1>(test_case);

        gen_king_dirt g(params, board);
        assert_no_split(&g);
    }
}

void test_split()
{
    // clang-format off
    /*
       - Parameters
       - Board string
       - Expected split result (as game strings)
    */
    typedef tuple<vector<int>, string, vector<string>> test_case_t;

    vector<test_case_t> test_cases =
    {
        //////////////////// No stones to place
        {
            {0, 0, 1},
            "..#.|X.#.|..#O|..#.",
            {
                "<0, 0, 0>:..|X.|..|..",
                "<0, 0, 0>:.|.|O|.",
            },
        },

        {
            {0, 0, 1},
            "..#.|X.#.|..#.|###.",
            {
                "<0, 0, 0>:..|X.|..",
            },
        },

        {
            {0, 0, 0},
            "...|...|...",
            {
            },
        },

        {
            {0, 0, 0},
            "..#.|..#.|..#.|..#.",
            {
            },
        },

        {
            {1, 1, 1},
            "##|##",
            {
            },
        },

        {
            {1, 1, 1},
            "###|XOX|###",
            {
            },
        },

        {
            {0, 0, 0},
            "#.#.#|.#.#.",
            {
            },
        },

        //////////////////// Stones to place
        {
            {1, 1, 1},
            "...|.##|.#X",
            {
                "<1, 1, 1>:...|.##|.##",
            },
        },

        {
            {1, 1, 1},
            "#...|##..|##..|#.X.",
            {
                "<1, 1, 1>:...|#..|#..|.X.",
            },
        },

        {
            {1, 1, 1},
            "X#...#O|##...##|...#...|..#X#..|...#...|##...##|O#...#X",
            {
                "<1, 1, 1>:##...##|##...##|...#...|..#X#..|...#...|##...##|##...##",
            },
        },

        {
            {1, 1, 1},
            "..##|..#X|..##|..##",
            {
                "<1, 1, 1>:..|..|..|..",
            },
        },


    };
    // clang-format on

    const string GEN_KING_DIRT_PREFIX = "gen_king_dirt";

    for (test_case_t& test_case : test_cases)
    {
        const vector<int>& params = get<0>(test_case);
        const string& board = get<1>(test_case);
        vector<string>& exp_split_result = get<2>(test_case);

        for (string& str : exp_split_result)
            str = GEN_KING_DIRT_PREFIX + str;

        gen_king_dirt g(params, board);
        test_game_split(g, exp_split_result);
    }

}

} // namespace

//////////////////////////////////////////////////
void split_test_gen_king_dirt_all()
{
    test_no_split();
    test_split();
}
