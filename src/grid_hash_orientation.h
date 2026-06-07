#pragma once
#include <array>

#include "utilities.h"

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
inline static constexpr unsigned int GRID_HASH_ACTIVE_MASK_ALL =
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
inline static constexpr unsigned int GRID_HASH_ACTIVE_MASK_MIRRORS =
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_0) |    //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_0T) |   //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90) |   //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_90T) |  // vert flip
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_180) |  // vert and horiz flip
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180T) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270) |  //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_270T);  // horiz flip

inline static constexpr unsigned int GRID_HASH_ACTIVE_MASK_ROTATION90 =
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_0) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_0T) |   //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_90) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90T) |  //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_180) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180T) | //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_270); //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270T);  //

inline static constexpr unsigned int GRID_HASH_ACTIVE_MASK_MIRROR_VERT =
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_0) |    //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_0T) |   //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90) |   //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_90T);  // vert flip
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180) |  // vert and horiz flip
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180T) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270) |  //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270T);  // horiz flip

inline static constexpr unsigned int GRID_HASH_ACTIVE_MASK_MIRROR_HORIZ =
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_0) |    //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_0T) |   //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90) |   //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90T) | // vert flip
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180) |  // vert and horiz flip
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180T) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270) |  //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_270T);  // horiz flip

inline static constexpr unsigned int GRID_HASH_ACTIVE_MASK_ROTATION180 =
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_0) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_0T) |   //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90T) |  //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_180); //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180T) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270); //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270T);  //

inline static constexpr unsigned int GRID_HASH_ACTIVE_MASK_IDENTITY =
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_0);     //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_0T) |   //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90) |   //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90T) |  //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180) |  //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180T) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270) |  //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270T);  //

