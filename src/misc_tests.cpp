#include "misc_tests.h"

#include "cgt_basics.h"
#include "grid.h"
#include "grid_hash.h"
#include "utilities.h"
#include "n_bit_int.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <iostream>

typedef std::pair<std::vector<int>, int_pair> grid_pair_t;

using namespace std;

namespace {

void print_grid(const std::vector<int>& board, const int_pair& shape)
{
    int idx = 0;
    for (int r = 0; r < shape.first; r++)
    {
        for (int c = 0; c < shape.second; c++)
        {
            cout << color_to_char(board[idx]);

            idx++;
        }

        cout << endl;
    }
    cout << endl;
}


// TODO static asserts?

grid_pair_t rotate_grid_pair(const grid_pair_t& grid_pair)
{
    const vector<int>& vec = grid_pair.first;
    const int_pair& shape = grid_pair.second;

    const int_pair shape_rotated(shape.second, shape.first);

    return {grid::rotate_90_board(vec, shape), shape_rotated};
}

grid_pair_t transpose_grid_pair(const grid_pair_t& grid_pair)
{
    const vector<int>& vec = grid_pair.first;
    const int_pair& shape = grid_pair.second;

    const int_pair shape_rotated(shape.second, shape.first);

    return {grid::transpose_board(vec, shape), shape_rotated};
}

hash_t get_grid_pair_hash(const grid_pair_t& grid_pair, grid_hash& gh)
{
    gh.init_from_board_and_type(grid_pair.first, grid_pair.second, 0);
    return gh.get_value();
}


#warning ADD STATIC ASSERTS TO MAKE THESE SAFE...

void test_grid_hash_impl(unsigned int mask, const grid_pair_t& grid0)
{
    // 0 and 0t
    grid_pair_t grid0t = transpose_grid_pair(grid0);

    // 90 and 90t
    grid_pair_t grid90 = rotate_grid_pair(grid0);
    grid_pair_t grid90t = transpose_grid_pair(grid90);

    // 180 and 180t
    grid_pair_t grid180 = rotate_grid_pair(grid90);
    grid_pair_t grid180t = transpose_grid_pair(grid180);

    // 270 and 270t
    grid_pair_t grid270 = rotate_grid_pair(grid180);
    grid_pair_t grid270t = transpose_grid_pair(grid270);

    grid_hash gh(mask);

    const hash_t hash0 = get_grid_pair_hash(grid0, gh);
    const hash_t hash90 = get_grid_pair_hash(grid90, gh);
    const hash_t hash180 = get_grid_pair_hash(grid180, gh);
    const hash_t hash270 = get_grid_pair_hash(grid270, gh);

    const hash_t hash0t = get_grid_pair_hash(grid0t, gh);
    const hash_t hash90t = get_grid_pair_hash(grid90t, gh);
    const hash_t hash180t = get_grid_pair_hash(grid180t, gh);
    const hash_t hash270t = get_grid_pair_hash(grid270t, gh);

    vector<hash_t> hash_vec(8, 0);

    auto add_to_hash_vec = [&](const hash_t hash, int idx) -> void
    {
        assert(hash_vec[idx] == 0);
        hash_vec[idx] = hash;
    };

    add_to_hash_vec(hash0, GRID_HASH_ORIENTATION_0);
    add_to_hash_vec(hash90, GRID_HASH_ORIENTATION_90);
    add_to_hash_vec(hash180, GRID_HASH_ORIENTATION_180);
    add_to_hash_vec(hash270, GRID_HASH_ORIENTATION_270);
    add_to_hash_vec(hash0t, GRID_HASH_ORIENTATION_0T);
    add_to_hash_vec(hash90t, GRID_HASH_ORIENTATION_90T);
    add_to_hash_vec(hash180t, GRID_HASH_ORIENTATION_180T);
    add_to_hash_vec(hash270t, GRID_HASH_ORIENTATION_270T);

    for (int bit_idx = 0; bit_idx < 8; bit_idx++)
    {
        const hash_t hash = hash_vec[bit_idx];

        if (mask & (1 << bit_idx))
        {
            assert(hash == hash0);
        }
    }
}



#warning CLEAN UP COMMENTS HERE AND IN GRID_HASH.H
void test_grid_hash()
{

    typedef tuple<string> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        "XO.|.X.",
        ".X|XO|..",
        ".X.|.OX",
        "..|OX|X.",
        "X.|OX|..",
        ".X.|XO.",
        "..|XO|.X",
        ".OX|.X.",
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& board_string = get<0>(test_case);

        const grid_pair_t grid_pair = string_to_grid(board_string);

        /*
           TODO NOTE:
           Not all orientation masks are legal. There's some "closure"
           condition we should check for in grid_hash.

           An orientation mask denotes an equivalence class for boards.
           All boards in the equivalence class must be reached in one "step".
           This means the mask containing only 0 and 90 is illegal:
           - The inverse of 90 (270) is missing
           - 90 implies 180, but this takes a 2nd step to reach...
        */

        /*
           These are all of the masks which pass all test cases:

            1: 00000000000000000000000000000001
            3: 00000000000000000000000000000011
            9: 00000000000000000000000000001001
            17: 00000000000000000000000000010001
            33: 00000000000000000000000000100001
            51: 00000000000000000000000000110011
            85: 00000000000000000000000001010101
            129: 00000000000000000000000010000001
            153: 00000000000000000000000010011001
            255: 00000000000000000000000011111111
        */

        test_grid_hash_impl(GRID_HASH_ACTIVE_MASK_IDENTITY, grid_pair);

        test_grid_hash_impl(GRID_HASH_ACTIVE_MASK_ROTATION90, grid_pair);
        test_grid_hash_impl(GRID_HASH_ACTIVE_MASK_ROTATION180, grid_pair);

        test_grid_hash_impl(GRID_HASH_ACTIVE_MASK_MIRROR_VERT, grid_pair);
        test_grid_hash_impl(GRID_HASH_ACTIVE_MASK_MIRROR_HORIZ, grid_pair);
        test_grid_hash_impl(GRID_HASH_ACTIVE_MASK_MIRRORS, grid_pair);

        test_grid_hash_impl(GRID_HASH_ACTIVE_MASK_ALL, grid_pair);
    }
}

} // namespace



void misc_tests_all()
{

    test_grid_hash();


}
