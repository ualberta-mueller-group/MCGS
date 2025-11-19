#pragma once
/*
    grid_hash class

    For computing the same local hash between identical grids (considering
    rotation/transpose symmetry).

    Manages 8 local_hash objects and computes their values for each 90 degree
    rotation and transpose combination. The value returned by grid_hash is the
    minimum of these.

    Orientations are enabled by passing a bit mask to the grid_hash constructor.
*/

#include <array>
#include <cassert>
#include <limits>
#include <type_traits>

#include "grid.h"
#include "game.h"
#include "grid_location.h"
#include "utilities.h"

#define USE_GRID_HASH

////////////////////////////////////////////////// grid_hash_orientation

/*
    Enum of the 8 rotation/transpose orientations for grids

    Numeric suffix is the clockwise rotation (in degrees). "T" indicates
    transpose, computed AFTER the rotation.
*/
enum grid_hash_orientation
{
    GRID_HASH_ORIENTATION_0 = 0,
    GRID_HASH_ORIENTATION_0T,
    GRID_HASH_ORIENTATION_90,
    GRID_HASH_ORIENTATION_90T,
    GRID_HASH_ORIENTATION_180,
    GRID_HASH_ORIENTATION_180T,
    GRID_HASH_ORIENTATION_270,
    GRID_HASH_ORIENTATION_270T,
};

// Array of the 8 grid_hash_orientations
static constexpr std::array<grid_hash_orientation, 8> GRID_HASH_ORIENTATIONS
{
    GRID_HASH_ORIENTATION_0,
    GRID_HASH_ORIENTATION_0T,
    GRID_HASH_ORIENTATION_90,
    GRID_HASH_ORIENTATION_90T,
    GRID_HASH_ORIENTATION_180,
    GRID_HASH_ORIENTATION_180T,
    GRID_HASH_ORIENTATION_270,
    GRID_HASH_ORIENTATION_270T,
};

//////////////////////////////////////// Common active orientation bit masks
// TODO static_assert that these are 8 and 4 bits, and within the lower 8 bits

/*
    Bit mask indicating that all 8 orientations should be active.

    i.e. clobber, nogo, amazons
*/
static constexpr unsigned int GRID_HASH_ACTIVE_MASK_ALL =
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_0) |    //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_0T) |   //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_90) |   //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_90T) |  //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_180) |  //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_180T) | //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_270) |  //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_270T);  //

/*
    Bit mask indicating that only orientations achievable by mirroring the grid
    (vertically, horizontally, or both) should be active.

    i.e. domineering, fission
*/
static constexpr unsigned int GRID_HASH_ACTIVE_MASK_MIRRORS =
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_0) |    //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_0T) |   //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90) |   //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_90T) |  // vert flip
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_180) |  // vert and horiz flip
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180T) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270) |  //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_270T);  // horiz flip

////////////////////////////////////////////////// implementation details

// NOLINTNEXTLINE(readability-identifier-naming)
namespace __grid_hash_impl {

/*
    1 bit for each possible operation used to transform grid coordinates given
    a target orientation
*/
enum orientation_op
{
    ORIENTATION_OP_ROW_INV = (1 << 0), // r -> (R - 1) - r
    ORIENTATION_OP_COL_INV = (1 << 1), // c -> (C - 1) - c
    ORIENTATION_OP_SWAP = (1 << 2), // transpose: R, C, r, c -> C, R, c, r
};

/*
    Bit masks of orientation_ops for each 90 degree rotation (excluding
    transposes)
*/
static constexpr std::array<unsigned int, 4> ORIENTATION_OPS_NO_T {
    0, // 0
    ORIENTATION_OP_ROW_INV | ORIENTATION_OP_SWAP, // 90
    ORIENTATION_OP_ROW_INV | ORIENTATION_OP_COL_INV, // 180
    ORIENTATION_OP_COL_INV | ORIENTATION_OP_SWAP, // 270
};

inline unsigned int get_op_mask(grid_hash_orientation ori)
{
    assert(!bit_is_1(ori, 0) &&                // Odd numbers only
           0 <= ori &&                         //
           ((std::make_unsigned_t<grid_hash_orientation>) ori) < GRID_HASH_ORIENTATIONS.size() //
    );

    return ORIENTATION_OPS_NO_T[ori / 2];
}

} // namespace __grid_hash_impl

