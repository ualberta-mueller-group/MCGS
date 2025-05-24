#include  "clobber_test.h"
#include "cgt_move.h"
#include "clobber.h"

#include "test/test_utilities.h"
#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <memory>

using std::cout, std::endl, std::vector, std::string, std::tuple;
using cgt_move::two_part_move;

namespace {

// check black/white outcomes: empty
void test_outcomes1()
{
    vector<string> boards =
    {
        "", // 0x0
        ".", // 1x1
        "..", // 1x2
        "...", // 1x3
        ".|.", // 2x1
        "..|..", // 2x2
        "...|...", // 2x3
        ".|.|.", // 3x1
        "..|..|..", // 3x2
        "...|...|...", // 3x3
    };

    for (const string& b : boards)
    {
        clobber c(b);

        assert_solve(c, BLACK, false);
        assert_num_moves(c, BLACK, 0);
        assert_solve(c, WHITE, false);
        assert_num_moves(c, WHITE, 0);
    }

}

// check black/white outcomes: zero
void test_outcomes2()
{
    vector<string> boards =
    {
        "XXO|...|OOX",
        "XO|XO",
        "XOXO|....|XO..|..XO|....|XOXO",
    };

    for (const string& b : boards)
    {
        clobber c(b);

        assert_solve(c, BLACK, false);
        assert_solve(c, WHITE, false);
    }
}

// check black/white outcomes: various boards
void test_outcomes3()
{
    typedef tuple<string, bool, bool> test_case_t;

    vector<test_case_t> test_cases =
    {
        {"XO..|....|XXXO", true, false},
        {"XO..|....|OOOX|....", false, true},
        {"X|O|.|X", true, true},
    };

    for (const test_case_t& test_case : test_cases)
    {
        const std::string& board = std::get<0>(test_case);
        const bool b_win = std::get<1>(test_case);
        const bool w_win = std::get<2>(test_case);

        clobber c(board);

        assert_solve(c, BLACK, b_win);
        assert_solve(c, WHITE, w_win);
    }
}

// check move generator moves
void test_moves()
{
    typedef tuple<string, vector<string>, vector<string>> test_case_t;
    /*
        board
        sequence of boards for black moves
        sequence of boards for white moves
    */

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {"", {}, {}},

        {"..|..", {}, {}},

        {"XO", {".X"}, {"O."}},

        {"XXO|.OX|..X",
            {"X.X|.OX|..X", "X.O|.XX|..X", "XXX|.O.|..X", "XXO|.X.|..X"},
            {"XX.|.OO|..X", "XO.|.OX|..X", "XOO|..X|..X", "XXO|..O|..X"},
        },

        {"X..|O.O|..X",
            {"...|X.O|..X", "X..|O.X|..."},
            {"O..|..O|..X", "X..|O..|..O"},
        },

        {"XXO|OOX",
            {".XO|XOX", "X.X|OOX", "X.O|OXX", "XXX|OO.", "XXO|OX."},
            {"XX.|OOO", "XO.|OOX", "OXO|.OX", "XOO|O.X", "XXO|O.O"}
        },

        {"XO|OX|XO",
            {
                ".X|OX|XO",
                ".O|XX|XO",
                "XX|O.|XO",
                "XO|O.|XX",
                "XO|X.|XO",
                "XO|XX|.O",
                "XO|OX|.X"
            },
            {
                "X.|OO|XO",
                "O.|OX|XO",
                "OO|.X|XO",
                "XO|.O|XO",
                "XO|.X|OO",
                "XO|OO|X.",
                "XO|OX|O."
            },
        }

    };
    // clang-format on

    vector<bw> colors = {BLACK, WHITE};

    for (const test_case_t& test_case : test_cases)
    {
        const std::string& board = std::get<0>(test_case);
        const vector<string>& b_options = std::get<1>(test_case);
        const vector<string>& w_options = std::get<2>(test_case);

        clobber c(board);
        assert(c.board_as_string() == board);

        for (const bw color : colors)
        {
            const vector<string>& options = color == BLACK ? b_options : w_options;
            std::unique_ptr<move_generator> mg(c.create_move_generator(color));

            size_t i = 0;
            const size_t N = options.size();
            for (; i < N; i++)
            {
                assert(*mg);
                move m = mg->gen_move();
                ++(*mg);
                c.play(m, color);
                assert(c.board_as_string() == options[i]);
                c.undo_move();
                assert(c.board_as_string() == board);
            }

            assert(!(*mg));
        }
    }
}

// check result from file
void test_from_file()
{
    assert_solve_test_file(UNIT_TEST_INPUT_DIR + "/clobber.test", 12);
}

void test_inverse()
{

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

    for (const string& board : boards)
    {
        string board_inv = board;
        for (char& c : board_inv)
        {
            if (c == 'O')
                c = 'X';
            else if (c == 'X')
                c = 'O';
        }

        clobber clob(board);
        game* g = clob.inverse();
        clobber* clob_inv = dynamic_cast<clobber*>(g);
        assert(clob_inv != nullptr);
        assert(clob_inv->board_as_string() == board_inv);
        delete g;
    }
}
} // namespace

void clobber_test_all()
{
    test_outcomes1();
    test_outcomes2();
    test_outcomes3();
    test_moves();
    test_from_file();
    test_inverse();
}
