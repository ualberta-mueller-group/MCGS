#include "hashing3.h"
#include <algorithm>
#include <functional>
#include <random>
#include <type_traits>
#include <unordered_map>
#include <map>
#include <vector>
#include <cstdint>
#include <cassert>
#include "clobber_1xn.h"
#include "elephants.h"
#include "game_type.h"
#include "nogo_1xn.h"
#include "safe_arithmetic.h"
#include "throw_assert.h"
#include <iostream>
#include "hashing_benchmark.h"

/* 
    TODO add warnings when table size becomes extremely large when it 
    probably shouldn't?
*/


namespace {
//////////////////////////////////////// random_table

// NOLINTBEGIN(readability-identifier-naming)
// NOLINTEND(readability-identifier-naming)


//static std::random_device rd;
std::mt19937_64 rng;
std::uniform_int_distribution<hash_t> dist(1, UINT64_MAX);

inline hash_t get_random_hash_t()
{

    hash_t value = 0;
    while (value == 0)
        value = dist(rng);

    return value;
}


typedef int int_type_t;

template <class T>
constexpr int_type_t get_int_type()
{
    static_assert(std::is_integral_v<T>);

    constexpr int_type_t WIDTH = sizeof(T);
    return std::is_signed_v<T> ? -WIDTH : WIDTH;
}

struct letter_t
{
    const int_type_t type;
    const uint64_t value; // Should be u64 and not hash_t

    letter_t() = delete;

    template <class T_Explicit, class T>
    static letter_t get_letter(const T& value)
    {
        static_assert(std::is_same_v<T_Explicit, T>, "Type mismatch");
        static_assert(sizeof(uint64_t) >= sizeof(T));

        constexpr int_type_t INT_TYPE = get_int_type<T>();

        letter_t letter(INT_TYPE, static_cast<uint64_t>(value));
        return letter;
    }

    inline bool operator==(const letter_t& rhs) const
    {
        return (value == rhs.value) && (type == rhs.type);
    }
    
    inline bool operator<(const letter_t& rhs) const
    {
        if (type != rhs.type)
            return type < rhs.type;

        if (value != rhs.value)
            return value < rhs.value;

        return false;
    }

    class hash
    {
    public:
        inline size_t operator()(const letter_t& letter) const
        {
            return (letter.value ^ letter.type);
        }
    };


private:
    letter_t(const int_type_t& type, const uint64_t& value): type(type), value(value)
    {
    }
};


typedef std::pair<uint64_t, size_t> offset_pair_t;

class random_table
{
public:
    typedef std::unordered_map<letter_t, size_t, letter_t::hash> offset_map_t;
    //typedef std::map<letter_t, size_t> offset_map_t;
    typedef std::vector<hash_t> table_t;


    random_table() = delete;

    random_table(const std::vector<letter_t>& letters)
        : _offset_map(),
        _position_stride(0),
        _table(),
        _table_position_count(0),
        _simple_offset_map(),
        _simple_offset_vec()
    {
        size_t next_offset = 0;

        for (const letter_t& l : letters)
        {
            _offset_map.insert({l, next_offset});
            _simple_offset_map.insert({l.value, next_offset});
            _simple_offset_vec.push_back({l.value, next_offset});

            next_offset++;
        }

        // no duplicates
        THROW_ASSERT(letters.size() == _offset_map.size());
        THROW_ASSERT(letters.size() == _simple_offset_map.size());
        THROW_ASSERT(letters.size() == _simple_offset_vec.size());

        std::sort(_simple_offset_vec.begin(), _simple_offset_vec.end(),
        [](const offset_pair_t& p1, const offset_pair_t& p2) -> bool
        {
            return p1.first < p2.first;
        });


        _position_stride = _offset_map.size();
        _extend_table_if_needed(127);
    }

