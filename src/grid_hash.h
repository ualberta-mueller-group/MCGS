#pragma once
/*
    grid_hash class

    For computing the same local hash between identical grids (considering
    rotation/transpose symmetry).

    Manages 8 local_hash objects and computes their values for each 90 degree
    rotation and transpose combination. The value returned by grid_hash is the
    minimum of these.

    Orientations are enabled by passing a bit mask to the grid_hash constructor.

    TODO check inlining in this file
*/

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

#include <array>
#include <cassert>
#include <limits>
#include <type_traits>
#include <vector>

#include "grid.h"
#include "game.h"
#include "grid_hash_orientation.h"
#include "grid_location.h"
#include "int_pair.h"
#include "type_table.h"
#include "utilities.h"

// If defined: supported grid games use grid_hash class
#define USE_GRID_HASH


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

static constexpr unsigned int GRID_HASH_ACTIVE_MASK_ROTATION90 =
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_0) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_0T) |   //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_90) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90T) |  //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_180) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180T) | //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_270); //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270T);  //

static constexpr unsigned int GRID_HASH_ACTIVE_MASK_MIRROR_VERT =
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_0) |    //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_0T) |   //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90) |   //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_90T);  // vert flip
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180) |  // vert and horiz flip
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180T) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270) |  //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270T);  // horiz flip

static constexpr unsigned int GRID_HASH_ACTIVE_MASK_MIRROR_HORIZ =
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_0) |    //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_0T) |   //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90) |   //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90T) | // vert flip
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180) |  // vert and horiz flip
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180T) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270) |  //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_270T);  // horiz flip

static constexpr unsigned int GRID_HASH_ACTIVE_MASK_ROTATION180 =
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_0) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_0T) |   //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90T) |  //
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_180); //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180T) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270); //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270T);  //

static constexpr unsigned int GRID_HASH_ACTIVE_MASK_IDENTITY =
    set_bit<unsigned int>(GRID_HASH_ORIENTATION_0);     //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_0T) |   //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90) |   //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_90T) |  //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180) |  //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_180T) | //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270) |  //
    //set_bit<unsigned int>(GRID_HASH_ORIENTATION_270T);  //


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
    Bit masks of `orientation_op`s for each 90 degree rotation (excluding
    transposes)
*/
static constexpr std::array<unsigned int, 4> ORIENTATION_OPS_NO_T {
    0, // 0
    ORIENTATION_OP_ROW_INV | ORIENTATION_OP_SWAP, // 90
    ORIENTATION_OP_ROW_INV | ORIENTATION_OP_COL_INV, // 180
    ORIENTATION_OP_COL_INV | ORIENTATION_OP_SWAP, // 270
};

// Ignores "T" in grid_hash_orientation
inline unsigned int get_op_mask_no_t(grid_hash_orientation ori)
{
    assert(0 <= ori && //
           ((std::make_unsigned_t<grid_hash_orientation>) ori) <
               GRID_HASH_ORIENTATIONS.size() //
    );

    return ORIENTATION_OPS_NO_T[ori / 2];
}

static constexpr std::array<grid_hash_orientation, 4>
    INVERSE_GRID_HASH_ORIENTATIONS_NO_T 
{
    GRID_HASH_ORIENTATION_0, // 0 -> 0
    GRID_HASH_ORIENTATION_270, // 90 -> 270
    GRID_HASH_ORIENTATION_180, // 180 -> 180
    GRID_HASH_ORIENTATION_90, // 270 -> 90
};

inline grid_hash_orientation get_inverse_orientation_no_t(grid_hash_orientation ori)
{
    //assert(!bit_is_1(ori, 0)); // Shouldn't have T suffix
    const unsigned int idx = ori / 2;

    assert(0 <= idx && idx < INVERSE_GRID_HASH_ORIENTATIONS_NO_T.size());
    return INVERSE_GRID_HASH_ORIENTATIONS_NO_T[idx];
}

} // namespace __grid_hash_impl

////////////////////////////////////////////////// class grid_hash
class grid_hash
{
public:
    grid_hash(unsigned int grid_hash_mask);

    void reset(const int_pair& grid_shape);

    hash_t get_value() const;
    grid_hash_orientation get_orientation() const;

    template <class T>
    void toggle_value(int r, int c, const T& color);

    template <class T>
    void toggle_value(const int_pair& coord, const T& color);

    void toggle_type(game_type_t type);

    void init_from_grid(const grid& g);

    template <class T>
    void init_from_board_and_type(const std::vector<T>& board,
                                  const int_pair& shape, game_type_t type);

    // All 4 of these properly handle "T" suffix for orientation
    static int_pair get_transformed_coords(const int_pair& shape,
                                           const int_pair& coords,
                                           grid_hash_orientation ori);

