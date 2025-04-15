#pragma once
/*
   Defines data types used for game hashing:
       hash_t, random_table, local_hash, global_hash
*/

#include <vector>
#include <cstdint>
#include <stddef.h>
#include <climits>
#include <type_traits>
#include "utilities.h"
#include <random>
#include "game_type.h"
#include <iostream>

typedef uint64_t hash_t;

////////////////////////////////////////////////// random_table
class random_table
{
public:
    // seed 0 means seed with current system time
    random_table(size_t n_positions, uint64_t seed);

    // get a number from the random_table for zobrist hashing
    template <class T>
    hash_t get_zobrist_val(size_t position, const T& color);

    // Because bool is invalid input to std::make_unsigned_t<T>
    inline hash_t get_zobrist_val(size_t position, bool color);

    inline static bool did_resize();

    static void print_resize_warning();


private:
    void _init(uint64_t seed, size_t n_positions);
    void _resize_to(size_t new_n_positions);
    inline void _resize_if_out_of_range(size_t idx);

    static constexpr size_t _ELEMENTS_PER_POSITION = 256;

    // using hash_t for int distribution may be undefined behavior
    static_assert(sizeof(unsigned long long) >= sizeof(hash_t));
    static std::uniform_int_distribution<unsigned long long> _dist;

    static bool _did_resize;

    std::mt19937_64 _rng;
    size_t _n_positions;
    std::vector<hash_t> _number_table;
};

////////////////////////////////////////////////// random_table implementation
template <class T>
hash_t random_table::get_zobrist_val(size_t position, const T& color)
{
    static_assert((sizeof(T) * CHAR_BIT) / 8 <= 16); // T at most 16 bytes
    // Size of T should be multiple of a byte
    static_assert((sizeof(T) * CHAR_BIT) % 8 == 0);

    hash_t value = 0;

    _resize_if_out_of_range(position);

    // don't right shift signed values
    // NOLINTNEXTLINE(readability-identifier-naming)
    using T_Unsigned = std::make_unsigned_t<T>;
    static_assert(sizeof(T_Unsigned) == sizeof(T));
    const T_Unsigned& color_u = reinterpret_cast<const T_Unsigned&>(color);

    const size_t base_idx = position * _ELEMENTS_PER_POSITION;

    size_t i = 0;

    do
    {
        // explicitly get 8 bits; the size of a byte may not be 8 bits, and
        // the table only has 256 elements per position
        const uint8_t byte = (color_u >> (i * 8)) & 0xFF;

        const size_t idx = base_idx + ((size_t) byte);

        hash_t element = _number_table[idx];
        element = rotate_interleaved(element, (3 * i) % size_in_bits<hash_t>());
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

inline hash_t random_table::get_zobrist_val(size_t position, bool color)
{
    return get_zobrist_val(position, (uint8_t) color);
}

inline bool random_table::did_resize()
{
    return _did_resize;
}

inline void random_table::_resize_if_out_of_range(size_t idx)
{
    if (idx < _n_positions)
        return;

    _resize_to(idx + 1);

    if (!_did_resize)
    {
        _did_resize = true;
        std::cerr << "WARNING: random_table resized" << std::endl;
    }
}

////////////////////////////////////////////////// global random_tables
enum global_random_table_id
{
    RANDOM_TABLE_DEFAULT = 0, // for content of a game (nogo tiles, integer_game int etc)
    RANDOM_TABLE_TYPE, // for game type
    RANDOM_TABLE_MODIFIER, // to modify local_hash in a sum based on its position
    RANDOM_TABLE_PLAYER, // for player color (i.e. "to_play")
};

// seed 0 means seed with current time
void init_global_random_tables(uint64_t seed);
random_table& get_global_random_table(global_random_table_id table_id);

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
    void toggle_tile(size_t position, const T& color)
    {
        random_table& rt = get_global_random_table(RANDOM_TABLE_DEFAULT);
        _value ^= rt.get_zobrist_val(position, color);
    }

    void toggle_type(game_type_t type);

private:
    hash_t _value;
};

////////////////////////////////////////////////// global_hash

class global_hash
{
public:
    global_hash(): _value(0), _to_play(EMPTY)
    {
    }

    void reset();
    hash_t get_value() const;

    void add_subgame(size_t subgame_idx, game* g);
    void remove_subgame(size_t subgame_idx, game* g);

    void set_to_play(bw to_play);

private:
    void _resize_if_out_of_range(size_t subgame_idx);

    // hash modifier of a local_hash based on subgame index
    hash_t _get_modified_hash(size_t subgame_idx, game* g);

    hash_t _value;

    ebw _to_play;
    std::vector<hash_t> _subgame_hashes;
    std::vector<bool> _subgame_valid_mask;
};