    template <class T_Explicit, class T>
    hash_t get(size_t position, const T& value)
    {
        static_assert(std::is_same_v<T_Explicit, T>, "Type mismatch");
        static_assert(std::is_integral_v<T>);

        // Get offset and index
        size_t offset = value;

        // Using value as offset: ~10s

        //{ // ~20s
        //    // TODO this map lookup is doubling the time...

        //    letter_t letter = letter_t::get_letter<T>(value);
        //    auto it = _offset_map.find(letter);
        //    assert(it != _offset_map.end());

        //    offset = it->second;
        //}

        //{ // ~16.6s
        //    auto it = _simple_offset_map.find(value);
        //    assert(it != _simple_offset_map.end());

        //    offset = it->second;
        //}

        //{ // ~18s
        //    const offset_pair_t& op = _find_offset_pair(value);
        //    offset = op.second;
        //}

        size_t idx = position * _position_stride + offset;

        // Possibly extend table
        if (position >= _table_position_count) [[unlikely]]
        {
            /*
                The following function call must be guarded by this condition,
                otherwise performance may be HALVED. Furthermore, inlining
                choices seem to make a big difference.

                The methods "_extend_table_if_needed()" and
                "_extend_table_impl()" can be forcibly inlined with attributes
                [[clang::always_inline]] and [[gnu::always_inline]]
                but the running time may be dramatically affected:

                    Experimental data: generate all strips of size 1 through 16,
                    and hash the corresponding clobber, nogo, and elephants
                    games. Skips map lookup (uses tile value as offset).
                compiler    run time (attributes)   run time (no attributes)
                clang++     ~11s                    ~11.5
                g++         ~19.5s                  ~10.5s
            */

            [[clang::always_inline]] _extend_table_if_needed(position);
        }


        // Return value
        assert(position < _table_position_count);
        assert(idx < _table.size());

        return _table[idx];
    }


private:
    inline void _extend_table_if_needed(size_t query_position)
    {
        assert(query_position >= _table_position_count);

        size_t pos_incremented = _table_position_count; // position count + increment
        size_t pos_required = query_position; // position + 1

        THROW_ASSERT(                                               //
                     safe_add(pos_incremented, _POSITION_INCREMENT) && //
                     safe_add(pos_required, (size_t) 1)            //
                     );                                             //


        const size_t& new_positions = std::max(pos_incremented, pos_required);

        [[clang::always_inline]] _extend_table_impl(new_positions);
    }

    inline void _extend_table_impl(size_t new_position_count)
    {
        assert(new_position_count > _table_position_count);
        assert(_table_position_count * _offset_map.size() == _table.size());

        size_t old_size = _table.size();
        size_t new_size = new_position_count * _offset_map.size();
        assert(new_size > old_size);

        std::cout << "Resizing table from (position count, size): (";
        std::cout << _table_position_count << ", " << old_size << ") to (";
        std::cout << new_position_count << ", " << new_size << ")" << std::endl;

        _table.resize(new_size);
        _table_position_count = new_position_count;

        for (size_t i = old_size; i < new_size; i++)
        {
            hash_t& element = _table[i];
            element = get_random_hash_t();
        }

        assert(_table_position_count * _offset_map.size() == _table.size());
    }

    inline const offset_pair_t& _find_offset_pair(const uint64_t& value)
    {
        if (_simple_offset_vec.size() == 0) [[unlikely]]
            exit(-1);

        size_t low = 0;
        size_t high = _simple_offset_vec.size() - 1;

        while (low <= high)
        {
            size_t idx = (low + high) / 2;

            const offset_pair_t& op = _simple_offset_vec[idx];
            const uint64_t& value_i = op.first;

            if (value_i == value)
                return op;

            if (value_i < value)
            {
                // look higher
                low = idx + 1;
                continue;
            }
            else
            {
                // look lower
                if (idx == 0) [[unlikely]]
                    exit(-1);
                high = idx - 1;
            }
        }

        exit(-1);
    }

    offset_map_t _offset_map;
    size_t _position_stride;

    table_t _table;
    size_t _table_position_count;

    std::unordered_map<uint64_t, size_t> _simple_offset_map;
    std::vector<offset_pair_t> _simple_offset_vec;

    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr size_t _POSITION_INCREMENT = 64;
};




random_table* strip_table = nullptr;



//////////////////////////////////////// implement random_table
using namespace std;


constexpr hash_t MULTIPLIER = 48271;

inline hash_t hash_func(const strip& g)
{
    assert(strip_table != nullptr);
    random_table& table = *strip_table;

    hash_t hash = 0;
    const game_type_t type = g.game_type();
    assert(type > 0);

    const size_t N = g.size();
    for (size_t i = 0; i < N; i++)
    {
        const int& tile = g.at(i);
        hash ^= table.get<int>(i, tile);
    }

    return hash * (MULTIPLIER * type);
}


////////////////////////////////////////
} // namespace



void test_hashing3()
{
    rng.seed(time(0));

    random_table table({
        letter_t::get_letter<int>(EMPTY),
        letter_t::get_letter<int>(BLACK),
        letter_t::get_letter<int>(WHITE),
    });

    strip_table = &table;

    hash_func_t fn = [&](const strip& g) -> hash_t
    {
        return hash_func(g);
    };


    benchmark_hash_function(fn);
}
