/*
   TODO inline some of these functions...
*/
#include "grid_generator.h"

#include <iostream>
#include <cassert>

#include "clobber.h"
#include "clobber_1xn.h"
#include "grid_hash.h"
#include "throw_assert.h"
#include "utilities.h"
#include "cgt_basics.h"

using namespace std;


////////////////////////////////////////////////// i_grid_generator functions

bool i_grid_generator::increment_dims_standard(int_pair& dims,
                                                   const int_pair& max_dims,
                                                   bool init)
{
    assert(init || dims_le_max_standard(dims, max_dims));

    if (init)
    {
        dims = int_pair(0, 0);
        return dims_le_max_standard(dims, max_dims);
    }

    if (dims.first == 0 && dims.second == 0)
        dims = int_pair(1, 1);
    else
    {
        dims.second++;
        if (dims.second > max_dims.second)
        {
            dims.first++;
            dims.second = 1;
        }
    }

    return dims_le_max_standard(dims, max_dims);
}

bool i_grid_generator::dims_le_max_standard(const int_pair& dims,
                                                   const int_pair& max_dims)

{
    assert(dims.first >= 0 &&     //
           dims.second >= 0 &&    //
           max_dims.first >= 0 && //
           max_dims.second >= 0   //
    );

    return dims.first <= max_dims.first && //
           dims.second <= max_dims.second; //
}

bool i_grid_generator::increment_dims_transpose(int_pair& dims,
                                                const int_pair& max_dims,
                                                bool init)
{
    assert(init || dims_le_max_transpose(dims, max_dims));

    const int max_dim = max(max_dims.first, max_dims.second);
    const int min_dim = min(max_dims.first, max_dims.second);

    if (init)
    {
        dims = int_pair(0, 0);
        return dims_le_max_transpose(dims, max_dims);
    }

    if (dims.first == 0 && dims.second == 0)
        dims = int_pair(1, 1);
    else if (dims.first < dims.second)
        swap(dims.first, dims.second);
    else
    {
        swap(dims.first, dims.second);
        dims.second++;

        if (dims.second > max_dim)
        {
            const int next_val = min(dims.first, dims.second) + 1;
            dims.first = next_val;
            dims.second = next_val;
        }
    }

    const bool both_le_max = (dims.first <= max_dim) && //
                             (dims.second <= max_dim);  //

    const bool one_le_min = (dims.first <= min_dim) || //
                            (dims.second <= min_dim);  //

    return both_le_max && one_le_min;
}

bool i_grid_generator::dims_le_max_transpose(const int_pair& dims,
                                             const int_pair& max_dims)
{
    assert(dims.first >= 0 &&     //
           dims.second >= 0 &&    //
           max_dims.first >= 0 && //
           max_dims.second >= 0   //
    );

    const int max_dim = max(max_dims.first, max_dims.second);
    const int min_dim = min(max_dims.first, max_dims.second);

    const bool both_le_max = (dims.first <= max_dim) && //
                             (dims.second <= max_dim);  //

    const bool one_le_min = (dims.first <= min_dim) || //
                            (dims.second <= min_dim);  //

    return both_le_max && one_le_min;
}

////////////////////////////////////////////////// grid_generator methods
grid_generator::grid_generator(const int_pair& max_dims,
                               const std::vector<int>& tile_sequence,
                               bool strips_only)
    : _strips_only(strips_only),
      _max_dims(max_dims),
      _tile_sequence(tile_sequence),
      _idx_invalid(tile_sequence.size()),
      _mask_active_bit(true),
      _mask_inactive_tile(0)
{
    THROW_ASSERT(tile_sequence.size() > 0, "Tile sequence must not be empty!");
    THROW_ASSERT(max_dims.first >= 0 && max_dims.second >= 0,
                 "Max dimensions must be non-negative!");
    THROW_ASSERT(LOGICAL_IMPLIES(_strips_only, _max_dims.first <= 1),
                 "Specified strips only but got max_dims with more than "
                 "1 row!");

    assert(_mask.get() == nullptr);
    _increment(true);
}

grid_generator::grid_generator(const int_pair& max_dims,
                               const std::vector<int>& tile_sequence,
                               bool mask_active_bit, int mask_inactive_tile,
                               bool strips_only,
                               unsigned int grid_hash_symmetry_mask)

    : _strips_only(strips_only),
      _max_dims(max_dims),
      _tile_sequence(tile_sequence),
      _idx_invalid(tile_sequence.size()),
      _mask(new grid_mask(grid_hash_symmetry_mask)),
      _mask_active_bit(mask_active_bit),
      _mask_inactive_tile(mask_inactive_tile)
{

    THROW_ASSERT(tile_sequence.size() > 0, "Tile sequence must not be empty!");
    THROW_ASSERT(max_dims.first >= 0 && max_dims.second >= 0,
                 "Max dimensions must be non-negative!");

    THROW_ASSERT(LOGICAL_IMPLIES(_strips_only, _max_dims.first <= 1),
                 "Specified strips only but got max_dims with more than "
                 "1 row!");

    assert(_mask.get() != nullptr);
    _increment(true);
}

