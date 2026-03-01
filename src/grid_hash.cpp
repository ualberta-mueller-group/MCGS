#include "grid_hash.h"

#include <cassert>

#include "grid.h"
#include "utilities.h"
#include "hashing.h"

using namespace std;

////////////////////////////////////////////////// helper functions

////////////////////////////////////////////////// grid_hash methods
void grid_hash::reset(const int_pair& grid_shape)
{
    _is_dirty = true;
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

        const bool active1 = bit_is_1(_grid_hash_mask, idx1);
        const bool active2 = bit_is_1(_grid_hash_mask, idx2);

        const grid_hash_orientation ori = GRID_HASH_ORIENTATIONS[idx1];
        const int_pair shape = _get_transformed_shape_no_t(ori);

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

void grid_hash::_compute_value() const
{
    assert(_is_dirty);

    hash_t min_val = std::numeric_limits<hash_t>::max();
    unsigned int min_idx = std::numeric_limits<unsigned int>::max();

    for (unsigned int i = 0; i < _N_HASHES; i++)
    {
        if (bit_is_1(_grid_hash_mask, i))
        {
            const hash_t h = _hashes[i].get_value();

            if (h < min_val)
            {
                min_val = h;
                min_idx = i;
            }
        }
    }

    assert(min_idx != std::numeric_limits<unsigned int>::max());

    _is_dirty = false;
    _selected_orientation = static_cast<grid_hash_orientation>(min_idx);
}

