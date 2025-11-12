#include "grid_hash.h"

#include <iostream>
#include <vector>
#include <cassert>

#include "grid.h"
#include "utilities.h"
#include "hashing.h"

using namespace std;

////////////////////////////////////////////////// helper functions
namespace {
// TODO why unused?
[[maybe_unused]] void print_grid(const int_pair& shape, const vector<int>& vec)
{
    for (int r = 0; r < shape.first; r++)
    {
        for (int c = 0; c < shape.second; c++)
        {
            cout << vec[r * shape.second + c] << ' ';
        }
        cout << endl;
    }
}

} // namespace

////////////////////////////////////////////////// grid_hash methods
void grid_hash::reset(const int_pair& grid_shape)
{
    assert(grid_shape.first >= 0 && grid_shape.second >= 0);

    _grid_shape = grid_shape;

    static_assert(_N_HASHES == GRID_HASH_ORIENTATIONS.size());

    for (unsigned int idx_no_t = 0; idx_no_t < GRID_HASH_ORIENTATIONS.size();
         idx_no_t += 2)
    {
        // Orientation/_hashes index for N degree rotation, N degree + transpose
        const unsigned int idx1 = idx_no_t;
        const unsigned int idx2 = idx1 + 1;

        assert(!bit_is_1(idx1, 0));

        const bool active1 = bit_is_1(_active_orientation_mask, idx1);
        const bool active2 = bit_is_1(_active_orientation_mask, idx2);

        const grid_hash_orientation ori = GRID_HASH_ORIENTATIONS[idx1];
        const int_pair shape = _get_transformed_shape(ori);

        if (active1)
        {
            local_hash& hash1 = _hashes[idx1];
            hash1.reset();
            hash1.toggle_value(0, shape.first);
            hash1.toggle_value(1, shape.second);
        }

        if (active2)
        {
            local_hash& hash2 = _hashes[idx2];
            hash2.reset();
            hash2.toggle_value(0, shape.second);
            hash2.toggle_value(1, shape.first);
        }
    }
}

//////////////////////////////////////////////////
void test_grid_hash_stuff()
{
}
