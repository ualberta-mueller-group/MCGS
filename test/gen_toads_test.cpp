#include "gen_toads_test.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <tuple>
#include <string>
#include <cassert>

#include "gen_toads.h"
#include "test/test_utilities.h"

using namespace std;

namespace {
////////////////////////////////////////////////// test logic helpers
string get_game_prefix(const vector<int>& param_vec)
{
    assert(param_vec.size() == 4);

    stringstream str;

    str << "gen_toads<";
    str << param_vec[0] << ", ";
    str << param_vec[1] << ", ";
    str << param_vec[2] << ", ";
    str << param_vec[3] << ">:";

    return str.str();
}

////////////////////////////////////////////////// main test functions
void test_moves_main()
{
    typedef tuple<vector<int>, string, vector<string>, vector<string>>
        test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            {1, 1, 0, 0},
            "",
            {
            },
            {
            },
        },

        {
            {1, 4, 2, 1},
            "",
            {
            },
            {
            },
        },

        {
            {1, 1, 0, 0},
            "X...XO...XXOO...XX...OX...X...O.....O",
            {
                ".X..XO...XXOO...XX...OX...X...O.....O",
                "X...XO...XXOO...X.X..OX...X...O.....O",
                "X...XO...XXOO...XX...O.X..X...O.....O",
                "X...XO...XXOO...XX...OX....X..O.....O",
            },
            {
                "X...XO...XXOO...XX..O.X...X...O.....O",
                "X...XO...XXOO...XX...OX...X..O......O",
                "X...XO...XXOO...XX...OX...X...O....O.",
            },
        },

        {
            {1, 2, 0, 0},
            "X...XO...XXOO...XX...OX...X...O.....O",
            {
                ".X..XO...XXOO...XX...OX...X...O.....O",
                "..X.XO...XXOO...XX...OX...X...O.....O",
                "X...XO...XXOO...X.X..OX...X...O.....O",
                "X...XO...XXOO...X..X.OX...X...O.....O",
                "X...XO...XXOO...XX...O.X..X...O.....O",
                "X...XO...XXOO...XX...O..X.X...O.....O",
                "X...XO...XXOO...XX...OX....X..O.....O",
                "X...XO...XXOO...XX...OX.....X.O.....O",
            },
            {
                "X...XO...XXOO...XX..O.X...X...O.....O",
                "X...XO...XXOO...XX.O..X...X...O.....O",
                "X...XO...XXOO...XX...OX...X..O......O",
                "X...XO...XXOO...XX...OX...X.O.......O",
                "X...XO...XXOO...XX...OX...X...O....O.",
                "X...XO...XXOO...XX...OX...X...O...O..",
            },
        },

        {
            {1, 2, 1, 0},
            "X...XO...XXOO...XX...OX...X...O.....O",
            {
                ".X..XO...XXOO...XX...OX...X...O.....O",
                "..X.XO...XXOO...XX...OX...X...O.....O",
                "X...XO...XXOO...X.X..OX...X...O.....O",
                "X...XO...XXOO...X..X.OX...X...O.....O",
                "X...XO...XXOO...XX...O.X..X...O.....O",
                "X...XO...XXOO...XX...O..X.X...O.....O",
                "X...XO...XXOO...XX...OX....X..O.....O",
                "X...XO...XXOO...XX...OX.....X.O.....O",
                "X....OX..XXOO...XX...OX...X...O.....O",
            },
            {
                "X...XO...XXOO...XX..O.X...X...O.....O",
                "X...XO...XXOO...XX.O..X...X...O.....O",
                "X...XO...XXOO...XX...OX...X..O......O",
                "X...XO...XXOO...XX...OX...X.O.......O",
                "X...XO...XXOO...XX...OX...X...O....O.",
                "X...XO...XXOO...XX...OX...X...O...O..",
                "X..OX....XXOO...XX...OX...X...O.....O",
            },
        },

        {
            {1, 2, 1, 1},
            "X...XO...XXOO...XX...OX...X...O.....O",
            {
                ".X..XO...XXOO...XX...OX...X...O.....O",
                "..X.XO...XXOO...XX...OX...X...O.....O",
                "X...XO...XXOO...X.X..OX...X...O.....O",
                "X...XO...XXOO...X..X.OX...X...O.....O",
                "X...XO...XXOO...XX...O.X..X...O.....O",
                "X...XO...XXOO...XX...O..X.X...O.....O",
                "X...XO...XXOO...XX...OX....X..O.....O",
                "X...XO...XXOO...XX...OX.....X.O.....O",
                "X....OX..XXOO...XX...OX...X...O.....O",
                "X...XO...XXOO....XX..OX...X...O.....O",
            },
            {
                "X...XO...XXOO...XX..O.X...X...O.....O",
                "X...XO...XXOO...XX.O..X...X...O.....O",
                "X...XO...XXOO...XX...OX...X..O......O",
                "X...XO...XXOO...XX...OX...X.O.......O",
                "X...XO...XXOO...XX...OX...X...O....O.",
                "X...XO...XXOO...XX...OX...X...O...O..",
                "X..OX....XXOO...XX...OX...X...O.....O",
            },
        },

