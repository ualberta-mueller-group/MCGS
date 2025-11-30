/*
    NOTE: This game doesn't split
*/
#include "split_test_toppling_dominoes.h"

#include <vector>
#include <cassert>
#include <string>

#include "split_test_utils.h"
#include "test/test_utilities.h"
#include "toppling_dominoes.h"

using namespace std;

namespace {
////////////////////////////////////////////////// helpers

////////////////////////////////////////////////// main test functions
void test_no_split()
{
    // clang-format off
    vector<string> test_cases =
    {
        "",
        "X",
        "O",
        "#",
        "XXX",
        "OOO",
        "###",
        "XO#XO",
        "XXOO",
        "#XOO#X",
    };
    // clang-format on

    for (const string& board : test_cases)
    {
        toppling_dominoes g(board);
        assert_no_split(&g);
    }
}

} // namespace

//////////////////////////////////////////////////
void split_test_toppling_dominoes_all()
{
    test_no_split();
    ASSERT_DID_THROW(toppling_dominoes("X."));
}
