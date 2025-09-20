#pragma once

#include <algorithm>
#include <cstddef>
#include <array>

#include "grid.h"
#include "game.h"
#include "grid_location.h"

////////////////////////////////////////////////// constants
/*
    Notation for these comments:
    r, c: row and column indices
    R, C: row count, column count (in grid shape)
*/

// clockwise rotations (degrees)
enum grid_hash_rotation
{
    GRID_HASH_ROTATION_0 = 0,
    GRID_HASH_ROTATION_90,
    GRID_HASH_ROTATION_180,
    GRID_HASH_ROTATION_270,
};

static constexpr std::array<grid_hash_rotation, 4> GRID_HASH_ROTATIONS
{
    GRID_HASH_ROTATION_0,
    GRID_HASH_ROTATION_90,
    GRID_HASH_ROTATION_180,
    GRID_HASH_ROTATION_270,
};

enum grid_hash_bit_enum
{
    GRID_HASH_BIT_ROW_INVERSE = (1 << 0), // r -> (R - 1) - r
    GRID_HASH_BIT_COL_INVERSE = (1 << 1), // c -> (C - 1) - c
    GRID_HASH_BIT_FLIP = (1 << 2), // transpose: R, C, r, c -> C, R, c, r
};

static constexpr std::array<unsigned int, 4> _GRID_HASH_ROTATION_BITS {
    0, // 0
    GRID_HASH_BIT_ROW_INVERSE | GRID_HASH_BIT_FLIP, // 90
    GRID_HASH_BIT_ROW_INVERSE | GRID_HASH_BIT_COL_INVERSE, // 180
    GRID_HASH_BIT_COL_INVERSE | GRID_HASH_BIT_FLIP, // 270
};

inline unsigned int get_grid_hash_rotation_bits(grid_hash_rotation rot)
{
    assert(0 <= rot && rot < 4);
    return _GRID_HASH_ROTATION_BITS[rot];
}

////////////////////////////////////////////////// class grid_hash
class grid_hash
{
public:
    void reset(const int_pair& grid_shape);

    hash_t get_value() const;

    template <class T>
    void toggle_value(int r, int c, const T& color);

    void toggle_type(game_type_t type);

private:
    int_pair _get_rotated_coords(int r, int c, grid_hash_rotation rot) const;
    int_pair _get_rotated_shape(grid_hash_rotation rot) const;

    static bool _compare(const local_hash& hash1, const local_hash& hash2);

    static constexpr unsigned int _N_HASHES = 8;

    int_pair _grid_shape;
    std::array<local_hash, _N_HASHES> _hashes;
};

/*
    0
    R C -> R C
    r c -> r c

    90
    R C -> C R
    r c -> c, (R - 1) - r

    180
    R C -> R C
    r c -> (R - 1) - r, (C - 1) - c

    270
    R C -> C R
    r c -> (C - 1) - c, r

    flip row_inverse  col_inverse
    000 -> 0
    110 -> 90
    011 -> 180
    101 -> 270
*/


////////////////////////////////////////////////// grid_hash methods
// TODO don't inline these all...

inline void grid_hash::reset(const int_pair& grid_shape)
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

inline hash_t grid_hash::get_value() const
{
    const local_hash* result = std::min_element(_hashes.begin(), _hashes.end(), _compare);
    return result->get_value();
}

template <class T>
void grid_hash::toggle_value(int r, int c, const T& color)
{
    const size_t N_ROTATIONS = GRID_HASH_ROTATIONS.size();

    for (size_t rot_idx = 0; rot_idx < N_ROTATIONS; rot_idx++)
    {
        const grid_hash_rotation rot = GRID_HASH_ROTATIONS[rot_idx];

        const int_pair coords = _get_rotated_coords(r, c, rot);
        const int_pair shape = _get_rotated_shape(rot);

        const int_pair coords_transpose(coords.second, coords.first);
        const int_pair shape_transpose(shape.second, shape.first);

        const size_t i = rot_idx * 2;

        local_hash& hash = _hashes[i];
        local_hash& hash_transpose = _hashes[i + 1];

        const int point = grid_location::coord_to_point(coords, shape);
        const int point_transpose =
            grid_location::coord_to_point(coords_transpose, shape_transpose);

        hash.toggle_value(2 + point, color);
        hash_transpose.toggle_value(2 + point_transpose, color);
    }
}

inline void grid_hash::toggle_type(game_type_t type)
{
    for (local_hash& hash : _hashes)
        hash.toggle_type(type);
}

inline int_pair grid_hash::_get_rotated_coords(int r, int c, grid_hash_rotation rot) const
{
    const unsigned int bits = get_grid_hash_rotation_bits(rot);

    if (bits & GRID_HASH_BIT_ROW_INVERSE)
        r = (_grid_shape.first - 1) - r;

    if (bits & GRID_HASH_BIT_COL_INVERSE)
        c = (_grid_shape.second - 1) - c;

    if (bits & GRID_HASH_BIT_FLIP)
        return {c, r};

    return {r, c};
}

inline int_pair grid_hash::_get_rotated_shape(grid_hash_rotation rot) const
{
    const unsigned int bits = get_grid_hash_rotation_bits(rot);

    if (bits & GRID_HASH_BIT_FLIP)
        return {_grid_shape.second, _grid_shape.first};

    return _grid_shape;
}

inline bool grid_hash::_compare(const local_hash& hash1, const local_hash& hash2)
{
    return hash1.get_value() < hash2.get_value();
}

//////////////////////////////////////////////////
void test_grid_hash_stuff();
