#include "grid_mask_test.h"
#include "grid_hash.h"
#include "grid_mask.h"
#include <iostream>

using namespace std;

namespace {
////////////////////////////////////////////////// helpers
void generate_remaining_masks(vector<vector<bool>>& mask_sequence, grid_mask& gm)
{
    while (gm)
    {
        mask_sequence.emplace_back(gm.get_mask());
        ++gm;
    }
}

////////////////////////////////////////////////// main test functions

void test_generation()
{
    /*
       get<0>: grid dimension
       get<1>: expected mask sequence
    */
    typedef tuple<int_pair, vector<vector<bool>>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            int_pair(2, 2),
            {
                {0, 0, 0, 0},

                {1, 0, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 1, 0},
                {0, 0, 0, 1},

                {1, 1, 0, 0},
                {1, 0, 1, 0},
                {1, 0, 0, 1},
                {0, 1, 1, 0},
                {0, 1, 0, 1},
                {0, 0, 1, 1},

                {1, 1, 1, 0},
                {1, 1, 0, 1},
                {1, 0, 1, 1},
                {0, 1, 1, 1},

                {1, 1, 1, 1},
            }
        },

        {
            int_pair(1, 1),
            {
                {0},
                {1},
            },
        },

        {
            int_pair(2, 3),
            {
                {0, 0, 0, 0, 0, 0},

                {1, 0, 0, 0, 0, 0},
                {0, 1, 0, 0, 0, 0},
                {0, 0, 1, 0, 0, 0},
                {0, 0, 0, 1, 0, 0},
                {0, 0, 0, 0, 1, 0},
                {0, 0, 0, 0, 0, 1},

                {1, 1, 0, 0, 0, 0},
                {1, 0, 1, 0, 0, 0},
                {1, 0, 0, 1, 0, 0},
                {1, 0, 0, 0, 1, 0},
                {1, 0, 0, 0, 0, 1},
                {0, 1, 1, 0, 0, 0},
                {0, 1, 0, 1, 0, 0},
                {0, 1, 0, 0, 1, 0},
                {0, 1, 0, 0, 0, 1},
                {0, 0, 1, 1, 0, 0},
                {0, 0, 1, 0, 1, 0},
                {0, 0, 1, 0, 0, 1},
                {0, 0, 0, 1, 1, 0},
                {0, 0, 0, 1, 0, 1},
                {0, 0, 0, 0, 1, 1},

                {1, 1, 1, 0, 0, 0},
                {1, 1, 0, 1, 0, 0},
                {1, 1, 0, 0, 1, 0},
                {1, 1, 0, 0, 0, 1},
                {1, 0, 1, 1, 0, 0},
                {1, 0, 1, 0, 1, 0},
                {1, 0, 1, 0, 0, 1},
                {1, 0, 0, 1, 1, 0},
                {1, 0, 0, 1, 0, 1},
                {1, 0, 0, 0, 1, 1},
                {0, 1, 1, 1, 0, 0},
                {0, 1, 1, 0, 1, 0},
                {0, 1, 1, 0, 0, 1},
                {0, 1, 0, 1, 1, 0},
                {0, 1, 0, 1, 0, 1},
                {0, 1, 0, 0, 1, 1},
                {0, 0, 1, 1, 1, 0},
                {0, 0, 1, 1, 0, 1},
                {0, 0, 1, 0, 1, 1},
                {0, 0, 0, 1, 1, 1},

                {1, 1, 1, 1, 0, 0},
                {1, 1, 1, 0, 1, 0},
                {1, 1, 1, 0, 0, 1},
                {1, 1, 0, 1, 1, 0},
                {1, 1, 0, 1, 0, 1},
                {1, 1, 0, 0, 1, 1},
                {1, 0, 1, 1, 1, 0},
                {1, 0, 1, 1, 0, 1},
                {1, 0, 1, 0, 1, 1},
                {1, 0, 0, 1, 1, 1},
                {0, 1, 1, 1, 1, 0},
                {0, 1, 1, 1, 0, 1},
                {0, 1, 1, 0, 1, 1},
                {0, 1, 0, 1, 1, 1},
                {0, 0, 1, 1, 1, 1},

                {1, 1, 1, 1, 1, 0},
                {1, 1, 1, 1, 0, 1},
                {1, 1, 1, 0, 1, 1},
                {1, 1, 0, 1, 1, 1},
                {1, 0, 1, 1, 1, 1},
                {0, 1, 1, 1, 1, 1},

                {1, 1, 1, 1, 1, 1},
            },
        },