////////////////////////////////////////////////// class grid_hash
class grid_hash
{
public:
    grid_hash(unsigned int active_orientation_mask);

    void reset(const int_pair& grid_shape);

    hash_t get_value() const;

    template <class T>
    void toggle_value(int r, int c, const T& color);

    template <class T>
    void toggle_value(const int_pair& coord, const T& color);

    void toggle_type(game_type_t type);

    void init_from_grid(const grid& g);

private:
    int_pair _get_transformed_coords(int r, int c,
                                     grid_hash_orientation ori) const;

    int_pair _get_transformed_shape(grid_hash_orientation ori) const;

    static constexpr unsigned int _N_HASHES = GRID_HASH_ORIENTATIONS.size();

    const unsigned int _active_orientation_mask;

    int_pair _grid_shape;
    std::array<local_hash, _N_HASHES> _hashes;
};


////////////////////////////////////////////////// grid_hash methods


inline grid_hash::grid_hash(unsigned int active_orientation_mask)
    : _active_orientation_mask(active_orientation_mask)
{
    assert(bit_is_1(_active_orientation_mask, GRID_HASH_ORIENTATION_0) && //
           0 == (_active_orientation_mask &
                 ~get_bit_mask_lower<unsigned int>(_N_HASHES)) //
    );
}

inline hash_t grid_hash::get_value() const
{
    hash_t min_val = std::numeric_limits<hash_t>::max();

    for (unsigned int i = 0; i < _N_HASHES; i++)
    {
        if (bit_is_1(_active_orientation_mask, i))
        {
            const hash_t h = _hashes[i].get_value();

            if (h < min_val)
                min_val = h;
        }
    }

    return min_val;
}

template <class T>
void grid_hash::toggle_value(int r, int c, const T& color)
{
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

        if (!(active1 || active2))
            continue;

        const grid_hash_orientation ori = GRID_HASH_ORIENTATIONS[idx1];

        const int_pair coords = _get_transformed_coords(r, c, ori);
        const int_pair shape = _get_transformed_shape(ori);

        const int_pair coords_transpose(coords.second, coords.first);
        const int_pair shape_transpose(shape.second, shape.first);

        if (active1)
        {
            local_hash& hash1 = _hashes[idx1];
            const int point1 = grid_location::coord_to_point(coords, shape);
            hash1.toggle_value(2 + point1, color);
        }

        if (active2)
        {
            local_hash& hash2 = _hashes[idx2];
            const int point2 = grid_location::coord_to_point(coords_transpose,
                                                             shape_transpose);
            hash2.toggle_value(2 + point2, color);
        }
    }
}

template <class T>
inline void grid_hash::toggle_value(const int_pair& coord, const T& color)
{
    toggle_value(coord.first, coord.second, color);
}

inline void grid_hash::toggle_type(game_type_t type)
{
    for (local_hash& hash : _hashes)
        hash.toggle_type(type);
}

inline int_pair grid_hash::_get_transformed_coords(
    int r, int c, grid_hash_orientation ori) const
{
    const unsigned int bits = __grid_hash_impl::get_op_mask(ori);

    if (bits & __grid_hash_impl::ORIENTATION_OP_ROW_INV)
        r = (_grid_shape.first - 1) - r;

    if (bits & __grid_hash_impl::ORIENTATION_OP_COL_INV)
        c = (_grid_shape.second - 1) - c;

    if (bits & __grid_hash_impl::ORIENTATION_OP_SWAP)
        return {c, r};

    return {r, c};
}

inline int_pair grid_hash::_get_transformed_shape(
    grid_hash_orientation ori) const
{
    const unsigned int bits = __grid_hash_impl::get_op_mask(ori);

    if (bits & __grid_hash_impl::ORIENTATION_OP_SWAP)
        return {_grid_shape.second, _grid_shape.first};

    return _grid_shape;
}



//////////////////////////////////////////////////
void test_grid_hash_stuff();
