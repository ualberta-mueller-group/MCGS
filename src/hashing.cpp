#include "hashing.h"
#include "utilities.h"
#include <iostream>
#include "hashing_benchmark.h"
#include <limits>
#include <vector>
#include <random>
#include "throw_assert.h"

using namespace std;

////////////////////////////////////////////////// random_table
std::uniform_int_distribution<hash_t> random_table::_dist(1, std::numeric_limits<hash_t>::max());

random_table::random_table(size_t n_positions, uint64_t seed): _n_positions(n_positions)
{
    while (seed == 0)
        seed = ms_since_epoch();

    std::cout << "Random table constructing with seed " << seed << std::endl;

    _rng.seed(seed);
    _init();
}

void random_table::_init()
{
    assert(_n_positions > 0);
    assert(is_power_of_2(_n_positions));

    _wrap_shift_amount = 0;
    while (((_n_positions >> _wrap_shift_amount) & 0x1) == 0)
        _wrap_shift_amount++;

    auto get_number = [&]() -> hash_t
    {
        return _dist(_rng);
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
    std::uniform_int_distribution<uint64_t> dist(1, std::numeric_limits<uint64_t>::max());
    std::mt19937_64 rng;

    std::cout << "init_global_random_tables() seed: " << seed << endl;

    while (seed == 0)
        seed = ms_since_epoch();

    rng.seed(seed);

    auto next_seed = [&]() -> uint64_t
    {
        return dist(rng) * 5167;
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
void local_hash::toggle_type(const game_type_t& type)
{
    // avoid future compiler warning
    using u_game_type_t = std::make_unsigned_t<game_type_t>;
    const u_game_type_t& utype = reinterpret_cast<const u_game_type_t&>(type);

    random_table& rt = get_global_random_table(RANDOM_TABLE_TYPE);
    _value ^= rt.get(utype, utype);
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
        _value ^= rt.get(_to_play, _to_play);
        _to_play = EMPTY;
    }

    _to_play = new_to_play;
    _value ^= rt.get(new_to_play, new_to_play);
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
    hash_t base_hash = g->compute_hash().get_value();

    random_table& rt = get_global_random_table(RANDOM_TABLE_MODIFIER);
    return rt.get(subgame_idx, base_hash);
}

void test_hashing_final()
{
//
//
//    /*
//    hash_func_t fn = [](const strip& g) -> hash_t
//    {
//        local_hash hash;
//
//        const size_t N = g.size();
//        for (size_t i = 0; i < N; i++)
//        {
//            int tile = g.at(i);
//            hash.toggle_tile(i, tile);
//        }
//
//        hash.toggle_type(g.game_type());
//
//        return hash.get_value();
//    };
//
//
//    benchmark_hash_function(fn);
//    */
//
//    unordered_set<hash_t> hash_set;
//    uint64_t n_games = 0;
//    uint64_t n_collisions = 0;
//    uint64_t n_zeroes = 0;
//    string test_name;
//
//    auto test_hash = [&](const hash_t& hash) -> void
//    {
//        n_games++;
//
//        if (hash == 0)
//            n_zeroes++;
//        else
//        {
//            auto it = hash_set.insert(hash);
//            if (!it.second)
//                n_collisions++;
//        }
//    };
//
//    auto test_game = [&](game& g) -> void
//    {
//        const hash_t hash = g.compute_hash().get_value();
//        test_hash(hash);
//    };
//
//    auto begin_test = [&](const string& name) -> void
//    {
//        hash_set.clear();
//        n_games = 0;
//        n_collisions = 0;
//        n_zeroes = 0;
//        test_name = name;
//    };
//
//    auto end_test = [&]() -> void
//    {
//        cout << "TEST: \"" << test_name << "\"" << endl;
//        cout << "Games: " << n_games << endl;
//        cout << "Collisions: " << n_collisions << endl;
//        cout << "Zeroes: " << n_zeroes << endl;
//        cout << "Collision %: " << (100.0 * (double) n_collisions) / (double) n_games << endl;
//        cout << "Set size: " << hash_set.size() << endl;
//        cout << endl;
//    };
//
//    /*
//    */
//
//    begin_test("Sums");
//
//
//    strip_iterator it1(10);
//
//    while (++it1)
//    {
//        const vector<int>& board1 = it1.get();
//        //clobber_1xn g1(board1);
//
//        for (int n_repeats = 1; n_repeats <= 70; n_repeats++)
//        {
//            vector<game*> games;
//            games.resize(n_repeats);
//
//            sumgame sum(BLACK);
//
//            //global_hash gh;
//
//            for (int i = 0; i < n_repeats; i++)
//            {
//                //gh.add_subgame(i, &g1);
//
//                game* g = new clobber_1xn(board1);
//                games[i] = g;
//                sum.add(g);
//            }
//
//            //hash_t value = gh.get_value();
//            hash_t value = sum.get_global_hash_value();
//
//            for (game* g : games)
//                delete g;
//
//            test_hash(value);
//        }
//
//        /*
//        strip_iterator it2(4);
//        while (++it2)
//        {
//            const vector<int>& board2 = it2.get();
//            clobber_1xn g2(board2);
//
//            global_hash hash;
//            hash.add_subgame(0, &g1);
//            hash.add_subgame(1, &g2);
//
//            hash_t value = hash.get_value();
//            test_hash(value);
//        }
//        */
//    }
//
//
//
//    end_test();
}
