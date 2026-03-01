#include "cannibal_clobber_test.h"
#include "cannibal_clobber.h"

#include "test/test_utilities.h"
#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include "grid_test_utilities.h"

using std::cout, std::endl, std::vector, std::string, std::tuple;

namespace {

const string CANNIBAL_CLOBBER_PREFIX = "cannibal_clobber:";

// check black/white outcomes: empty
void test_outcomes1()
{
    vector<string> boards = {
        "",            // 0x0
        ".",           // 1x1
        "..",          // 1x2
        "...",         // 1x3
        ".|.",         // 2x1
        "..|..",       // 2x2
        "...|...",     // 2x3
        ".|.|.",       // 3x1
        "..|..|..",    // 3x2
        "...|...|...", // 3x3
    };

    for (const string& b : boards)
    {
        cannibal_clobber c(b);

        assert_solve(c, BLACK, false);
        assert_num_moves(c, BLACK, 0);
        assert_solve(c, WHITE, false);
        assert_num_moves(c, WHITE, 0);
    }
}

// check black/white outcomes: zero
void test_outcomes2()
{
    // clang-format off
    vector<string> boards =
    {
        "XXO|...|OOX",
        "XO|XO",
        "XOXO|....|XO..|..XO|....|XOXO",
    };
    // clang-format on

    for (const string& b : boards)
    {
        cannibal_clobber c(b);

        assert_solve(c, BLACK, false);
        assert_solve(c, WHITE, false);
    }
}

// check black/white outcomes: various boards
void test_outcomes3()
{
    /*
       cannibal_clobber board string, result (black first), result (white first)
    */
    typedef tuple<string, bool, bool> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {"XO..|....|XXXO", true, false},
        {"XO..|....|OOOX|....", false, true},
        {"X|O|.|X", true, true},
        {"X.|O.|..|XX", true, false},
        {"XO.|...|OO.", false, true},
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const std::string& board = std::get<0>(test_case);
        const bool b_win = std::get<1>(test_case);
        const bool w_win = std::get<2>(test_case);

        cannibal_clobber c(board);

        assert_solve(c, BLACK, b_win);
        assert_solve(c, WHITE, w_win);
    }
}

// check move generator moves
void test_moves()
{
    /*
        board
        sequence of boards for black moves
        sequence of boards for white moves
    */
    typedef tuple<string, vector<string>, vector<string>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {"", {}, {}},

        {"..|..", {}, {}},

    {
        "XO",
        {
            ".X",
        },
        {
            "O.",
        },
    },

    {
        "XXO|.OX|..X",
        {
            ".XO|.OX|..X",
            "X.X|.OX|..X",
            "X.O|.XX|..X",
            "X.O|.OX|..X",
            "XXX|.O.|..X",
            "XXO|.O.|..X",
            "XXO|.X.|..X",
            "XXO|.OX|...",
        },
        {
            "XX.|.OO|..X",
            "XO.|.OX|..X",
            "XOO|..X|..X",
            "XXO|..O|..X",
        },
    },

    {
        "X..|O.O|..X",
        {
            "...|X.O|..X",
            "X..|O.X|...",
        },
        {
            "O..|..O|..X",
            "X..|O..|..O",
        },
    },

    {
        "XXO|OOX",
        {
            ".XO|OOX",
            ".XO|XOX",
            "X.X|OOX",
            "X.O|OXX",
            "X.O|OOX",
            "XXX|OO.",
            "XXO|OX.",
        },
        {
            "XX.|OOO",
            "XO.|OOX",
            "OXO|.OX",
            "XXO|.OX",
            "XOO|O.X",
            "XXO|O.O",
            "XXO|O.X",
        },
    },

    {
        "XO|OX|XO",
        {
            ".X|OX|XO",
            ".O|XX|XO",
            "XX|O.|XO",
            "XO|O.|XX",
            "XO|X.|XO",
            "XO|XX|.O",
            "XO|OX|.X",
        },
        {
            "X.|OO|XO",
            "O.|OX|XO",
            "OO|.X|XO",
            "XO|.O|XO",
            "XO|.X|OO",
            "XO|OO|X.",
            "XO|OX|O.",
        },
    },


    };
    // clang-format on

    vector<bw> colors = {BLACK, WHITE};

    for (const test_case_t& test_case : test_cases)
    {
        const std::string& board = std::get<0>(test_case);
        const vector<string>& b_options = std::get<1>(test_case);
        const vector<string>& w_options = std::get<2>(test_case);

        cannibal_clobber g(board);

        test_moves_as_strings_for_player(&g, BLACK, b_options,
                                         CANNIBAL_CLOBBER_PREFIX);

        test_moves_as_strings_for_player(&g, WHITE, w_options,
                                         CANNIBAL_CLOBBER_PREFIX);
    }
}

// check result from file
void test_from_file()
{
    assert_solve_test_file(UNIT_TEST_INPUT_DIR + "/cannibal_clobber.test", 12);
}

void test_inverse()
{
    // clang-format off
    vector<string> boards =
    {
        ".XOO|X.OX|...O|OXOO|X..X|OOO.|O..X|....",
        "..OX|XOXX|.X..|OOO.|OXOO|X...|.O.O|XX.O",
        "..OX|OOO.|.X..|X.XX|.OO.|O..O|.O.O|O.X.",
        "X..O|OX.X|..X.|..X.|O...|....|O.OX|X.O.",
        "..O.|..O.|OX.X|.O.X|OXOO|..OO|XOO.|XXO.",
        ".OOX|X...|.XO.|O.OX|...X|.OX.|..O.|..O.",
        ".X.O|OO..|..XO|....|XX.X|X..X|.OX.|XO..",
        "XX.X|.OO.|OXXX|XX.O|.OX.|O.O.|OXOO|OXOX",
        "O..O|XOX.|OXXX|.XOO|.O..|OX.O|.XXX|XOXO",
        "..OX|O..O|OX.O|.XO.|OX.X|.OXX|.XXX|..X.",
        "X.XO|.O.X|O..O|O.XX|XO..|OXXO|.O..|.OO.",
        ".O..|.OXO|X..X|..XX|.X.O|XO..|.O..|OX..",
        ".OOO|.XXO|XOX.|.X.X|OOOX|.X.O|X..X|..OO",
        "O.XX|XXOX|OXOX|..O.|..XO|.OOX|OOXX|.X..",
        ".X..|..XO|....|.XO.|XOOO|O.XO|.OO.|.OOX",
        "XOOX|.XX.|O.X.|..X.|..O.|X.O.|XO.O|O.OX",
        "O...|XXOO|X.XX|OXXO|..O.|OXX.|O...|XO.X",
        "X..X|O.XX|XOOO|...X|O...|...X|..XO|OOO.",
        "X.XX|OOXO|XO..|OOXX|..X.|O.XX|X..O|....",
        "..OX|OOO.|O...|OOX.|.O..|XX.O|..XX|.OOO",
    };
    // clang-format on

    for (const string& board : boards)
        assert_grid_game_inverse<cannibal_clobber>(board);
}
} // namespace

void cannibal_clobber_test_all()
{
    test_outcomes1();
    test_outcomes2();
    test_outcomes3();
    test_moves();
    test_from_file();
    test_inverse();
}