        {
            {2, 3, 2, 0},
            "O....X....XO....XXOO....XOXO....X....O....O",
            {
                "O......X..XO....XXOO....XOXO....X....O....O",
                "O.......X.XO....XXOO....XOXO....X....O....O",
                "O....X....XO....XXOO....XOXO......X..O....O",
                "O....X....XO....XXOO....XOXO.......X.O....O",
                "O....X.....OX...XXOO....XOXO....X....O....O",
                "O....X....XO....X.OOX...XOXO....X....O....O",
                "O....X....XO....XXOO....XO.OX...X....O....O",
            },
            {
                "O....X....XO....XXOO....XOXO....X..O......O",
                "O....X....XO....XXOO....XOXO....X.O.......O",
                "O....X....XO....XXOO....XOXO....X....O..O..",
                "O....X....XO....XXOO....XOXO....X....O.O...",
                "O....X...OX.....XXOO....XOXO....X....O....O",
                "O....X....XO...OXX.O....XOXO....X....O....O",
                "O....X....XO....XXOO...OX.XO....X....O....O",
            },
        },

        {
            {2, 3, 2, 1},
            "O....X....XO....XXOO....XOXO....X....O....O",
            {
                "O......X..XO....XXOO....XOXO....X....O....O",
                "O.......X.XO....XXOO....XOXO....X....O....O",
                "O....X....XO....XXOO....XOXO......X..O....O",
                "O....X....XO....XXOO....XOXO.......X.O....O",
                "O....X.....OX...XXOO....XOXO....X....O....O",
                "O....X....XO....X.OOX...XOXO....X....O....O",
                "O....X....XO....XXOO....XO.OX...X....O....O",
            },
            {
                "O....X....XO....XXOO....XOXO....X..O......O",
                "O....X....XO....XXOO....XOXO....X.O.......O",
                "O....X....XO....XXOO....XOXO....X....O..O..",
                "O....X....XO....XXOO....XOXO....X....O.O...",
                "O....X...OX.....XXOO....XOXO....X....O....O",
                "O....X....XO...OXX.O....XOXO....X....O....O",
                "O....X....XO....XXOO...OX.XO....X....O....O",
            },
        },

        {
            {2, 3, 4, 1},
            "O....X....XO....XXOO....XOXO....X....O....O",
            {
                "O......X..XO....XXOO....XOXO....X....O....O",
                "O.......X.XO....XXOO....XOXO....X....O....O",
                "O....X....XO....XXOO....XOXO......X..O....O",
                "O....X....XO....XXOO....XOXO.......X.O....O",
                "O....X.....OX...XXOO....XOXO....X....O....O",
                "O....X....XO.....XOOX...XOXO....X....O....O",
                "O....X....XO....X.OOX...XOXO....X....O....O",
                "O....X....XO....XXOO.....OXOX...X....O....O",
                "O....X....XO....XXOO....XO.OX...X....O....O",
            },
            {
                "O....X....XO....XXOO....XOXO....X..O......O",
                "O....X....XO....XXOO....XOXO....X.O.......O",
                "O....X....XO....XXOO....XOXO....X....O..O..",
                "O....X....XO....XXOO....XOXO....X....O.O...",
                "O....X...OX.....XXOO....XOXO....X....O....O",
                "O....X....XO...OXX.O....XOXO....X....O....O",
                "O....X....XO...OXXO.....XOXO....X....O....O",
                "O....X....XO....XXOO...OX.XO....X....O....O",
                "O....X....XO....XXOO...OXOX.....X....O....O",
            },
        },

        {
            {1, 4, 10, 1},
            "O....X....XO....XXOO....XOXO....X....O....O",
            {
                "O.....X...XO....XXOO....XOXO....X....O....O",
                "O......X..XO....XXOO....XOXO....X....O....O",
                "O.......X.XO....XXOO....XOXO....X....O....O",
                "O........XXO....XXOO....XOXO....X....O....O",
                "O....X....XO....XXOO....XOXO.....X...O....O",
                "O....X....XO....XXOO....XOXO......X..O....O",
                "O....X....XO....XXOO....XOXO.......X.O....O",
                "O....X....XO....XXOO....XOXO........XO....O",
                "O....X.....OX...XXOO....XOXO....X....O....O",
                "O....X....XO.....XOOX...XOXO....X....O....O",
                "O....X....XO....X.OOX...XOXO....X....O....O",
                "O....X....XO....XXOO.....OXOX...X....O....O",
                "O....X....XO....XXOO....XO.OX...X....O....O",
            },
            {
                "O....X....XO....XXOO....XOXO....X...O.....O",
                "O....X....XO....XXOO....XOXO....X..O......O",
                "O....X....XO....XXOO....XOXO....X.O.......O",
                "O....X....XO....XXOO....XOXO....XO........O",
                "O....X....XO....XXOO....XOXO....X....O...O.",
                "O....X....XO....XXOO....XOXO....X....O..O..",
                "O....X....XO....XXOO....XOXO....X....O.O...",
                "O....X....XO....XXOO....XOXO....X....OO....",
                "O....X...OX.....XXOO....XOXO....X....O....O",
                "O....X....XO...OXX.O....XOXO....X....O....O",
                "O....X....XO...OXXO.....XOXO....X....O....O",
                "O....X....XO....XXOO...OX.XO....X....O....O",
                "O....X....XO....XXOO...OXOX.....X....O....O",
            },
        },
    };
    // clang-format on


    for (const test_case_t& test_case : test_cases)
    {
        const vector<int>& params = get<0>(test_case);
        const string& board = get<1>(test_case);
        const vector<string>& b_move_board_strings = get<2>(test_case);
        const vector<string>& w_move_board_strings = get<3>(test_case);

        gen_toads g(params, board);

        const string prefix = get_game_prefix(params);

        test_moves_as_strings_for_player(&g, BLACK, b_move_board_strings,
                                         prefix);

        test_moves_as_strings_for_player(&g, WHITE, w_move_board_strings,
                                         prefix);
    }
}

