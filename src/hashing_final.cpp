#include "hashing_final.h"
#include "utilities.h"
#include <iostream>
#include "hashing_benchmark.h"

#include "throw_assert.h"

////////////////////////////////////////////////// .h files

//////////////////////////////////////// hashing.h
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

namespace {

/*

... 0101 0101
*/


template <class T>
constexpr T alternating_mask()
{
    static_assert(std::is_integral_v<T>);

    T val = 0;

    constexpr uint8_t BYTE_MASK = 0x55;
    constexpr size_t SIZE = sizeof(T);

    for (size_t i = 0; i < SIZE; i++)
    {
        val <<= sizeof(uint8_t) * CHAR_BIT;
        val |= BYTE_MASK;
    }

    return val;
}

template <class T>
T rotate_interleaved(const T& val, size_t distance)
{
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);

    constexpr T MASK1 = alternating_mask<T>();
    constexpr T MASK2 = MASK1 << 1;

    T val1 = rotate_left(val & MASK1, distance);
    T val2 = rotate_right(val & MASK2, distance);

    return val1 | val2;
}

} // namespace


typedef uint64_t hash_t;

class random_table
{
public:
    random_table();
    random_table(size_t n_positions);

    template <class T>
    hash_t get(const size_t& position, const T& color) const
    {
        static_assert(sizeof(T) <= 8);

        hash_t value = 0;

        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&color);

        using T_Unsigned = std::make_unsigned_t<T>; // NOLINT
        const T_Unsigned& color_u = reinterpret_cast<const T_Unsigned&>(color);

        const size_t pos_constrained = position & (_n_positions - 1);
        const size_t wrap_count = position >> _wrap_shift_amount;

        const size_t base_idx = pos_constrained << 8;

        size_t i = 0;


#pragma unroll
        do
        {
            THROW_ASSERT(i == 0);

            const size_t relative_idx = (ptr[i] + wrap_count) & (256 - 1);
            const size_t idx = base_idx + relative_idx;

            hash_t element = _number_table[idx];
            element = rotate_interleaved(element, wrap_count);
            value ^= element;

        }
        while (
            (++i) < sizeof(T) &&
            (color_u >> (i * sizeof(uint8_t) * CHAR_BIT))
                );

        return value;
    }

private:
    void _init(hash_t seed = 0);

    static constexpr size_t DEFAULT_N_POSITIONS = 4;

    const size_t _n_positions;
    size_t _wrap_shift_amount;

    std::vector<hash_t> _number_table;
};

extern random_table default_random_table;

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
        _value ^= default_random_table.get(position, color);
    }

private:
    hash_t _value;
};


////////////////////////////////////////////////// .cpp files

//////////////////////////////////////// hashing.cpp
#include <random>

namespace {



} // namespace

random_table default_random_table;


random_table::random_table(): _n_positions(DEFAULT_N_POSITIONS)
{
    _init();
}

random_table::random_table(size_t n_positions): _n_positions(n_positions)
{
    _init();
}

void random_table::_init(hash_t seed)
{
    assert(_n_positions > 0);
    assert(is_power_of_2(_n_positions));

    _wrap_shift_amount = 0;
    while (((_n_positions >> _wrap_shift_amount) & 0x1) == 0)
        _wrap_shift_amount++;

    std::mt19937_64 rng;
    std::uniform_int_distribution<hash_t> dist(1, std::numeric_limits<hash_t>::max());

    if (seed == 0)
        rng.seed(time(0));
    else
        rng.seed(seed);

    auto get_number = [&]() -> hash_t
    {
        return dist(rng);
    };

    _number_table.resize(256 * _n_positions);

    for (hash_t& val : _number_table)
        val = get_number();

    std::cout << "Initialized with n positions " << _n_positions << std::endl;
    std::cout << "Size is " << _number_table.size() << std::endl;
}



////////////////////////////////////////////////// "main"


using namespace std;

void test_hashing_final()
{
    hash_func_t fn = [](const strip& g) -> hash_t
    {
        local_hash hash;

        const size_t N = g.size();
        for (size_t i = 0; i < N; i++)
        {
            int tile = g.at(i);
            hash.toggle_tile(i, tile);
        }

        hash.toggle_tile(N, g.game_type());

        return hash.get_value();
    };




    benchmark_hash_function(fn);
}
