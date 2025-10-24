#include "domineering_test.h"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <tuple>
#include "cgt_move.h"
#include "domineering.h"

using namespace std;

namespace {
////////////////////////////////////////////////// domineering utilities
////////////////////////////////////////////////// test helper logic
unordered_set<::move> coord_vec_to_move_set(const vector<int_pair>& coords)
{
    unordered_set<::move> moves;

    for (const int_pair& coord : coords)
    {
        const ::move m = encode_domineering_coord(coord);
        auto inserted = moves.insert(m);
        assert(inserted.second);
    }

    return moves;
}
////////////////////////////////////////////////// main test functions

void test_moves_main()
{
    typedef tuple<string, vector<int_pair>, vector<int_pair>> test_case_t;


    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            ".",
            {
            },
            {
            },
        },

        {
            "..",
            {
            },
            {
                {0, 0},
            },
        },

        {
            ".|.",
            {
                {0, 0},
            },
            {
            },
        },

        {
            "..|..",
            {
                {0, 0},
                {0, 1},
            },
            {
                {0, 0},
                {1, 0},
            },
        },
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& board = get<0>(test_case);
        const vector<int_pair>& exp_b_coords = get<1>(test_case);
        const vector<int_pair>& exp_w_coords = get<2>(test_case);

        const unordered_set<::move> exp_b_moves =
            coord_vec_to_move_set(exp_b_coords);

        const unordered_set<::move> exp_w_moves =
            coord_vec_to_move_set(exp_w_coords);
    }
}

void test_constructors()
{
}

} // namespace

void domineering_test_all()
{
    cout << "TODO: " << __FILE__ << endl;

    test_moves_main();
    test_constructors();
}
