#include "hashing.h"
#include <iostream>
#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>
#include "clobber_1xn.h"
#include "elephants.h"
#include "hashing_benchmark.h"
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

    template <class T_Explicit, class T>
    void toggle_tile(int idx, const T& val)
    {
        static_assert(std::is_same_v<T_Explicit, T>, "Type mismatch");

        const size_t N_BYTES = sizeof(T);
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&val);

        for (size_t i = 0; i < N_BYTES; i++)
        {
            _toggle_tile(idx, i, ptr[i]);
        }
    }

    void asd()
    {
        int x = 4;

        toggle_tile<int>(0, x);
        //toggle_tile<double>(0, x);
        //toggle_tile(0, x);
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

//random_device rd;
mt19937_64 rng;
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


inline uint64_t compute_hash(const strip& str)
{
    //zobrist z(str.game_type());
    zobrist z(1);

    const size_t N = str.size();
    for (size_t i = 0; i < N; i++)
    {
        int x = str.at(i);
        z.toggle_tile<int>(i, x);
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


} // namespace


void test_hashing1()
{
    rng.seed(time(0));
    zobrist::init();

    hash_func_t fn = [&](const strip& g) -> uint64_t
    {
        return compute_hash(g);
    };

    benchmark_hash_function(fn);
}
