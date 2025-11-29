#pragma once

#include <cassert>
#include <cstddef>
#include <unordered_set>
#include <vector>
#include <ostream>

#include "int_pair.h"
#include "grid_hash.h"
#include "hashing.h"

////////////////////////////////////////////////// class grid_mask

/*
    NOTE: set_shape() will advance to the first unique mask according
    to the grid_hash. However, it will not clear the set of seen hashes.
*/
class grid_mask
{
public:
    grid_mask(unsigned int grid_hash_symmetry_mask);

    size_t size() const; // number of tiles (excluding SEP)
    int_pair get_shape() const;

    void set_shape(const int_pair& shape);

    const std::vector<bool>& get_mask() const;

    void reset();

    // TODO weird idx type -- should have one for ints?
    bool operator[](size_t idx) const;

    operator bool() const;
    void operator++();

protected:
    mutable grid_hash _gh;
    std::unordered_set<hash_t> _mask_hash_set;

    int_pair _current_shape;
    size_t _marker_count_end;

    std::vector<bool> _mask;
    std::vector<size_t> _indices; // locations of each marker in the mask

    bool _increment(bool init);
    bool _increment_no_hash_check();
    bool _increment_by_moving(size_t marker_idx, size_t max_pos);
    bool _increment_by_adding();

    bool _add_mask_hash_to_set();

private:
    friend std::ostream& operator<<(std::ostream& os, const grid_mask& mask);
};

std::ostream& operator<<(std::ostream& os, const grid_mask& mask);


////////////////////////////////////////////////// grid_mask methods
inline grid_mask::grid_mask(unsigned int grid_hash_symmetry_mask)
    : _gh(grid_hash_symmetry_mask), _current_shape(0, 0), _marker_count_end(0)
{
}

inline size_t grid_mask::size() const
{
    assert(*this);
    return _mask.size();
}

inline int_pair grid_mask::get_shape() const
{
    assert(*this);
    return _current_shape;
}

inline const std::vector<bool>& grid_mask::get_mask() const
{
    assert(*this);
    return _mask;
}

inline void grid_mask::reset()
{
    _mask_hash_set.clear();
    set_shape(int_pair(0, 0));
}

inline bool grid_mask::operator[](size_t idx) const
{
    assert(*this);
    return _mask[idx];
}

inline grid_mask::operator bool() const
{
    return _indices.size() < _marker_count_end;
}

inline void grid_mask::operator++()
{
    assert(*this);
    _increment(false);
}

