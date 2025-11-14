#include "nogo_test.h"
#include "nogo.h"
#include <tuple>
#include <vector>
#include <string>
#include <cassert>
#include "grid_test_utilities.h"
#include "test_utilities.h"
#include <exception>

using namespace std;

namespace {

void test_outcomes()
{
    /*
       nogo board
       result (black first)
       result (white first)
    */
    typedef tuple<string, bool, bool> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {"", false, false},
        {".", false, false},
        {"..", true, true},
        {".|.", true, true},
        {"..|..", true, true},
        {"XO|..", true, true},
        {"XO|.O", false, false},
        {"X.X|XXX|...", true, false},
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& board = get<0>(test_case);
        const bool b_win = get<1>(test_case);
        const bool w_win = get<2>(test_case);

        nogo g(board);

        assert_solve(g, BLACK, b_win);
        assert_solve(g, WHITE, w_win);
    }
}

void test_moves()
{
    /*
       nogo board, black options, white options
    */
    typedef tuple<string, vector<string>, vector<string>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {"", {}, {}},
        {".", {}, {}},
        {"..", {"X.", ".X"}, {"O.", ".O"}},
        {"..|..",
            {"X.|..",".X|..", "..|X.", "..|.X"},
            {"O.|..",".O|..", "..|O.", "..|.O"},
        },
        {"X.X|.X.|.O.|O.O",
            {
                "XXX|.X.|.O.|O.O",
                "X.X|XX.|.O.|O.O",
                "X.X|.XX|.O.|O.O",
                "X.X|.X.|XO.|O.O",
                "X.X|.X.|.OX|O.O",
            },
            {
                "X.X|OX.|.O.|O.O",
                "X.X|.XO|.O.|O.O",
                "X.X|.X.|OO.|O.O",
                "X.X|.X.|.OO|O.O",
                "X.X|.X.|.O.|OOO",
            },
        },
        {".X.|X.O|.O.",
            {
                "XX.|X.O|.O.",
                ".XX|X.O|.O.",
                ".X.|XXO|.O.",
                ".X.|X.O|XO.",
            },
            {
                ".XO|X.O|.O.",
                ".X.|XOO|.O.",
                ".X.|X.O|OO.",
                ".X.|X.O|.OO",
            },
        },
        {"XXX.OOO|OOO.XXX|XXX.OOO|OOO.XXX", {}, {}},

    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& board = get<0>(test_case);
        const vector<string>& black_options = get<1>(test_case);
        const vector<string>& white_options = get<2>(test_case);

        assert_grid_game_move_sequence<nogo>(board, black_options,
                                             white_options);
    }
}

void test_from_file()
{
    assert_solve_test_file(UNIT_TEST_INPUT_DIR + "/nogo.test", 18);
}

void test_inverse()
{
    // clang-format off
    vector<string> boards =
    {
        "X.O.|.O..|....|....",
        "..X.|....|.O..|XO..",
        "..O.|X...|..X.|....",
        "....|O...|....|X...",
        "..XX|....|...O|.O..",
        "....|..OX|.O..|....",
        "X...|....|O...|....",
        "....|....|.O..|.OX.",
        "...X|O...|....|X...",
        "....|....|.OO.|...X",
        "..O.|....|.X..|....",
        "....|....|...O|XX..",
        "O...|..X.|....|....",
        "....|...X|..O.|....",
        "....|X...|O.O.|....",
        "...X|X...|....|O...",
        "O...|.O..|....|.X..",
        "..O.|....|XO..|....",
        "....|....|..XO|....",
        ".X.O|...O|....|....",
        "..O.O|X..O.|....X|..O..",
        "..OX.|XX.X.|O....|.O...",
        "XOXO.|.O.O.|.....|...OX",
        "X.OX.|X....|O..OO|..X..",
        ".O...|O.O..|..OX.|X....",
        "OX..X|.O.X.|..O..|O.X.O",
        ".OOX.|.....|....X|XO...|.O.X.",
        ".O...|XX...|.X..O|O..O.|X....",
        "X..O.|...X.|X....|X.OO.|O....",
    };
    // clang-format on

    for (const string& board : boards)
        assert_grid_game_inverse<nogo>(board);
}

void test_exceptions()
{
    // clang-format off
    vector<string> boards =
    {
        ".........X....XOX.XOO.X.X.O...",
        "......OX....O.XXOX..O.XX......",
        "......X..XOXX.O...O.....O.XX..",
        "....XO..X...OXO......X...X..OX",
        "...OXXX.XO......XOX........O..",
        "...X.....XO.XX..........O.XXOO",
        "...XOX......O......XXOX.....OX",
        "..O.....X.OXO.X.XO.....X.X....",
        "..O..O....XXOX......OX...X...X",
        "..OX........XX.OXXO..X.....O..",
        ".O..........O....XO..X.X.XOX.X",
        ".OO.OX.X....X........X..XOX...",
        ".OXO..X.....X......X.X....OOX.",
        ".X....O....X...X.O...XOXO...X.",
        ".X....X...X..X.....O.X.O..OXO.",
        ".X...X...X.X...X..O....O...XOO",
        ".XOX.....O.X....XO.XXO........",
        ".XXO.....X..OOX...X.........XO",
        "O......XX......OX...OX.....XXO",
        "O.X.X.XOX.X.X.O.....O.........",
        "OX.....X.X.....XO....OX.X.O...",
        "X.....X..OX...X..O.....XOX..O.",
        "X....OXOX..OX..X........X...O.",
        "X..O....X.X.X.OXO...........OX",
        "X.O.....O.X.X.XX.OXO..........",
        "XO...........O.OX.X.X.O...X..X",
        "XO..O......XX...X.O.....X.XO..",
        "XOX..XXOO..X.X........O.......",
        "XXOX.O.......X......OXO....X..",
        "O.OOXO|XX.OXX|.XO.X.|......|X..OO.|..XO.X",
        "X...X.|.O.X.O|..X.XO|O...XO|O.O..X|O.X.XO",
        "OX.X.X|X.XO.O|O.XOX.|XOO..O|..OO.X|X.O...",
        ".X..O.|OO.OXO|.O.OOX|XOX...|.....O|.XO.XX",
        "OX.X..|OXOX..|XXXOOX|.OO.O.|..X...|O..XO.",
        "OX.|X..",
    };
    // clang-format on

    for (const string& board : boards)
    {
        bool did_throw = false;

        try
        {
            nogo g(board);
        }
        catch (exception& e)
        {
            did_throw = true;
        }

        assert(did_throw);
    }
}

} // namespace

void nogo_test_all()
{
    test_outcomes();
    test_moves();
    test_from_file();
    test_inverse();
    test_exceptions();
}
