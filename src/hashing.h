#pragma once
/*
   random_table, local_hash, global_hash
*/

#include <vector>
#include <cstdint>
#include <stddef.h>
#include <climits>
#include <type_traits>
#include "utilities.h"
#include <random>
#include "game_type.h"

typedef uint64_t hash_t;

////////////////////////////////////////////////// random_table
class random_table
{
public:
    random_table(size_t n_positions);

    template <class T>
    hash_t get(const size_t& position, const T& color) const
    {
        // Prevent rotate_interleaved distance from wrapping and overlapping
        static_assert((sizeof(T) * CHAR_BIT) / 8 <= 16); // <= 16 bytes
        // Size of T should be multiple of a byte
        static_assert((sizeof(T) * CHAR_BIT) % 8 == 0);

        hash_t value = 0;

        // Signed values are sign extended when right shifted; avoid this
        using T_Unsigned = std::make_unsigned_t<T>; // NOLINT
        static_assert(sizeof(T_Unsigned) == sizeof(T));
        const T_Unsigned& color_u = reinterpret_cast<const T_Unsigned&>(color);

        const size_t pos_constrained = position % _n_positions;
        const size_t wrap_count = position / _n_positions;
        const size_t base_idx = pos_constrained * _ELEMENTS_PER_POSITION;

        // add "random" offset to relative index
        const hash_t idx_offset = _number_table[base_idx + (wrap_count % _ELEMENTS_PER_POSITION)];
        size_t i = 0;

        do
        {
            // explicitly get 8 bits; the size of a byte may not be 8 bits, and
            // the table only has 256 elements per position
            const uint8_t byte = (color_u >> (i * 8)) & 0xFF;

            const size_t relative_idx = (byte + idx_offset) % _ELEMENTS_PER_POSITION;
            const size_t idx = base_idx + relative_idx;

            hash_t element = _number_table[idx];
            element = rotate_interleaved(element, (3 * (wrap_count + i)) % size_in_bits<hash_t>());
            value ^= element;
        }
        while (                                   // Next byte still within
            (++i * 8) < size_in_bits<T>() &&      // bounds, and remaining
            ((color_u >> (i * 8)) != 0)           // bytes are not all 0.
                );                                //
        /*
            Both conditions are necessary because shifting the full width of
            a variable is undefined, and often produces unexpected results.
        */

        return value;
    }

private:
    void _init();

    static constexpr size_t _ELEMENTS_PER_POSITION = 256;
    static constexpr uint64_t _DEFAULT_RANDOM_TABLE_SEED = 0;

    static std::mt19937_64 _rng;
    static std::uniform_int_distribution<hash_t> _dist;

    const size_t _n_positions;
    size_t _wrap_shift_amount;

    std::vector<hash_t> _number_table;
};

namespace random_tables {
extern random_table default_table;
extern random_table type_table;
} // namespace random_tables 

////////////////////////////////////////////////// local_hash
class local_hash
{
public:
    local_hash(): _value(0)
    {
    }

    inline void reset()
    {
        _value = 0;
    }

    inline hash_t get_value() const
    {
        return _value;
    }

    template <class T>
    void toggle_tile(const size_t& position, const T& color)
    {
        _value ^= random_tables::default_table.get(position, color);
    }

    void toggle_type(const game_type_t& type);

private:
    hash_t _value;
};


////////////////////////////////////////////////////////////
void test_hashing_final();
