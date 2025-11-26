#include "grid_mask.h"

#include <cassert>
#include <vector>
#include <ostream>
#include <cstddef>

#include "hashing.h"

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

