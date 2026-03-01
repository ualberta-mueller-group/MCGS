#pragma once

#include <array>

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

