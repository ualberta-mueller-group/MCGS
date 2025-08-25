#include "hashing.h"
#include "global_options.h"
#include "utilities.h"
#include "type_table.h"
#include <iostream>
#include "game.h"
#include <limits>
#include <type_traits>
#include <vector>
#include <random>
#include "throw_assert.h"
#include <cstddef>
#include <cassert>
#include <stdexcept>

using namespace std;

////////////////////////////////////////////////// random_table
bool random_table::_did_resize_warning = false;

std::uniform_int_distribution<unsigned long long> random_table::_dist(
    1, std::numeric_limits<unsigned long long>::max());

random_table::random_table(size_t n_positions, uint64_t seed) : _n_positions(0)
{
    assert(seed != 0);

    // TODO: DEBUG PRINTING
    // std::cout << "Random table constructing with seed " << seed << std::endl;

    _init(seed, n_positions);
}

void random_table::print_resize_warning()
{
    if (global::silence_warnings())
        return;

    cerr << "WARNING: a random_table was resized 1 or more times during search."
            " This may affect validity of reported times."
         << endl;
}

void random_table::_init(uint64_t seed, size_t n_positions)
{
    assert(n_positions > 0);

    _rng.seed(seed);

    _resize_to(n_positions);

    // TODO: DEBUG PRINTING
    // std::cout << "Initialized with n_positions = " << n_positions <<
    // std::endl; std::cout << "Size is " << _number_table.size() << std::endl;
}

void random_table::_resize_to(size_t new_n_positions)
{
    assert(new_n_positions > _n_positions);

    auto get_number = [&]() -> hash_t
    {
        return (hash_t) _dist(_rng);
    };

    const size_t old_n_positions = _n_positions;

    _n_positions = new_n_positions;
    _number_table.resize(new_n_positions * _ELEMENTS_PER_POSITION);

    const size_t new_table_size = _number_table.size();
    for (size_t i = old_n_positions * _ELEMENTS_PER_POSITION;
         i < new_table_size; i++)
    {
        _number_table[i] = get_number();
    }
}

namespace {
std::vector<random_table> global_random_tables;
} // namespace

void init_global_random_tables(uint64_t seed)
{
    assert(seed != 0);
    static_assert(sizeof(unsigned long long) >= sizeof(uint64_t));

    std::uniform_int_distribution<unsigned long long> dist(
        1, std::numeric_limits<unsigned long long>::max());
    std::mt19937_64 rng;

    // TODO: DEBUG PRINTING
    // std::cout << "init_global_random_tables() seed: " << seed << endl;

    rng.seed(seed);

    auto next_seed = [&]() -> uint64_t
    {
        /*
           don't seed tables with values directly from the random number
           generator; this may cause table values to overlap
        */
        return (uint64_t) (dist(rng) * 5167);
    };

    assert(global_random_tables.empty());
    global_random_tables.reserve(4);

    assert(RANDOM_TABLE_DEFAULT == 0);
    global_random_tables.emplace_back(1024, next_seed());

    assert(RANDOM_TABLE_TYPE == 1);
    global_random_tables.emplace_back(1, next_seed());

    assert(RANDOM_TABLE_MODIFIER == 2);
    global_random_tables.emplace_back(128, next_seed());

    assert(RANDOM_TABLE_PLAYER == 3);
    global_random_tables.emplace_back(1, next_seed());
}

random_table& get_global_random_table(global_random_table_id table_id)
{
    THROW_ASSERT_DEBUG(
        table_id < global_random_tables.size(),
        std::logic_error("global random tables not initialized yet"));

    return global_random_tables[table_id];
}

////////////////////////////////////////////////// local_hash
void local_hash::toggle_type(game_type_t type)
{
    static_assert(!is_signed_v<game_type_t>);

    random_table& rt = get_global_random_table(RANDOM_TABLE_TYPE);
    _value ^= rt.get_zobrist_val(0, type);
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

void global_hash::add_subgame(size_t subgame_idx, const game* g)
{
    _resize_if_out_of_range(subgame_idx);
    assert(!_subgame_valid_mask[subgame_idx]);

    hash_t modified_hash = _get_modified_hash(subgame_idx, g);

    _subgame_valid_mask[subgame_idx] = true;
    _subgame_hashes[subgame_idx] = modified_hash;

    _value ^= modified_hash;
}

void global_hash::remove_subgame(size_t subgame_idx, const game* g)
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
        _value ^= rt.get_zobrist_val(0, _to_play);
        _to_play = EMPTY;
    }

    _to_play = new_to_play;
    _value ^= rt.get_zobrist_val(0, new_to_play);
}

void global_hash::_resize_if_out_of_range(size_t subgame_idx)
{
    assert(_subgame_hashes.size() == _subgame_valid_mask.size());
    const size_t current_size = _subgame_hashes.size();

    if (subgame_idx < current_size)
        return;

    const size_t target_size = new_vector_capacity(subgame_idx, current_size);
    assert(subgame_idx < target_size);

    _subgame_hashes.resize(target_size);
    _subgame_valid_mask.resize(target_size);
}

void global_hash::_reserve_space(size_t capacity)
{
    _subgame_hashes.reserve(capacity);
    _subgame_valid_mask.reserve(capacity);
}

hash_t global_hash::_get_modified_hash(size_t subgame_idx, const game* g)
{
    hash_t base_hash = g->get_local_hash();

    random_table& rt = get_global_random_table(RANDOM_TABLE_MODIFIER);
    return rt.get_zobrist_val(subgame_idx, base_hash);
}
