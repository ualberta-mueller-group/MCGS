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


