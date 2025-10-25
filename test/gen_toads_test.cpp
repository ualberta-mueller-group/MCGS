#include "gen_toads_test.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <tuple>
#include <unordered_set>
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
            "X..XO..XXOO..XX..OX..X..O...O",
            {
                ".X.XO..XXOO..XX..OX..X..O...O",
                "X..XO..XXOO..X.X.OX..X..O...O",
                "X..XO..XXOO..XX..O.X.X..O...O",
                "X..XO..XXOO..XX..OX...X.O...O",
            },
            {
                "X..XO..XXOO..XX.O.X..X..O...O",
                "X..XO..XXOO..XX..OX..X.O....O",
                "X..XO..XXOO..XX..OX..X..O..O.",
            },
        },
        {
            {1, 2, 0, 0},
            "X..XO..XXOO..XX..OX..X..O...O",
            {
                ".X.XO..XXOO..XX..OX..X..O...O",
                "..XXO..XXOO..XX..OX..X..O...O",
                "X..XO..XXOO..X.X.OX..X..O...O",
                "X..XO..XXOO..X..XOX..X..O...O",
                "X..XO..XXOO..XX..O.X.X..O...O",
                "X..XO..XXOO..XX..O..XX..O...O",
                "X..XO..XXOO..XX..OX...X.O...O",
                "X..XO..XXOO..XX..OX....XO...O",
            },
            {
                "X..XO..XXOO..XX.O.X..X..O...O",
                "X..XO..XXOO..XXO..X..X..O...O",
                "X..XO..XXOO..XX..OX..X.O....O",
                "X..XO..XXOO..XX..OX..XO.....O",
                "X..XO..XXOO..XX..OX..X..O..O.",
                "X..XO..XXOO..XX..OX..X..O.O..",
            },
        },
        {
            {1, 2, 1, 0},
            "X..XO..XXOO..XX..OX..X..O...O",
            {
                ".X.XO..XXOO..XX..OX..X..O...O",
                "..XXO..XXOO..XX..OX..X..O...O",
                "X..XO..XXOO..X.X.OX..X..O...O",
                "X..XO..XXOO..X..XOX..X..O...O",
                "X..XO..XXOO..XX..O.X.X..O...O",
                "X..XO..XXOO..XX..O..XX..O...O",
                "X..XO..XXOO..XX..OX...X.O...O",
                "X..XO..XXOO..XX..OX....XO...O",
                "X...OX.XXOO..XX..OX..X..O...O",
            },
            {
                "X..XO..XXOO..XX.O.X..X..O...O",
                "X..XO..XXOO..XXO..X..X..O...O",
                "X..XO..XXOO..XX..OX..X.O....O",
                "X..XO..XXOO..XX..OX..XO.....O",
                "X..XO..XXOO..XX..OX..X..O..O.",
                "X..XO..XXOO..XX..OX..X..O.O..",
                "X.OX...XXOO..XX..OX..X..O...O",
            },
        },
        {
            {1, 2, 1, 1},
            "X..XO..XXOO..XX..OX..X..O...O",
            {
                ".X.XO..XXOO..XX..OX..X..O...O",
                "..XXO..XXOO..XX..OX..X..O...O",
                "X..XO..XXOO..X.X.OX..X..O...O",
                "X..XO..XXOO..X..XOX..X..O...O",
                "X..XO..XXOO..XX..O.X.X..O...O",
                "X..XO..XXOO..XX..O..XX..O...O",
                "X..XO..XXOO..XX..OX...X.O...O",
                "X..XO..XXOO..XX..OX....XO...O",
                "X...OX.XXOO..XX..OX..X..O...O",
                "X..XO..XXOO...XX.OX..X..O...O",
            },
            {
                "X..XO..XXOO..XX.O.X..X..O...O",
                "X..XO..XXOO..XXO..X..X..O...O",
                "X..XO..XXOO..XX..OX..X.O....O",
                "X..XO..XXOO..XX..OX..XO.....O",
                "X..XO..XXOO..XX..OX..X..O..O.",
                "X..XO..XXOO..XX..OX..X..O.O..",
                "X.OX...XXOO..XX..OX..X..O...O",
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

        cout << params << endl;
        cout << board << endl << endl;

        const string prefix = get_game_prefix(params);

        test_moves_as_strings_for_player(&g, BLACK, b_move_board_strings,
                                         prefix);

        test_moves_as_strings_for_player(&g, WHITE, w_move_board_strings,
                                         prefix);
    }
}

void test_constructors()
{

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
