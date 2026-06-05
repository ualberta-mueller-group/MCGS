#include "gen_king_dirt_test.h"

#include <sstream>
#include <vector>
#include <string>
#include <cassert>
#include <tuple>

#include "gen_king_dirt.h"
#include "test_utilities.h"
#include "int_pair.h"
#include "cgt_basics.h"


using namespace std;

namespace {
////////////////////////////////////////////////// Helpers
//////////////////////////////////////// class parameterized_game_state
class parameterized_game_state
{
public:
    vector<int> params;
    string board;

    string to_string() const;
};

//////////////////////////////////////// Misc functions
string join_parameters(const vector<int>& params)
{
    stringstream str;

    bool not_first = false;
    for (const int p : params)
    {
        if (not_first)
            str << ", ";

        str << p;

        not_first = true;
    }

    return str.str();
}

vector<string> states_to_strings(const vector<parameterized_game_state>& states)
{
    vector<string> strings;
    strings.reserve(states.size());

    for (const parameterized_game_state& state : states)
        strings.emplace_back(state.to_string());

    return strings;
}

//////////////////////////////////////// parameterized_game_state methods
string parameterized_game_state::to_string() const
{
    stringstream str;

    str << '<';
    str << join_parameters(params);
    str << ">:";
    str << board;

    return str.str();
}

//void gen_tests()
//{
//    vector<parameterized_game_state> states =
//    {
//    };
//
//    for (const parameterized_game_state& state : states)
//    {
//        gen_king_dirt g(state.params, state.board);
//        const vector<string> b_opts = get_subgame_options(&g, BLACK);
//        const vector<string> w_opts = get_subgame_options(&g, WHITE);
//
//        cout << "{" << endl;
//
//        cout << "\t{{" << join_parameters(state.params) << "}, \"";
//        cout << state.board << "\"},\n";
//
//        cout << "\t{" << endl;
//        for (const string& opt : b_opts)
//            cout << "\t\t" << opt << endl;
//        cout << "\t}," << endl;
//
//        cout << "\t{" << endl;
//        for (const string& opt : w_opts)
//            cout << "\t\t" << opt << endl;
//        cout << "\t}," << endl;
//
//        cout << "}," << endl;
//        cout << endl;
//    }
//
//}


////////////////////////////////////////////////// Main test functions
const string GEN_KING_DIRT_PREFIX = "gen_king_dirt";

#ifdef TEST_CONSTRUCTOR_IMPL
#error Macro already defined!
#else
#define TEST_CONSTRUCTOR_IMPL(g)                                               \
    {                                                                          \
        assert(params.size() == gen_king_dirt::PARAM_COUNT);                   \
                                                                               \
        assert(g.shape() == shape);                                            \
        assert(g.board_const() == board);                                      \
                                                                               \
        assert(exp_params[gen_king_dirt::PIDX_BLACK_UNPLACED] ==               \
                   g.get_black_unplaced() &&                                   \
               exp_params[gen_king_dirt::PIDX_WHITE_UNPLACED] ==               \
                   g.get_white_unplaced() &&                                   \
               exp_params[gen_king_dirt::PIDX_MUST_PLACE] ==                   \
                   g.get_must_place());                                        \
                                                                               \
        assert(exp_string == g.to_string());                                   \
    }                                                                          \
    static_assert(true)
#endif

void test_constructors()
{
    // clang-format off
    /*
        - Parameters
        - Board string
        - Shape
        - Board
        - Expected parameters
    */
    typedef tuple<vector<int>, string, int_pair, vector<int>, vector<int>>
        test_case_t;

    vector<test_case_t> test_cases =
    {
        {
            {0, 0, 0},
            "...|...",
            {2, 3},
            {
                EMPTY, EMPTY, EMPTY,
                EMPTY, EMPTY, EMPTY,
            },
            {0, 0, 0},
        },

        {
            {0, 0, 1},
            "..|..",
            {2, 2},
            {
                EMPTY, EMPTY,
                EMPTY, EMPTY,
            },
            {0, 0, 0},
        },

        {
            {10, 10, 1},
            ".#.|.X.|..O",
            {3, 3},
            {
                EMPTY, BORDER, EMPTY,
                EMPTY, BLACK, EMPTY,
                EMPTY, EMPTY, WHITE,
            },
            {6, 6, 1},
        },

        {
            {2, 2, 1},
            "XO|#X",
            {2, 2},
            {
                BLACK, WHITE,
                BORDER, BLACK,
            },
            {0, 0, 0},
        },
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const vector<int>& params = get<0>(test_case);
        const string& board_string = get<1>(test_case);
        const int_pair& shape = get<2>(test_case);
        const vector<int>& board = get<3>(test_case);
        const vector<int>& exp_params = get<4>(test_case);

        const string exp_string = GEN_KING_DIRT_PREFIX + "<" +
                                  join_parameters(exp_params) +
                                  ">:" + board_string;

        gen_king_dirt g1(params, board, shape);
        gen_king_dirt g2(params, board_string);

        TEST_CONSTRUCTOR_IMPL(g1);
        TEST_CONSTRUCTOR_IMPL(g2);
    }

    {
        // Shouldn't throw
        gen_king_dirt g1({1, 0, 1}, "..|..");
        gen_king_dirt g2({1, 0, 1}, {EMPTY, EMPTY}, {1, 2});
    }

    // Bad parameter tests
    ASSERT_DID_THROW(gen_king_dirt g({1, 0}, "..|.."););
    ASSERT_DID_THROW(gen_king_dirt g({1, 0}, {EMPTY, EMPTY}, {1, 2}););

    ASSERT_DID_THROW(gen_king_dirt g({1, 0, 2}, "..|.."););
    ASSERT_DID_THROW(gen_king_dirt g({1, 0, 2}, {EMPTY, EMPTY}, {1, 2}););

    ASSERT_DID_THROW(gen_king_dirt g({1, 0, -1}, "..|.."););
    ASSERT_DID_THROW(gen_king_dirt g({1, 0, -1}, {EMPTY, EMPTY}, {1, 2}););

    ASSERT_DID_THROW(gen_king_dirt g({1, 0, 1, 0}, "..|.."););
    ASSERT_DID_THROW(gen_king_dirt g({1, 0, 1, 0}, {EMPTY, EMPTY}, {1, 2}););

    ASSERT_DID_THROW(gen_king_dirt g({-1, 0, 0}, "..|.."););
    ASSERT_DID_THROW(gen_king_dirt g({-1, 0, 0}, {EMPTY, EMPTY}, {1, 2}););

    ASSERT_DID_THROW(gen_king_dirt g1({1, 0, 1}, "..|."));
    ASSERT_DID_THROW(gen_king_dirt g1({1, 0, 1}, "..|..|"));

}

#undef TEST_CONSTRUCTOR_IMPL

void test_moves()
{
    // clang-format off
    typedef tuple<
        parameterized_game_state,
        vector<parameterized_game_state>,
        vector<parameterized_game_state>
    > test_case_t;

    vector<test_case_t> test_cases =
    {
        {
            {{0, 0, 0}, ""},
            {
            },
            {
            },
        },

        {
            {{1, 1, 0}, ""},
            {
            },
            {
            },
        },

        {
            {{0, 0, 0}, "."},
            {
            },
            {
            },
        },

        {
            {{0, 0, 1}, "."},
            {
            },
            {
            },
        },

        {
            {{1, 1, 0}, "..|.."},
            {
                {{0, 1, 0}, "X.|.."},
                {{0, 1, 0}, ".X|.."},
                {{0, 1, 0}, "..|X."},
                {{0, 1, 0}, "..|.X"},
            },
            {
                {{1, 0, 0}, "O.|.."},
                {{1, 0, 0}, ".O|.."},
                {{1, 0, 0}, "..|O."},
                {{1, 0, 0}, "..|.O"},
            },
        },

        {
            {{2, 3, 0}, ".|."},
            {
                {{1, 1, 0}, "X|."},
                {{1, 1, 0}, ".|X"},
            },
            {
                {{1, 1, 0}, "O|."},
                {{1, 1, 0}, ".|O"},
            },
        },

        {
            {{3, 4, 1}, ".."},
            {
                {{1, 1, 1}, "X."},
                {{1, 1, 1}, ".X"},
            },
            {
                {{1, 1, 1}, "O."},
                {{1, 1, 1}, ".O"},
            },
        },

        {
            {{2, 3, 0}, ".#.|#X.|#.O"},
            {
                {{1, 3, 0}, "X#.|#X.|#.O"},
                {{1, 3, 0}, ".#X|#X.|#.O"},
                {{1, 3, 0}, ".#.|#XX|#.O"},
                {{1, 3, 0}, ".#.|#X.|#XO"},
                {{2, 3, 0}, ".#X|##.|#.O"},
                {{2, 3, 0}, ".#.|##X|#.O"},
                {{2, 3, 0}, ".#.|##.|#XO"},
                {{2, 3, 0}, "X#.|##.|#.O"},
            },
            {
                {{2, 2, 0}, "O#.|#X.|#.O"},
                {{2, 2, 0}, ".#O|#X.|#.O"},
                {{2, 2, 0}, ".#.|#XO|#.O"},
                {{2, 2, 0}, ".#.|#X.|#OO"},
                {{2, 3, 0}, ".#.|#XO|#.#"},
                {{2, 3, 0}, ".#.|#X.|#O#"},
            },
        },

        {
            {{3, 2, 1}, ".O.|..X"},
            {
                {{2, 2, 1}, "XO.|..X"},
                {{2, 2, 1}, ".OX|..X"},
                {{2, 2, 1}, ".O.|X.X"},
                {{2, 2, 1}, ".O.|.XX"},
            },
            {
                {{3, 1, 1}, "OO.|..X"},
                {{3, 1, 1}, ".OO|..X"},
                {{3, 1, 1}, ".O.|O.X"},
                {{3, 1, 1}, ".O.|.OX"},
            },
        },

        {
            {{1, 1, 1}, "."},
            {
                {{0, 0, 0}, "X"},
            },
            {
                {{0, 0, 0}, "O"},
            },
        },

        {
            {{1, 1, 0}, "."},
            {
                {{0, 0, 0}, "X"},
            },
            {
                {{0, 0, 0}, "O"},
            },
        },

        {
            {{2, 2, 0}, "."},
            {
                {{0, 0, 0}, "X"},
            },
            {
                {{0, 0, 0}, "O"},
            },
        },

        {
            {{1, 0, 1}, ".O."},
            {
                {{0, 0, 0}, "XO."},
                {{0, 0, 0}, ".OX"},
            },
            {
                {{1, 0, 1}, ".#O"},
                {{1, 0, 1}, "O#."},
            },
        },

        {
            {{9, 9, 0}, ".."},
            {
                {{1, 1, 0}, "X."},
                {{1, 1, 0}, ".X"},
            },
            {
                {{1, 1, 0}, "O."},
                {{1, 1, 0}, ".O"},
            },
        },

        {
            {{9, 9, 1}, ".."},
            {
                {{1, 1, 1}, "X."},
                {{1, 1, 1}, ".X"},
            },
            {
                {{1, 1, 1}, "O."},
                {{1, 1, 1}, ".O"},
            },
        },
    };
    // clang-format on


    for (const test_case_t& test_case : test_cases)
    {
        const parameterized_game_state& initial_state = get<0>(test_case);
        const vector<parameterized_game_state>& exp_black_states =
            get<1>(test_case);
        const vector<parameterized_game_state>& exp_white_states =
            get<2>(test_case);

        gen_king_dirt g(initial_state.params, initial_state.board);

        const vector<string> exp_black_moves =
            states_to_strings(exp_black_states);
        const vector<string> exp_white_moves =
            states_to_strings(exp_white_states);

        test_moves_as_strings_for_player(&g, BLACK, exp_black_moves, GEN_KING_DIRT_PREFIX);
        test_moves_as_strings_for_player(&g, WHITE, exp_white_moves, GEN_KING_DIRT_PREFIX);
    }
}

} // namespace

//////////////////////////////////////////////////
void gen_king_dirt_test_all()
{
    test_constructors();
    test_moves();
}