grid_generator::operator bool() const
{
    if (_strips_only)
        return i_grid_generator::dims_le_max_standard(_current_dims, _max_dims);
    return i_grid_generator::dims_le_max_transpose(_current_dims, _max_dims);
}

void grid_generator::operator++()
{
    assert(*this);
    _increment(false);
}

const std::vector<int>& grid_generator::gen_board() const
{
    assert(*this && _real_board_matches_idx_board());
    return _real_board;
}

int_pair grid_generator::get_shape() const
{
    assert(*this);
    return _current_dims;
}

bool grid_generator::only_strips() const
{
    return _max_dims.first <= 1;
}

int grid_generator::get_current_size() const
{
    assert(*this);
    return _current_dims.first * _current_dims.second;
}

bool grid_generator::_increment(bool init)
{
    assert(init || *this);

    bool has_dims = !init;
    bool has_mask = !init;
    bool has_tiles = !init;

    while (true)
    {
        // Try to increment tiles
        if (has_mask && _increment_tiles(!has_tiles))
        {
            assert(*this);
            return true;
        }

        has_tiles = false;

        // Try to increment mask
        if (has_dims && _increment_mask(!has_mask))
        {
            has_mask = true;
            continue;
        }

        has_mask = false;

        // Try to increment dimensions
        if (_increment_dimensions(!has_dims))
        {
            has_dims = true;
            continue;
        }

        has_dims = false;

        assert(!*this);
        return false;
    }
}

bool grid_generator::_increment_dimensions(bool init)
{

    assert(init || *this);

    if (_strips_only)
        return i_grid_generator::increment_dims_standard(_current_dims,
                                                         _max_dims, init);

    return i_grid_generator::increment_dims_transpose(_current_dims, _max_dims,
                                                      init);
}

bool grid_generator::_increment_mask(bool init)
{
    if (_mask.get() == nullptr)
        return init;

    assert(_mask.get() != nullptr);
    if (init)
    {
        _mask->set_shape(_current_dims);
        return *_mask;
    }

    assert(*_mask);
    ++(*_mask);
    return *_mask;
}

bool grid_generator::_increment_tiles(bool init)
{
    if (init)
    {
        _init_board();
        return true;
    }

    assert(_real_board_matches_idx_board() &&      //
           _idx_board.size() == _real_board.size() //
    );

    const size_t SIZE = _idx_board.size();
    bool carry = true;

    for (size_t offset = 1; offset <= SIZE; offset++)
    {
        const size_t idx = SIZE - offset;

        if (_idx_board[idx] == _idx_invalid)
            continue;

        size_t& tile_idx = _idx_board[idx];
        int& tile_real = _real_board[idx];

        tile_idx++;

        if (tile_idx < _idx_invalid)
        {
            tile_real = _tile_sequence[tile_idx];
            carry = false;
            break;
        }

        assert(tile_idx == _idx_invalid);
        tile_idx = 0;
        tile_real = _tile_sequence[0];
    }

    return !carry;
}

bool grid_generator::_real_board_matches_idx_board() const
{
    if (_real_board.size() != _idx_board.size())
        return false;

    const size_t SIZE = _real_board.size();
    for (size_t i = 0; i < SIZE; i++)
    {
        const size_t val_idx = _idx_board[i];
        const int val_real = _real_board[i];

        if (_is_active_tile(i))
        {
            if (val_idx == _idx_invalid ||          //
                val_real != _tile_sequence[val_idx] //
            )
                return false;
        }
        else
        {
            if (val_idx != _idx_invalid ||      //
                val_real != _mask_inactive_tile //
            )
                return false;
        }
    }

    return true;
}

void grid_generator::_init_board()
{
    assert(_current_dims.first >= 0 && //
           _current_dims.second >= 0   //
    );

    const int SIZE = _current_dims.first * _current_dims.second;
    assert(SIZE >= 0);

    _idx_board.resize(SIZE);
    _real_board.resize(SIZE);

    for (int i = 0; i < SIZE; i++)
    {
        if (_is_active_tile(i))
        {
            _idx_board[i] = 0;
            _real_board[i] = _tile_sequence[0];
        }
        else
        {
            _idx_board[i] = _idx_invalid;
            _real_board[i] = _mask_inactive_tile;
        }
    }

    assert(_real_board_matches_idx_board());
}

bool grid_generator::_is_active_tile(int tile_idx) const
{
    assert(0 <= tile_idx &&              //
           tile_idx < get_current_size() //
    );

    if (_mask.get() == nullptr)
        return true;

    return (*_mask)[tile_idx] == _mask_active_bit;
}

