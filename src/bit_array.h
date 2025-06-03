#pragma once

#include <cstddef>
#include <cassert>
#include "utilities.h"
#include "throw_assert.h"
#include <ostream>
#include <utility>
#include <vector>

enum bit_array_relation
{
    BIT_ARRAY_NO_OVERLAP = 0,
    BIT_ARRAY_SOME_OVERLAP,
    BIT_ARRAY_ALL_OVERLAP,
};

class bit_array
{
public:
    bit_array(size_t n_bits); // default all bits to false
    bit_array(size_t n_bits, bool default_value);

    bit_array(const bit_array& rhs);
    bit_array& operator=(const bit_array& rhs);

    bit_array(bit_array&& rhs);
    bit_array& operator=(bit_array&& rhs);

    // size in bits
    size_t size() const;

    bool get(size_t idx) const;
    void set(size_t idx, bool new_value);

    // Must be same size
    bit_array_relation compare(const bit_array& other) const;

private:
    bool _idx_in_range(size_t idx) const;
    void _init(bool default_value);
    void _get_bit_indices(size_t idx, size_t& element_idx, size_t& shift_amount) const;

    void _move_impl(bit_array&& rhs);

    size_t _n_bits;

    static constexpr size_t _BITS_PER_ELEMENT = size_in_bits<unsigned int>();
    size_t _vec_size;
    std::vector<unsigned int> _vec;
};

// Prints in order of decreasing index
std::ostream& operator<<(std::ostream& os, const bit_array& arr);

//////////////////////////////////////////////////
inline bit_array::bit_array(size_t n_bits)
    : _n_bits(n_bits),
      _vec_size(0)
{
    THROW_ASSERT_DEBUG(n_bits >= 0);
    _init(false);
}

inline bit_array::bit_array(size_t n_bits, bool default_value)
    : _n_bits(n_bits),
      _vec_size(0)
{
    THROW_ASSERT_DEBUG(n_bits >= 0);
    _init(default_value);
}

inline bit_array::bit_array(bit_array&& rhs): _n_bits(rhs._n_bits)
{
    _move_impl(std::forward<bit_array>(rhs));
}

inline bit_array& bit_array::operator=(bit_array&& rhs)
{
    _move_impl(std::forward<bit_array>(rhs));
    return *this;
}

inline size_t bit_array::size() const
{
    return _n_bits;
}

inline bool bit_array::_idx_in_range(size_t idx) const
{
    return (0 <= idx) && (idx < _n_bits);
}

