#include "split_test_sheep.h"

#include <string>
#include <tuple>
#include <cassert>
#include <type_traits>
#include <unordered_set>
#include <vector>

#include "sheep.h"
#include "test/split_test_utils.h"

using namespace std;

namespace {

////////////////////////////////////////////////// helpers
// TODO move this elsewhere?
template <class T>
bool game_split_matches(
    const std::string& game_as_string,
    const std::vector<std::string>& exp_split_as_strings)
{
    static_assert(std::is_base_of_v<game, T>);

    T g(game_as_string);

    std::unordered_multiset<std::string> exp_split_printed;
    std::unordered_multiset<std::string> found_split_printed;

    for (const std::string& exp_game_as_string : exp_split_as_strings)
    {
        T g2(exp_game_as_string);
        exp_split_printed.emplace(g2.to_string());
    }

    split_result sr = g.split();
    assert(sr.has_value());

    for (game* g2 : *sr)
    {
        found_split_printed.emplace(g2->to_string());
        delete g2;
    }

    return exp_split_printed == found_split_printed;
}

////////////////////////////////////////////////// main test functions
void split_test_main()
{
    /*
       get<0>: Input board
       get<1>: Expected boards from split
    */
    typedef tuple<string, vector<string>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            "0 3 0 0 | 1 -1 1 1 | 0 0 0 0",
            {
                "0 3 0 0",
            },
        },

        {
            "0 3 0 0 | 1 -1 1 1 | 0 0 -1 0",
            {
                "0 3 0 0",
            },
        },

        {
            "0 0 1 0 | 0 2 1 -3 | 0 0 -1 0",
            {
                "0 | -3 | 0",
                "0 0 | 0 2 | 0 0",
            },
        },

        {
            "0 0 6 0 | 1 -1 1 -1 | 0 0 -4 0",
            {
                "0 0 6 0",
                "0 0 -4 0",
            },
        },

        {
            "0 0 -1 0 1 | -2 0 0 1 0 | 0 0 1 0 0 | 0 1 0 0 3",
            {
                "0 0 1 0 | -2 0 0 1 | 0 0 1 1 | 0 1 1 1",
                "1 1 0 | 1 0 0 | 0 0 3",
            },
        },

        {
            "0 0 0 0 | 0 0 0 0 | 0 0 0 0",
            {
            },
        },

        {
            "0 0 1 0 | 0 0 1 0 | 0 0 1 0",
            {
            },
        },

        {
            "0 0 1 0 | 1 0 -1 -1 | 0 0 1 0",
            {
            },
        },

        {
            "0 0 1 0 | 1 0 1 -1 | 0 0 1 0",
            {
            },
        },

        {
            "1 0 0 1 0 0 1 | 0 1 0 1 0 1 0 | 0 0 1 1 1 0 0 | 1 1 1 1 1 1 1 | "
            "0 0 1 1 1 0 0 | 0 1 0 1 0 1 0 | 1 0 0 1 0 0 1",
            {
            },
        },
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& input_board = get<0>(test_case);
        const vector<string>& exp_split = get<1>(test_case);

        assert(game_split_matches<sheep>(input_board, exp_split));
    }


}

void split_test_nosplit()
{
    // clang-format off
    vector<string> test_cases =
    {
        "0 0 0 0 0 | 3 0 0 0 0 | 0 0 0 0 0",
        "1 0 0 0 0 | 0 1 0 0 0 | 3 0 1 0 0 | 0 0 0 1 0",
        "1 1 4 0 0 | 0 1 1 0 0 | 0 0 1 0 0 | 0 0 0 1 0",
    };
    // clang-format on

    for (const string& input_board : test_cases)
    {
        sheep g(input_board);
        assert_no_split(&g);
    }
}

} // namespace

void split_test_sheep_all()
{
    split_test_main();
    split_test_nosplit();
}
