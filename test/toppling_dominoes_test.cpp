#include "toppling_dominoes_test.h"
// TODO add more tests now that it no longer inherits from strip?

#include <tuple>
#include <string>
#include <cassert>
#include <vector>

#include "cgt_basics.h"
#include "game.h"
#include "test_utilities.h"
#include "toppling_dominoes.h"

using namespace std;

namespace {
const string TOPPLING_DOMINOES_PREFIX = "toppling_dominoes:";
////////////////////////////////////////////////// main test functions

void test_moves_main()
{
    /*
       toppling dominoes board string, all black moves, all white moves
    */
    typedef tuple<string, vector<string>, vector<string>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            "",
            {
            },
            {
            },
        },

        {
            "X",
            {
                "",
            },
            {
            },
        },

        {
            "O",
            {
            },
            {
                ""
            },
        },

        {
            "#",
            {
                ""
            },
            {
                ""
            },
        },

        {
            "XXX",
            {
                "",
                "X",
                "XX",
            },
            {
            },
        },

        {
            "OOO",
            {
            },
            {
                "",
                "O",
                "OO",
            },
        },

        {
            "###",
            {
                "",
                "#",
                "##",
            },
            {
                "",
                "#",
                "##",
            },
        },

        {
            "XO#XO",
            {
                "",
                "O#XO",
                "XO",
                "O",
                "XO#",
            },
            {
                "#XO",
                "X",
                "XO",
                "",
                "XO#X",
            },
        },

        {
            "XXOO",
            {
                "",
                "X",
                "XOO",
                "OO",
            },
            {
                "",
                "O",
                "XXO",
                "XX",
            },
        },

        {
            "#XOO#X",
            {
                "XOO#X",
                "",
                "OO#X",
                "#",
                "X",
                "#XOO",
                "#XOO#",
            },
            {
                "XOO#X",
                "",
                "O#X",
                "#X",
                "#XO",
                "X",
                "#XOO",
            },
        },
    };
    // clang-format on


    for (const test_case_t& test_case : test_cases)
    {
        const string& board = get<0>(test_case);
        const vector<string>& b_move_board_strings = get<1>(test_case);
        const vector<string>& w_move_board_strings = get<2>(test_case);

        toppling_dominoes g(board);

        test_moves_as_strings_for_player(&g, BLACK, b_move_board_strings,
                                         TOPPLING_DOMINOES_PREFIX);

        test_moves_as_strings_for_player(&g, WHITE, w_move_board_strings,
                                         TOPPLING_DOMINOES_PREFIX);
    }
}

void test_constructors()
{
    /*
        board string, expected board
    */
    typedef tuple<string, vector<int>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            "",
            {
            },
        },

        {
            "X",
            {
                BLACK,
            },
        },

        {
            "O",
            {
                WHITE,
            },
        },

        {
            "#",
            {
                BORDER,
            },
        },

        {
            "XXX",
            {
                BLACK, BLACK, BLACK,
            },
        },

        {
            "OOO",
            {
                WHITE, WHITE, WHITE,
            },
        },

        {
            "###",
            {
                BORDER, BORDER, BORDER,
            },
        },

        {
            "XO#XO",
            {
                BLACK, WHITE, BORDER, BLACK, WHITE,
            },
        },

        {
            "XXOO",
            {
                BLACK, BLACK, WHITE, WHITE,
            },
        },

        {
            "#XOO#X",
            {
                BORDER, BLACK, WHITE, WHITE, BORDER, BLACK,
            },
        },

    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& board = get<0>(test_case);
        const vector<int>& exp_board = get<1>(test_case);

        toppling_dominoes g(board);

        assert(g.current_dominoes() == exp_board);
    }

    // test 0 size games
    {
        toppling_dominoes g("");
        assert_num_moves(g, BLACK, 0);
        assert_num_moves(g, WHITE, 0);
    }

    // check invalid strings
    ASSERT_DID_THROW(toppling_dominoes g("X."));
    ASSERT_DID_THROW(toppling_dominoes g("A"));
    ASSERT_DID_THROW(toppling_dominoes g(".X"));
    ASSERT_DID_THROW(toppling_dominoes g("."));
}

} // namespace

void toppling_dominoes_test_all()
{
    test_constructors();
    test_moves_main();
}