    static int_pair get_transformed_shape(const int_pair& shape,
                                          grid_hash_orientation ori);

    static int_pair get_inverse_transformed_coords(int_pair shape,
                                                   int_pair coords,
                                                   grid_hash_orientation ori);

    static int_pair get_inverse_transformed_shape(int_pair shape,
                                                  grid_hash_orientation ori);

private:
    void _compute_value() const;

    // Both of these ignore "T" suffix
    int_pair _get_transformed_coords_no_t(int r, int c,
                                          grid_hash_orientation ori) const;

    int_pair _get_transformed_shape_no_t(grid_hash_orientation ori) const;

    static constexpr unsigned int _N_HASHES = GRID_HASH_ORIENTATIONS.size();

    const unsigned int _grid_hash_mask;

    mutable bool _is_dirty;
    mutable grid_hash_orientation _selected_orientation;

    int_pair _grid_shape;
    std::array<local_hash, _N_HASHES> _hashes;
};


////////////////////////////////////////////////// grid_hash methods
inline grid_hash::grid_hash(unsigned int grid_hash_mask)
    : _grid_hash_mask(grid_hash_mask), _is_dirty(true)
{
    assert(bit_is_1(_grid_hash_mask, GRID_HASH_ORIENTATION_0) && //
           0 == (_grid_hash_mask &
                 ~get_bit_mask_lower<unsigned int>(_N_HASHES)) //
    );
}

inline hash_t grid_hash::get_value() const
{
    if (_is_dirty)
        _compute_value();

    assert(!_is_dirty);
    return _hashes[_selected_orientation].get_value();
}

inline grid_hash_orientation grid_hash::get_orientation() const
{
    if (_is_dirty)
        _compute_value();

    assert(!_is_dirty);
    return _selected_orientation;
}

template <class T>
void grid_hash::toggle_value(int r, int c, const T& color)
{
    static_assert(_N_HASHES == GRID_HASH_ORIENTATIONS.size());
    _is_dirty = true;

    for (unsigned int idx_no_t = 0; idx_no_t < GRID_HASH_ORIENTATIONS.size();
         idx_no_t += 2)
    {
        // Orientation/_hashes index for N degree rotation, N degree + transpose
        const unsigned int idx1 = idx_no_t;
        const unsigned int idx2 = idx1 + 1;

        assert(!bit_is_1(idx1, 0));

        const bool active1 = bit_is_1(_grid_hash_mask, idx1);
        const bool active2 = bit_is_1(_grid_hash_mask, idx2);

        if (!(active1 || active2))
            continue;

        const grid_hash_orientation ori = GRID_HASH_ORIENTATIONS[idx1];

        const int_pair coords = _get_transformed_coords_no_t(r, c, ori);
        const int_pair shape = _get_transformed_shape_no_t(ori);

        const int_pair coords_transpose(coords.second, coords.first);
        const int_pair shape_transpose(shape.second, shape.first);

        if (active1)
        {
            local_hash& hash1 = _hashes[idx1];
            const int point1 = grid_location::coord_to_point(coords, shape);
            hash1.toggle_value<T>(2 + point1, color);
        }

        if (active2)
        {
            local_hash& hash2 = _hashes[idx2];
            const int point2 = grid_location::coord_to_point(coords_transpose,
                                                             shape_transpose);
            hash2.toggle_value<T>(2 + point2, color);
        }
    }
}

template <class T>
inline void grid_hash::toggle_value(const int_pair& coord, const T& color)
{
    _is_dirty = true;
    toggle_value<T>(coord.first, coord.second, color);
}

inline void grid_hash::toggle_type(game_type_t type)
{
    _is_dirty = true;
    for (local_hash& hash : _hashes)
        hash.toggle_type(type);
}

template <class T>
void grid_hash::init_from_board_and_type(const std::vector<T>& board,
                                         const int_pair& shape,
                                         game_type_t type)
{
    _is_dirty = true;
    reset(shape);
    toggle_type(type);

    int pos = 0;
    for (int r = 0; r < shape.first; r++)
    {
        for (int c = 0; c < shape.second; c++)
            toggle_value<T>(r, c, board[pos + c]);

        pos += shape.second;
    }
}

inline int_pair grid_hash::get_transformed_coords(const int_pair& shape,
                                                  const int_pair& coords,
                                                 grid_hash_orientation ori)
{
    int r = coords.first;
    int c = coords.second;

    // op mask (not considering "T" suffix)
    const unsigned int op_mask_no_t = __grid_hash_impl::get_op_mask_no_t(ori);

    // op mask indicates swap? 
    bool do_swap = (op_mask_no_t & __grid_hash_impl::ORIENTATION_OP_SWAP);
    do_swap ^= (bit_is_1(ori, 0)); // consider "T" suffix

    if (op_mask_no_t & __grid_hash_impl::ORIENTATION_OP_ROW_INV)
        r = (shape.first - 1) - r;

    if (op_mask_no_t & __grid_hash_impl::ORIENTATION_OP_COL_INV)
        c = (shape.second - 1) - c;

    int_pair result(r, c);
    if (do_swap)
        result = transpose_int_pair(result);

    return result;
}

