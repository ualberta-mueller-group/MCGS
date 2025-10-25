#include "toppling_dominoes_test.h"
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <string>
#include <vector>
#include "cgt_basics.h"
#include "game.h"
#include "test_utilities.h"
#include "toppling_dominoes.h"

using namespace std;

namespace {
////////////////////////////////////////////////// general utility
// TODO these are general enough to extract elsewhere
string game_to_string(const game* g)
{
    stringstream str;
    str << *g;
    return str.str();
}

////////////////////////////////////////////////// test logic helpers
const string GAME_STRING_PREFIX = "toppling_dominoes:";

unordered_set<string> get_exp_move_strings(
    const vector<string>& exp_move_board_strings)
{
    unordered_set<string> strs;

    for (const string& board : exp_move_board_strings)
        strs.insert(GAME_STRING_PREFIX + board);

    return strs;
}

void test_moves_for_player(toppling_dominoes* g, bw player,
                           const vector<string>& exp_move_board_strings)
{
    assert(g != nullptr &&        //
           is_black_white(player) //
    );

    unordered_set<string> exp_move_strings =
        get_exp_move_strings(exp_move_board_strings);

    unordered_set<string> gen_move_strings;

    move_generator* gen = g->create_move_generator(player);

    while (*gen)
    {
        assert_restore_game arg(*g);

        const ::move m = gen->gen_move();
        ++(*gen);

        const string before_string = game_to_string(g);
        const hash_t before_hash = g->get_local_hash();

        g->play(m, player);
        const string after_string = game_to_string(g);
        const hash_t after_hash = g->get_local_hash();

        g->undo_move();
        const string undo_string = game_to_string(g);
        const hash_t undo_hash = g->get_local_hash();

        assert(before_string == undo_string && //
               before_hash == undo_hash        //
        );

        assert(before_string != after_string && //
               before_hash != after_hash        //
        );

        gen_move_strings.insert(after_string);
    }

    delete gen;
    assert(exp_move_strings == gen_move_strings);
}

////////////////////////////////////////////////// main test functions

void test_moves_main()
{
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

        test_moves_for_player(&g, BLACK, b_move_board_strings);
        test_moves_for_player(&g, WHITE, w_move_board_strings);
    }

}

void test_constructors()
{
    // test string constructors
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
        assert(g.board_const() == exp_board);
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
    cout << "TODO: " << __FILE__ << endl;
    test_constructors();
    test_moves_main();
}