        {
            int_pair(0, 0),
            {
                {},
            },
        },
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const int_pair& dims = get<0>(test_case);
        const vector<vector<bool>>& exp_sequence = get<1>(test_case);

        grid_mask gm(GRID_HASH_ACTIVE_MASK_IDENTITY);
        assert(!gm);

        gm.set_shape(dims);
        vector<vector<bool>> sequence1;
        generate_remaining_masks(sequence1, gm);

        // Generate masks for transpose dimensions
        const int_pair dims_transpose(dims.second, dims.first);

        gm.set_shape(dims_transpose);
        vector<vector<bool>> sequence2;
        generate_remaining_masks(sequence2, gm);

        assert(sequence1 == exp_sequence);

        if (dims == dims_transpose)
            assert(sequence2.empty());
        else
            assert(sequence1 == sequence2);

        // Without first calling reset, these shouldn't produce values
        gm.set_shape(dims);
        assert(!gm);

        gm.set_shape(dims_transpose);
        assert(!gm);
    }
}

void test_symmetry1()
{
    grid_mask gm(GRID_HASH_ACTIVE_MASK_ALL);
    assert(!gm);

    gm.set_shape({2, 3});
    assert(gm);

    while (gm)
        ++gm;

    gm.set_shape({3, 2});
    assert(!gm);
}

void test_symmetry2()
{
    grid_mask gm(GRID_HASH_ACTIVE_MASK_ALL);
    assert(!gm);

    gm.set_shape({2, 2});
    assert(gm);

    vector<vector<bool>> sequence1;
    generate_remaining_masks(sequence1, gm);

    const vector<vector<bool>> exp_sequence =
    {
        {0, 0, 0, 0},

        {1, 0, 0, 0},

        {1, 1, 0, 0},
        //{1, 0, 1, 0},
        {1, 0, 0, 1},
        //{0, 1, 1, 0},
        //{0, 1, 0, 1},
        //{0, 0, 1, 1},

        {1, 1, 1, 0},
        //{1, 1, 0, 1},
        //{1, 0, 1, 1},
        //{0, 1, 1, 1},

        {1, 1, 1, 1},
    };

    assert(sequence1 == exp_sequence);

    gm.set_shape({2, 2});
    assert(!gm);

    // After reset, should produce the same result
    gm.reset();

    gm.set_shape({2, 2});
    assert(gm);

    vector<vector<bool>> sequence2;
    generate_remaining_masks(sequence2, gm);
    assert(sequence2 == exp_sequence);
}

void test_symmetry3()
{
    grid_mask gm(GRID_HASH_ACTIVE_MASK_MIRRORS);
    assert(!gm);

    gm.set_shape({2, 2});
    assert(gm);

    vector<vector<bool>> sequence1;
    generate_remaining_masks(sequence1, gm);

    const vector<vector<bool>> exp_sequence =
    {
        {0, 0, 0, 0},

        {1, 0, 0, 0},

        {1, 1, 0, 0},
        {1, 0, 1, 0},
        {1, 0, 0, 1},
        //{0, 1, 1, 0},
        //{0, 1, 0, 1},
        //{0, 0, 1, 1},

        {1, 1, 1, 0},
        //{1, 1, 0, 1},
        //{1, 0, 1, 1},
        //{0, 1, 1, 1},

        {1, 1, 1, 1},
    };

    assert(sequence1 == exp_sequence);

    gm.set_shape({2, 2});
    assert(!gm);

    // After reset, should produce the same result
    gm.reset();

    gm.set_shape({2, 2});
    assert(gm);

    vector<vector<bool>> sequence2;
    generate_remaining_masks(sequence2, gm);
    assert(sequence2 == exp_sequence);
}

} // namespace

//////////////////////////////////////////////////
void grid_mask_test_all()
{
    cout << __FILE__ << endl;
    test_generation();
    test_symmetry1();
    test_symmetry2();
    test_symmetry3();
}