inline int_pair grid_hash::get_transformed_shape(const int_pair& shape,
                                                 grid_hash_orientation ori)
{
    // op mask (not considering "T" suffix)
    const unsigned int op_mask_no_t = __grid_hash_impl::get_op_mask_no_t(ori);

    // op mask indicates swap? 
    bool do_swap = ((op_mask_no_t & __grid_hash_impl::ORIENTATION_OP_SWAP) != 0);
    do_swap ^= (bit_is_1(ori, 0)); // consider "T" suffix

    if (do_swap)
        return transpose_int_pair(shape);

    return shape;
}

inline int_pair grid_hash::get_inverse_transformed_coords(
    int_pair shape, int_pair coords, grid_hash_orientation ori)
{
    // op mask (not considering "T" suffix)
    const unsigned int op_mask_no_t = __grid_hash_impl::get_op_mask_no_t(ori);

    // op mask indicates swap? 
    bool do_swap = (op_mask_no_t & __grid_hash_impl::ORIENTATION_OP_SWAP);
    do_swap ^= (bit_is_1(ori, 0)); // consider "T" suffix

    if (do_swap)
        coords = transpose_int_pair(coords);

    int r = coords.first;
    int c = coords.second;

    if (op_mask_no_t & __grid_hash_impl::ORIENTATION_OP_ROW_INV)
        r = (shape.first - 1) - r;

    if (op_mask_no_t & __grid_hash_impl::ORIENTATION_OP_COL_INV)
        c = (shape.second - 1) - c;

    int_pair result(r, c);

    return result;
}


//inline int_pair grid_hash::get_inverse_transformed_coords(
//    int_pair shape, int_pair coords, grid_hash_orientation ori)
//{
//    // op mask (not considering "T" suffix)
//    const unsigned int op_mask_no_t = __grid_hash_impl::get_op_mask_no_t(ori);
//
//    // op mask indicates swap? 
//    bool do_swap = (op_mask_no_t & __grid_hash_impl::ORIENTATION_OP_SWAP);
//    do_swap ^= (bit_is_1(ori, 0)); // consider "T" suffix
//
//    if (do_swap)
//        coords = transpose_int_pair(coords);
//
//    int r = coords.first;
//    int c = coords.second;
//
//    if (op_mask_no_t & __grid_hash_impl::ORIENTATION_OP_ROW_INV)
//        r = (shape.first - 1) - r;
//
//    if (op_mask_no_t & __grid_hash_impl::ORIENTATION_OP_COL_INV)
//        c = (shape.second - 1) - c;
//
//    int_pair result(r, c);
//
//    return result;
//}

inline int_pair grid_hash::get_inverse_transformed_shape(
    int_pair shape, grid_hash_orientation ori)
{
    // First handle T suffix
    if (bit_is_1(ori, 0))
    {
        shape = transpose_int_pair(shape);
        // Remove suffix
        ori = static_cast<grid_hash_orientation>(ori - 1);
    }

    const grid_hash_orientation ori_inverse =
        __grid_hash_impl::get_inverse_orientation_no_t(ori);

    return get_transformed_shape(shape, ori_inverse);
}

inline int_pair grid_hash::_get_transformed_coords_no_t(
    int r, int c, grid_hash_orientation ori) const
{
    // Must be even number (no T suffix)
    assert(!bit_is_1(ori, 0));

    return get_transformed_coords(_grid_shape, {r, c}, ori);
}

inline int_pair grid_hash::_get_transformed_shape_no_t(
    grid_hash_orientation ori) const
{
    // Must be even number (no T suffix)
    assert(!bit_is_1(ori, 0));

    return get_transformed_shape(_grid_shape, ori);
}

inline void grid_hash::init_from_grid(const grid& g)
{
    _is_dirty = true;
    const int_pair& shape = g.shape();
    const game_type_t type = g.game_type();

    init_from_board_and_type(g.board_const(), shape, type);
}

////////////////////////////////////////////////// type_table stuff...
template <class Game_T>
unsigned int grid_hash_mask()
{
    static_assert(std::is_base_of_v<game, Game_T> &&
                  !std::is_abstract_v<Game_T>);

    return type_table<Game_T>()->grid_hash_mask();
}

template <class Game_T>
void set_grid_hash_mask(unsigned int mask)
{
    static_assert(std::is_base_of_v<game, Game_T> &&
                  !std::is_abstract_v<Game_T>);

    type_table<Game_T>()->set_grid_hash_mask(mask);
}

