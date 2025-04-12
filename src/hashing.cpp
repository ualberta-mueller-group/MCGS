#include "hashing.h"
#include "utilities.h"
#include <iostream>
#include "game.h"
#include <limits>
#include <type_traits>
#include <vector>
#include <random>
#include "throw_assert.h"

using namespace std;

////////////////////////////////////////////////// random_table
std::uniform_int_distribution<unsigned long long> random_table::_dist(1, std::numeric_limits<unsigned long long>::max());

random_table::random_table(size_t n_positions, uint64_t seed): _n_positions(n_positions)
{
    while (seed == 0)
        seed = ms_since_epoch();

    std::cout << "Random table constructing with seed " << seed << std::endl;

    _init(seed);
}

void random_table::_init(uint64_t seed)
{
    assert(_n_positions > 0);
    assert(is_power_of_2(_n_positions));

    _rng.seed(seed);

    auto get_number = [&]() -> hash_t
    {
        return (hash_t) _dist(_rng);
    };

    _number_table.resize(_n_positions * _ELEMENTS_PER_POSITION);

    for (hash_t& element : _number_table)
        element = get_number();

    std::cout << "Initialized with n_positions = " << _n_positions << std::endl;
    std::cout << "Size is " << _number_table.size() << std::endl;
}

namespace {
std::vector<random_table> global_random_tables;
} // namespace

void init_global_random_tables(uint64_t seed)
{
    static_assert(sizeof(unsigned long long) >= sizeof(uint64_t));
    std::uniform_int_distribution<unsigned long long> dist(1, std::numeric_limits<unsigned long long>::max());
    std::mt19937_64 rng;

    std::cout << "init_global_random_tables() seed: " << seed << endl;

    while (seed == 0)
        seed = ms_since_epoch();

    rng.seed(seed);

    auto next_seed = [&]() -> uint64_t
    {
        return (uint64_t) (dist(rng) * 5167);
    };

    assert(global_random_tables.empty());

    assert(RANDOM_TABLE_DEFAULT == 0);
    global_random_tables.emplace_back(1024, next_seed());

    assert(RANDOM_TABLE_TYPE == 1);
    global_random_tables.emplace_back(32, next_seed());

    assert(RANDOM_TABLE_MODIFIER == 2);
    global_random_tables.emplace_back(128, next_seed());

    assert(RANDOM_TABLE_PLAYER == 3);
    global_random_tables.emplace_back(1, next_seed());
}

random_table& get_global_random_table(global_random_table_id table_id)
{
    THROW_ASSERT(table_id < global_random_tables.size(),
                 std::logic_error("global random tables not initialized yet"));

    return global_random_tables[table_id];
}

////////////////////////////////////////////////// local_hash
void local_hash::toggle_type(game_type_t type)
{
    static_assert(!is_signed_v<game_type_t>);

    random_table& rt = get_global_random_table(RANDOM_TABLE_TYPE);
    _value ^= rt.get_zobrist_val(type, type);
}

////////////////////////////////////////////////// global_hash

void global_hash::reset()
{
    _value = 0;
    _to_play = EMPTY;
    _subgame_hashes.clear();
    _subgame_valid_mask.clear();
}

hash_t global_hash::get_value() const
{
    assert(_to_play != EMPTY); // Don't forget to set the player
    return _value;
}

void global_hash::add_subgame(size_t subgame_idx, game* g)
{
    _resize_if_out_of_range(subgame_idx);
    assert(!_subgame_valid_mask[subgame_idx]);

    hash_t modified_hash = _get_modified_hash(subgame_idx, g);

    _subgame_valid_mask[subgame_idx] = true;
    _subgame_hashes[subgame_idx] = modified_hash;

    _value ^= modified_hash;
}

void global_hash::remove_subgame(size_t subgame_idx, game* g)
{
    _resize_if_out_of_range(subgame_idx);
    assert(_subgame_valid_mask[subgame_idx]);
    assert(_subgame_hashes[subgame_idx] == _get_modified_hash(subgame_idx, g));

    _value ^= _subgame_hashes[subgame_idx];
    _subgame_valid_mask[subgame_idx] = false;
    _subgame_hashes[subgame_idx] = 0;
}

void global_hash::set_to_play(bw new_to_play)
{
    assert(new_to_play == BLACK || new_to_play == WHITE);

    random_table& rt = get_global_random_table(RANDOM_TABLE_PLAYER);

    if (_to_play != EMPTY)
    {
        _value ^= rt.get_zobrist_val(_to_play, _to_play);
        _to_play = EMPTY;
    }

    _to_play = new_to_play;
    _value ^= rt.get_zobrist_val(new_to_play, new_to_play);
}

void global_hash::_resize_if_out_of_range(size_t subgame_idx)
{
    assert(_subgame_hashes.size() == _subgame_valid_mask.size());
    size_t min_size = subgame_idx + 1;
    if (_subgame_hashes.size() < min_size)
    {
        _subgame_hashes.resize(min_size);
        _subgame_valid_mask.resize(min_size);
    }
}

hash_t global_hash::_get_modified_hash(size_t subgame_idx, game* g)
{
    hash_t base_hash = g->get_local_hash();

    random_table& rt = get_global_random_table(RANDOM_TABLE_MODIFIER);
    return rt.get_zobrist_val(subgame_idx, base_hash);
}

