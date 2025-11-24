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

////////////////////////////////////////////////// grid_mask methods
grid_mask::grid_mask(unsigned int grid_hash_symmetry_mask)
    : _gh(grid_hash_symmetry_mask), _current_shape(0, 0), _marker_count_end(0)
{
}

size_t grid_mask::size() const
{
    assert(*this);
    return _mask.size();
}

int_pair grid_mask::get_shape() const
{
    assert(*this);
    return _current_shape;
}

void grid_mask::set_shape(const int_pair& shape)
{
    assert(shape.first >= 0 && shape.second >= 0);

    _current_shape = shape;

    const int mask_size = shape.first * shape.second;
    _marker_count_end = mask_size + 1;

    _mask.resize(mask_size);
    for (int i = 0; i < mask_size; i++)
        _mask[i] = false;

    _indices.clear();
    _indices.reserve(mask_size);

    if (*this)
        _increment(true);
}

const std::vector<bool>& grid_mask::get_mask() const
{
    assert(*this);
    return _mask;
}

void grid_mask::reset()
{
    _mask_hash_set.clear();
    set_shape(int_pair(0, 0));
}

bool grid_mask::operator[](size_t idx) const
{
    assert(*this);
    return _mask[idx];
}

grid_mask::operator bool() const
{
    return _indices.size() < _marker_count_end;
}

void grid_mask::operator++()
{
    assert(*this);
    _increment(false);
}

bool grid_mask::_increment(bool init)
{
    assert(*this);

    if (!init)
    {
        if (!_increment_no_hash_check())
            return false;
    }

    while (*this && !_add_mask_hash_to_set())
    {
        if (!_increment_no_hash_check())
            return false;
    }

    return true;
}

bool grid_mask::_increment_no_hash_check()
{
    // At most as many markers as the board would allow
    assert(*this);

    // If there is at least one marker, try to move right, starting with last
    // marker
    if (                                                            //
        (_indices.size() > 0) &&                                    //
        _increment_by_moving(_indices.size() - 1, _mask.size() - 1) //
        )                                                           //
    {
        assert(*this);
        return true;
    }

    // Otherwise add another marker and reset positions of all markers
    if (!_increment_by_adding())
    {
        // Ensure operator bool() returns false
        _marker_count_end = 0;
        assert(!*this);
        return false;
    }

    assert(*this);
    return true;
}

bool grid_mask::_increment_by_moving(size_t marker_number, size_t max_pos)
{
    assert(max_pos >= marker_number);

    // Pick up marker i
    size_t& mi_pos = _indices[marker_number];
    assert(_mask[mi_pos]);
    _mask[mi_pos] = false;

    // Place marker i to the right if possible
    const size_t pos_right = mi_pos + 1;

    if (pos_right <= max_pos)
    {
        mi_pos = pos_right;
        assert(!_mask[pos_right]);
        _mask[pos_right] = true;
        return true;
    }

    // Otherwise try to move marker i-1 to the right, and place
    // marker i to its right
    if (marker_number > 0 &&
        _increment_by_moving(marker_number - 1, max_pos - 1))
    {
        const size_t pos_prev = _indices[marker_number - 1];
        const size_t pos_new = pos_prev + 1;

        assert(pos_new < _mask.size() && !_mask[pos_new]);

        mi_pos = pos_new;
        _mask[pos_new] = true;
        return true;
    }

    return false;
}

bool grid_mask::_increment_by_adding()
{
    assert(*this);

    // No space for another marker, but push marker anyway (for operator bool())
    if (_indices.size() >= _mask.size())
        return false;

    // Reset mask
    {
        const size_t mask_size = _mask.size();
        for (size_t i = 0; i < mask_size; i++)
            _mask[i] = false;
    }

    // Add another marker
    _indices.push_back(0);
    const size_t n_markers = _indices.size();

    for (size_t i = 0; i < n_markers; i++)
    {
        _indices[i] = i;
        _mask[i] = true;
    }

    return true;
}

bool grid_mask::_add_mask_hash_to_set()
{
    _gh.init_from_board_and_type(_mask, _current_shape, 0);

    const hash_t hash = _gh.get_value();
    auto inserted = _mask_hash_set.emplace(hash);

    return inserted.second;
}

// TODO why can't this just use the template from utilities.h? Header problem?
std::ostream& operator<<(std::ostream& os, const grid_mask& mask)
{
    const vector<bool>& vec = mask._mask;

    os << '[';

    const size_t N = vec.size();
    for (size_t i = 0; i < N; i++)
    {
        os << vec[i];

        if (i + 1 < N)
            os << ", ";
    }

    os << ']';
    return os;
}

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

//////////////////////////////////////////////////
void test_grid_generator_stuff()
{
    grid_mask m(GRID_HASH_ACTIVE_MASK_IDENTITY);

    m.set_shape({2, 2});

    while (m)
    {
        grid::print_grid(cout, m.get_mask(), m.get_shape());
        cout << endl;
        ++m;
    }

}

