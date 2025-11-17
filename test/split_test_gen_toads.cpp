#include "split_test_gen_toads.h"

#include <vector>
#include <cassert>
#include <iostream>
#include <string>
#include <tuple>

#include "split_test_utils.h"
#include "gen_toads.h"


using namespace std;

namespace {
////////////////////////////////////////////////// helpers

////////////////////////////////////////////////// main test functions
void test_no_split()
{
    // clang-format off
    vector<string> test_cases =
    {
    };
    // clang-format on

    for ([[maybe_unused]] const string& board : test_cases)
    {
        //gen_toads g(board);
        //assert_no_split(&g);
    }
}

void test_split()
{
    /*
       get<0> -- parameter list
       get<1> -- gen_toads board string
       get<2> -- Expected split result (as board strings)
    */
    typedef tuple<vector<int>, string, vector<string>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const vector<int>& params = get<0>(test_case);
        const string& board = get<1>(test_case);
        const vector<string>& exp_boards = get<2>(test_case);

        // TODO write
        gen_toads g(params, board);
        assert_no_split(&g);

        //assert(game_split_matches<gen_toads>(board, exp_boards));
    }
}


} // namespace

//////////////////////////////////////////////////
void split_test_gen_toads_all()
{
    cout << "GEN_TOADS SPLIT TEST" << endl;
    cout << "TODO check gen_toads::_split_impl() function pointer so test fails when someone implements it?" << endl;
    test_no_split();
    test_split();
}