void test_constructors()
{
    typedef tuple<vector<int>, string, vector<int>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            {1, 1, 0, 0},
            "XOXO.X",
            {BLACK, WHITE, BLACK, WHITE, EMPTY, BLACK},
        },
        {
            {1, 2, 3, 1},
            "O..X.O",
            {WHITE, EMPTY, EMPTY, BLACK, EMPTY, WHITE},
        },
        {
            {2, 4, 1, 0},
            "OXX..XOXO",
            {WHITE, BLACK, BLACK, EMPTY, EMPTY, BLACK, WHITE, BLACK, WHITE},
        },
        {
            {1, 4, 3, 1},
            "...X..O.",
            {EMPTY, EMPTY, EMPTY, BLACK, EMPTY, EMPTY, WHITE, EMPTY},
        },
        {
            {1, 4, 20, 1},
            "XO..XO",
            {BLACK, WHITE, EMPTY, EMPTY, BLACK, WHITE},
        },
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const vector<int>& exp_params = get<0>(test_case);
        const string& board_str = get<1>(test_case);
        const vector<int>& exp_board = get<2>(test_case);

        gen_toads g(exp_params, board_str);

        assert(exp_params.size() == 4 &&                     //
               g.get_min_slide() == exp_params[0] &&         //
               g.get_max_slide() == exp_params[1] &&         //
               g.get_max_jump() == exp_params[2] &&          //
               g.get_friendly_jump() == (bool) exp_params[3] //
        );

        assert(g.board_const() == exp_board);
    }

    // Illegal parameters tests below
    {
        // Legal instance (sanity check)
        gen_toads g({1, 2, 1, 0}, "XO.");
    }

    // Too few parameters
    ASSERT_DID_THROW(gen_toads g({1, 2, 1}, "XO."););
    ASSERT_DID_THROW(gen_toads g({1, 2}, "XO."););
    ASSERT_DID_THROW(gen_toads g({1}, "XO."););
    ASSERT_DID_THROW(gen_toads g({}, "XO."););

    // Too many parameters
    ASSERT_DID_THROW(gen_toads g({1, 2, 1, 0, 0}, "XO."););
    ASSERT_DID_THROW(gen_toads g({1, 2, 1, 0, 1}, "XO."););
    ASSERT_DID_THROW(gen_toads g({1, 2, 1, 0, 2, 3}, "XO."););

    // Illegal parameter values
    // min_slide <= 0
    ASSERT_DID_THROW(gen_toads g({0, 2, 1, 0}, "XO."););
    ASSERT_DID_THROW(gen_toads g({-1, 2, 1, 0}, "XO."););
    ASSERT_DID_THROW(gen_toads g({-6, 2, 1, 0}, "XO."););
    ASSERT_DID_THROW(gen_toads g({0, 0, 1, 0}, "XO."););

    // min_slide > max_slide
    ASSERT_DID_THROW(gen_toads g({2, 1, 1, 0}, "XO."););
    ASSERT_DID_THROW(gen_toads g({3, 2, 1, 0}, "XO."););

    // max_jump < 0
    ASSERT_DID_THROW(gen_toads g({1, 2, -1, 0}, "XO."););
    ASSERT_DID_THROW(gen_toads g({1, 2, -4, 0}, "XO."););

    // friendly_jump not in [0, 1]
    ASSERT_DID_THROW(gen_toads g({1, 2, 1, -1}, "XO."););
    ASSERT_DID_THROW(gen_toads g({1, 2, 1, 2}, "XO."););
    ASSERT_DID_THROW(gen_toads g({1, 2, 1, 21}, "XO."););
    ASSERT_DID_THROW(gen_toads g({1, 2, 1, -40}, "XO."););
}


} // namespace


//////////////////////////////////////////////////
void gen_toads_test_all()
{
    cout << "TODO: " << __FILE__ << endl;
    test_constructors();
    test_moves_main();

    cout << "TODO: Finish CGSuite functions related to this file" << endl;
}
