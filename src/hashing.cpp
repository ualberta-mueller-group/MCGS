#include "hashing.h"
#include "utilities.h"
#include <iostream>
#include "hashing_benchmark.h"
#include <limits>
#include <vector>
#include <random>

////////////////////////////////////////////////// random_table
std::mt19937_64 random_table::_rng;
std::uniform_int_distribution<hash_t> random_table::_dist(1, std::numeric_limits<hash_t>::max());

namespace random_tables {
random_table default_table(32);
random_table type_table(8);
} // namespace random_tables 

random_table::random_table(): _n_positions(DEFAULT_N_POSITIONS)
{
    _init();
}

random_table::random_table(size_t n_positions): _n_positions(n_positions)
{
    _init();
}

void random_table::_init()
{
    assert(_n_positions > 0);
    assert(is_power_of_2(_n_positions));

    static bool seeded = false;
    if (!seeded)
    {
        seeded = true;

        if (DEFAULT_RANDOM_TABLE_SEED == 0)
            _rng.seed(time(0));
        else
            _rng.seed(DEFAULT_RANDOM_TABLE_SEED);
    }

    _wrap_shift_amount = 0;
    while (((_n_positions >> _wrap_shift_amount) & 0x1) == 0)
        _wrap_shift_amount++;

    auto get_number = [&]() -> hash_t
    {
        return _dist(_rng);
    };

    _number_table.resize(256 * _n_positions);

    for (hash_t& val : _number_table)
        val = get_number();

    std::cout << "Initialized with n_positions = " << _n_positions << std::endl;
    std::cout << "Size is " << _number_table.size() << std::endl;
}

////////////////////////////////////////////////// local_hash
void local_hash::toggle_type(const game_type_t& type)
{
    // avoid future compiler warning
    using u_game_type_t = std::make_unsigned_t<game_type_t>;
    const u_game_type_t& utype = reinterpret_cast<const u_game_type_t&>(type);

    _value ^= random_tables::type_table.get(utype, utype);
}

//////////////////////////////////////////////////
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

        hash.toggle_type(g.game_type());

        return hash.get_value();
    };


    benchmark_hash_function(fn);
}
