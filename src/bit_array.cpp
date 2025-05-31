#include "bit_array.h"
#include <cassert>
#include <limits>
#include <cstddef>
#include <ostream>

bool bit_array::get(size_t idx) const
{
    assert(_idx_in_range(idx));

    size_t element_idx, shift_amount;
    _get_bit_indices(idx, element_idx, shift_amount);

    return ((_vec[element_idx] >> shift_amount) & 0x1) != 0;
}

void bit_array::set(size_t idx, bool new_value)
{
    assert(_idx_in_range(idx));

    size_t element_idx, shift_amount;
    _get_bit_indices(idx, element_idx, shift_amount);

    const unsigned int BIT_MASK = ((unsigned int)(1)) << shift_amount;
    const unsigned int INV_BIT_MASK = ~BIT_MASK;

    unsigned int& element = _vec[element_idx];

    if (new_value)
        element |= BIT_MASK;
    else
        element &= INV_BIT_MASK;
}

bit_array_relation bit_array::compare(const bit_array& other) const
{
    assert(size() == other.size());

    if (size() == 0)
        return BIT_ARRAY_ALL_OVERLAP;

    bool some_overlap = false;
    bool all_overlap = true;

    for (size_t i = 0; i < _vec_size; i++)
    {
        const unsigned int& chunk1 = _vec[i];
        const unsigned int& chunk2 = other._vec[i];

        some_overlap |= ((chunk1 & chunk2) != 0);
        all_overlap &= (chunk1 == chunk2);
    }

    if (all_overlap)
        return BIT_ARRAY_ALL_OVERLAP;
    if (some_overlap)
        return BIT_ARRAY_SOME_OVERLAP;

    return BIT_ARRAY_NO_OVERLAP;
}

void bit_array::_init(bool default_value)
{
    assert(_vec_size == 0 && _vec.size() == 0);

    if (_n_bits == 0)
        return;

    _vec_size = _n_bits / _BITS_PER_ELEMENT;
    _vec_size += (_n_bits % _BITS_PER_ELEMENT) != 0 ? 1 : 0;
    assert((_vec_size * _BITS_PER_ELEMENT) >= _n_bits);

    _vec.reserve(_vec_size);

    const unsigned int init_element = default_value ? 
        std::numeric_limits<unsigned int>::max() : 0;

    for (size_t i = 0; i < _vec_size; i++)
        _vec.push_back(init_element);

    // Discard some bits from last element
    const size_t remainder_bits = _n_bits % _BITS_PER_ELEMENT;
    if (remainder_bits != 0)
    {
        const size_t discard_bits = _BITS_PER_ELEMENT - remainder_bits;

        assert(discard_bits < _BITS_PER_ELEMENT);
        _vec[_vec_size - 1] >>= discard_bits;
    }
}

inline void bit_array::_get_bit_indices(size_t idx, size_t& element_idx, size_t& shift_amount) const
{
    assert(_idx_in_range(idx));

    element_idx = idx / _BITS_PER_ELEMENT;
    shift_amount = idx % _BITS_PER_ELEMENT;

    assert(0 <= element_idx && element_idx < _vec_size);
    assert(0 <= shift_amount && shift_amount < _BITS_PER_ELEMENT);
}

void bit_array::_move_impl(bit_array&& rhs)
{
    assert(_n_bits == rhs._n_bits);
    _vec_size = std::move(rhs._vec_size);
    _vec = std::move(rhs._vec);
}

//////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, const bit_array& arr)
{
    os << '[';

    const size_t N = arr.size();
    for (size_t i = 0; i < N; i++)
        os << arr.get(N - 1 - i);

    os << ']';
    return os;
}
