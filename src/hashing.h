#pragma once
/*
   Defines data types used for game hashing:
       hash_t, random_table, local_hash, global_hash
*/

// IWYU pragma: begin_exports
#include <cstdint>
// IWYU pragma: end_exports

#include <vector>
#include <cstddef>
#include <climits>
#include <type_traits>
#include "global_options.h"
#include "utilities.h"
#include <random>
#include <iostream>
#include <cassert>
#include "cgt_basics.h"
#include "game_type.h"

class game;

typedef uint64_t hash_t;

////////////////////////////////////////////////// random_table
class random_table
{
public:
    random_table(size_t n_positions, uint64_t seed);

    /*
       Position of get_zobrist_val functions can be past
           current_size() -- table will automatically grow
    */
    // get a number from the random_table for zobrist hashing
    template <class T>
    hash_t get_zobrist_val(size_t position, const T& color);

    // Because bool is invalid input to std::make_unsigned_t<T>
    inline hash_t get_zobrist_val(size_t position, bool color);

    size_t current_size() const;

    inline static bool did_resize_warning();
    static void print_resize_warning();

private:
    void _init(uint64_t seed, size_t n_positions);
    void _resize_to(size_t new_n_positions);
    inline void _resize_if_out_of_range(size_t idx);

    static constexpr size_t _ELEMENTS_PER_POSITION = 256;

    // using hash_t for int distribution may be undefined behavior
    static_assert(sizeof(unsigned long long) >= sizeof(hash_t));
    static std::uniform_int_distribution<unsigned long long> _dist;

    static bool _did_resize_warning;

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
    } while (                            // Next byte still within
        (++i * 8) < size_in_bits<T>() && // bounds, and remaining
        ((color_u >> (i * 8)) != 0)      // bytes are not all 0.
    ); //
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

inline size_t random_table::current_size() const
{
    return _n_positions;
}

inline bool random_table::did_resize_warning()
{
    return _did_resize_warning;
}

inline void random_table::_resize_if_out_of_range(size_t idx)
{
    if (idx < _n_positions)
        return;

    const size_t target_size = new_vector_capacity(idx, _n_positions);
    assert(idx < target_size);

    _resize_to(target_size);

    if (!_did_resize_warning && !global::silence_warnings())
    {
        _did_resize_warning = true;
        std::cerr << "WARNING: random_table resized" << std::endl;
    }
}

////////////////////////////////////////////////// global random_tables
enum global_random_table_id
{
    RANDOM_TABLE_DEFAULT = 0, // for content of a game (i.e. grid tiles)
    RANDOM_TABLE_TYPE,        // for game type
    RANDOM_TABLE_MODIFIER,    // to modify local_hash in a sum based on its
                              // position
    RANDOM_TABLE_PLAYER,      // for player color (i.e. "to_play")
};

void init_global_random_tables(uint64_t seed);
random_table& get_global_random_table(global_random_table_id table_id);

////////////////////////////////////////////////// local_hash
class local_hash
{
public:
    local_hash() : _value(0) {}

    inline void reset() { _value = 0; }

    inline hash_t get_value() const { return _value; }

    template <class T>
    void toggle_value(size_t position, const T& color)
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
    global_hash() : _value(0), _to_play(EMPTY) { _reserve_space(32); }

    void reset();
    hash_t get_value() const;

    void add_subgame(size_t subgame_idx, game* g);
    void remove_subgame(size_t subgame_idx, game* g);

    void set_to_play(bw to_play);

private:
    void _resize_if_out_of_range(size_t subgame_idx);
    void _reserve_space(size_t capacity);

    // hash modifier of a local_hash based on subgame index
    hash_t _get_modified_hash(size_t subgame_idx, game* g);

    hash_t _value;

    ebw _to_play;
    std::vector<hash_t> _subgame_hashes;
    std::vector<bool> _subgame_valid_mask;
};
