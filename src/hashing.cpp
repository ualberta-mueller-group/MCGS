#include "hashing.h"
#include <iostream>
#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>
#include "clobber_1xn.h"
#include "elephants.h"
#include "nogo_1xn.h"
#include "game.h"
#include "game_type.h"
#include "utilities.h"

#include <random>
#include <unordered_set>

namespace {

//////////////////////////////////////// class zobrist
class zobrist
{
public:
    static void init();
    zobrist(game_type_t type);
    inline uint64_t get_value() {return _value;}

    //template <class T1, class T2>
    template <class T1>
    void toggle_tile(int idx, const T1& val)
    {
        //static_assert(std::is_same_v<T1, T2>, "Type mismatch");

        const size_t N_BYTES = sizeof(T1);
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&val);

        for (size_t i = 0; i < N_BYTES; i++)
        {
            _toggle_tile(idx, i, ptr[i]);
        }
    }

private:
    void _toggle_tile(int idx, int sub_idx, uint8_t byte);

    static std::vector<uint64_t> _table;

    uint64_t _value;
    game_type_t _type;
};

//////////////////////////////////////// implement zobrist

using namespace std;

// position -> subtable -> element

vector<uint64_t> zobrist::_table;

random_device rd;
mt19937 rng(rd());
uniform_int_distribution<uint64_t> dist(1, numeric_limits<uint64_t>::max());



constexpr size_t ZT_POSITIONS = 1024;
constexpr size_t ZT_SUBTABLES_PER_POSITION = sizeof(uint64_t);
constexpr size_t ZT_ELEMS_PER_SUBTABLE = (1 << size_in_bits<uint8_t>());

constexpr size_t ZT_ELEMS_PER_POSITION = (ZT_SUBTABLES_PER_POSITION * ZT_ELEMS_PER_SUBTABLE);

constexpr size_t ZT_ELEM_COUNT = (ZT_POSITIONS * ZT_SUBTABLES_PER_POSITION * ZT_ELEMS_PER_SUBTABLE);

void zobrist::init()
{
    assert(_table.size() == 0);
    cout << "Init table with count: " << ZT_ELEM_COUNT << endl;

    cout << "Size in bytes: " << (ZT_ELEM_COUNT * sizeof(uint64_t)) << endl;

    _table.resize(ZT_ELEM_COUNT);



    for (size_t i = 0; i < ZT_ELEM_COUNT; i++)
    {
        uint64_t val = dist(rng);
        while (val == 0)
        {
            val = dist(rng);
        }

        _table[i] = val;
    }

    cout << "First values are: ";
    cout << _table[0] << " ";
    cout << _table[1] << " ";
    cout << _table[2] << endl;
}

zobrist::zobrist(game_type_t type): _value(0), _type(type)
{
}

void zobrist::_toggle_tile(int idx, int sub_idx, uint8_t byte)
{
    assert(_table.size() == ZT_ELEM_COUNT);

    size_t actual_idx = 0;

    // find position
    actual_idx += idx * ZT_ELEMS_PER_POSITION;

    // within position, find subtable
    actual_idx += sub_idx * ZT_ELEMS_PER_SUBTABLE;

    // within subtable, find element
    actual_idx += (byte + _type) % (256);

    const uint64_t& table_value = _table[actual_idx];

    _value ^= table_value;
}


uint64_t compute_hash(const strip& str)
{
    zobrist z(str.game_type());

    const int N = str.size();
    for (int i = 0; i < N; i++)
    {
        int x = str.at(i);
        z.toggle_tile(i, x);
    }

    return z.get_value();
}

void print_hash(const strip& str)
{
    uint64_t hash = compute_hash(str);
    cout << "GAME IS: " << str << endl;
    cout << "ZOBRIST IS: " << hash << endl;
    cout << endl;
}


////////////////////////////////////////

uint64_t n_zeroes = 0;
uint64_t n_collisions = 0;
uint64_t n_games = 0;
std::unordered_set<uint64_t> hash_set;


void test_hash(uint64_t full_hash)
{

    n_games++;

    uint64_t bottom_half_mask(-1);
    bottom_half_mask >>= 32;

    uint64_t hash = 0;
    hash ^= full_hash & bottom_half_mask;
    hash ^= (full_hash >> 32) & bottom_half_mask;

    if (hash == 0)
    {
        n_zeroes += 1;
        return;
    }

    auto it = hash_set.insert(hash);

    if (!it.second)
    {
        n_collisions += 1;
    }
}

} // namespace


void test_hashing1()
{
    rng.seed(7079);
    zobrist::init();

    for (int size = 1; size <= 16; size++)
    {

        cout << "Size: " << size << endl;

        vector<int> board(size);

        for (int& x : board)
            x = 0;

        auto increment = [&]() -> bool
        {
            board.back() += 1;
            bool carry = false;

            for (auto it = board.rbegin(); it != board.rend(); it++)
            {
                int& val = *it;

                if (carry)
                {
                    val += 1;
                    carry = false;
                }

                if (val >= 3)
                {
                    val %= 3;
                    carry = true;
                }

            }

            return !carry;
        };

        do
        {
            clobber_1xn c(board);
            nogo_1xn n(board);
            elephants e(board);

            uint64_t hash1 = compute_hash(c);
            uint64_t hash2 = compute_hash(n);
            uint64_t hash3 = compute_hash(e);

            test_hash(hash1);
            test_hash(hash2);
            test_hash(hash3);

            //cout << c << " " << hash1 << endl;
            //cout << n << " " << hash2 << endl;
            //cout << endl;


        }
        while (increment());
    }

    cout << "ZEROES: " << n_zeroes << endl;
    cout << "COLLISIONS: " << n_collisions << endl;
    cout << "GAMES: " << n_games << endl;

    cout << "Collision rate: " << 100.0 * (double) n_collisions / (double) n_games << endl;


    const size_t N_GAMES = n_games;
    n_zeroes = 0;
    n_collisions = 0;
    n_games = 0;
    hash_set.clear();

    cout << "Computing random rate" << endl;
    for (size_t i = 0; i < N_GAMES; i++)
    {
        uint64_t hash = dist(rng);
        test_hash(hash);
    }

    cout << "Random collision rate: " << 100.0 * (double) n_collisions / (double) n_games << endl;
}
