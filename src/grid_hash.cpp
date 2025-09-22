#include "grid_hash.h"

#include <iostream>
#include "grid.h"
#include "grid_location.h"

using namespace std;

////////////////////////////////////////////////// helper functions
namespace {
void print_grid(const int_pair& shape, const vector<int>& vec)
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

    constexpr size_t N_ROTATIONS = GRID_HASH_ROTATIONS.size();
    for (size_t rot_idx = 0; rot_idx < N_ROTATIONS; rot_idx++)
    {
        const grid_hash_rotation rot = GRID_HASH_ROTATIONS[rot_idx];
        const int_pair shape = _get_rotated_shape(rot);

        const size_t i = rot_idx * 2;
        local_hash& hash_normal = _hashes[i];
        local_hash& hash_transpose = _hashes[i + 1];

        hash_normal.reset();
        hash_normal.toggle_value(0, shape.first);
        hash_normal.toggle_value(1, shape.second);

        hash_transpose.reset();
        hash_transpose.toggle_value(0, shape.second);
        hash_transpose.toggle_value(1, shape.first);
    }
}

//////////////////////////////////////////////////
void test_grid_hash_stuff()
{
}
