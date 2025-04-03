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
random_table default_table(1024);
random_table type_table(32);
random_table modifier_table(128);
} // namespace random_tables 

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

        if (_DEFAULT_RANDOM_TABLE_SEED == 0)
            _rng.seed(time(0));
        else
            _rng.seed(_DEFAULT_RANDOM_TABLE_SEED);
    }

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

////////////////////////////////////////////////// local_hash
void local_hash::toggle_type(const game_type_t& type)
{
    // avoid future compiler warning
    using u_game_type_t = std::make_unsigned_t<game_type_t>;
    const u_game_type_t& utype = reinterpret_cast<const u_game_type_t&>(type);

    _value ^= random_tables::type_table.get(utype, utype);
}

////////////////////////////////////////////////// global_hash

void global_hash::reset()
{
    _value = 0;
    _subgame_hashes.clear();
    _subgame_valid_mask.clear();
}

hash_t global_hash::get_value() const
{
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
    return random_tables::modifier_table.get(subgame_idx, base_hash);
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
//    //const int LARGE_INT = 64000;
//    const int LARGE_INT = 100000;
//    const int SMALL_INT = 300;
//
//    // test all games
//    begin_test("all games");
//
//    // integers
//    for (int i = -LARGE_INT; i <= LARGE_INT; i++)
//    {
//        integer_game g(i);
//        test_game(g);
//    }
//
//    // rational
//    for (int bottom_exponent = 0; bottom_exponent < 30; bottom_exponent++)
//    {
//        const int bottom = 1 << bottom_exponent;
//
//        for (int top = -LARGE_INT; top <= LARGE_INT; top++)
//        {
//            dyadic_rational g(top, bottom);
//
//            // only unique games
//            if (g.q() == bottom)
//                test_game(g);
//        }
//    }
//
//    // nimber
//    for (int i = 0; i <= LARGE_INT; i++)
//    {
//        nimber g(i);
//        test_game(g);
//    }
//
//    // up_star
//    for (int i = -LARGE_INT; i <= LARGE_INT; i++)
//    {
//        up_star g1(i, true);
//        up_star g2(i, false);
//
//        test_game(g1);
//        test_game(g2);
//    }
//
//    // switch_game
//    for (int exp1 = 0; exp1 < 30; exp1++)
//    {
//        const int b1 = 1 << exp1;
//        cout << "OUTER EXPONENT: " << exp1 << endl;
//
//        for (int t1 = -SMALL_INT; t1 <= SMALL_INT; t1++)
//        {
//            fraction f1(t1, b1);
//            f1.simplify();
//            if (f1.bottom() != b1)
//                continue;
//
//            for (int exp2 = 0; exp2 < 30; exp2++)
//            {
//                const int b2 = 1 << exp2;
//
//                for (int t2 = -SMALL_INT; t2 <= SMALL_INT; t2++)
//                {
//                    fraction f2(t2, b2);
//                    f2.simplify();
//                    if (f2.bottom() != b2)
//                        continue;
//
//                    switch_game g(f1, f2);
//                    test_game(g);
//                }
//            }
//        }
//    }
//
//    // strip games
//    for (int size = 1; size <= 15; size++)
//    {
//        cout << "SIZE: " << size << endl;
//        vector<int> board(size);
//
//        for (int& x : board)
//            x = 0;
//
//        auto increment = [&]() -> bool
//        {
//            board.back() += 1;
//            bool carry = false;
//
//            for (auto it = board.rbegin(); it != board.rend(); it++)
//            {
//                int& val = *it;
//
//                if (carry)
//                {
//                    val += 1;
//                    carry = false;
//                }
//
//                if (val >= 3)
//                {
//                    assert((val / 3) < 2);
//                    val %= 3;
//                    carry = true;
//                }
//
//            }
//
//            return !carry;
//        };
//
//        do
//        {
//            clobber_1xn c(board);
//            test_game(c);
//
//            nogo_1xn n(board);
//            test_game(n);
//
//            elephants e(board);
//            test_game(e);
//        }
//        while (increment());
//
//    }
//
//    end_test();
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
