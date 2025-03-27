#include "hashing2.h"
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


/*
    Dynamically expanding random table with an alphabet

    How to handle different data types? Define each letter as a pair
    (type, value)

    map from letter --> subtable offset
*/

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

    template <class T>
    static letter_t get_letter(const T& value)
    {
        static_assert(sizeof(uint64_t) >= sizeof(T));

        letter_t letter(get_int_type<T>(), static_cast<uint64_t>(value));
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


class random_table
{
public:
    typedef std::vector<hash_t> subtable_t;
    typedef std::unordered_map<letter_t, subtable_t, letter_t::hash> table_map_t;
    //typedef std::map<letter_t, subtable_t> table_map_t;

    random_table()
    {
    }

    template <class T_Explicit, class T>
    hash_t get(size_t position, const T& value)
    {
        static_assert(std::is_same_v<T_Explicit, T>, "Type mismatch");
        static_assert(std::is_integral_v<T>);

        // Get letter_t
        letter_t letter = letter_t::get_letter<T>(value);

        // Get subtable
        subtable_t* table = _get_subtable(letter);
        THROW_ASSERT(table != nullptr);

        // Possibly extend subtable
        {
            const size_t table_size = table->size();

            if (position >= table_size)
            {
                size_t size_incremented = table_size; // table size + increment
                size_t size_required = position; // position + 1

                THROW_ASSERT(                                               //
                             safe_add(size_incremented, _SIZE_INCREMENT) && //
                             safe_add(size_required, (size_t) 1)            //
                             );                                             //


                const size_t& new_size = std::max(size_incremented, size_required);

                _extend_table(*table, new_size);
            }
        }

        // Return value
        assert(position < table->size());
        return table->at(position);
    }

    template <class T_Explicit, class T>
    void add_letter(const T& value)
    {
        static_assert(std::is_same_v<T_Explicit, T>, "Type mismatch");
        static_assert(std::is_integral_v<T>);

        // Get letter_t
        letter_t letter = letter_t::get_letter<T>(value);

        // Make subtable
        subtable_t& table = _add_subtable(letter);
        assert(table.size() == 0);

        // Extend subtable
        _extend_table(table, _SIZE_INCREMENT);
    }


private:
    subtable_t* _get_subtable(const letter_t& letter)
    {
        auto it = _table_map.find(letter);

        if (it == _table_map.end())
            return nullptr;

        return &(it->second);
    }

    subtable_t& _add_subtable(const letter_t& letter)
    {
        const bool exists = _table_map.find(letter) != _table_map.end();
        THROW_ASSERT(!exists);

        return _table_map[letter];
    }

    void _extend_table(subtable_t& table, size_t new_size)
    {

        const size_t old_size = table.size();
        assert(old_size <= new_size);

        table.resize(new_size);
        std::cout << "Resizing table from " << old_size << " to " << new_size << std::endl;

        for (size_t i = old_size; i < new_size; i++)
        {
            hash_t& element = table[i];
            element = get_random_hash_t();
        }
    }

    table_map_t _table_map;

    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr size_t _SIZE_INCREMENT = 64;
};





random_table strip_table;
random_table game_table;

random_table clobber_table;
random_table nogo_table;
random_table elephants_table;

random_table& get_table(game_type_t type)
{
    static std::vector<random_table*> tables =
    {
        &clobber_table,
        &nogo_table,
        &elephants_table,
    };

    return *tables[type - 1];
}

//////////////////////////////////////// implement random_table
using namespace std;


constexpr hash_t MULTIPLIER = 48271;

inline hash_t hash_func(const strip& g)
{
    hash_t hash = 0;
    game_type_t type = g.game_type();
    hash_t type_multiplier = game_table.get<game_type_t>(0, type);

    random_table& table = get_table(type);

    assert(type > 0);

    const size_t N = g.size();
    for (size_t i = 0; i < N; i++)
    {
        const int& val = g.at(i);
        hash ^= table.get<int>(i, val);
    }

    return hash;
}


////////////////////////////////////////
} // namespace



void test_hashing2()
{
    rng.seed(time(0));
    
    auto init_table = [](random_table& table) -> void
    {
        table.add_letter<int>(EMPTY);
        table.add_letter<int>(BLACK);
        table.add_letter<int>(WHITE);
    };

    init_table(clobber_table);
    init_table(nogo_table);
    init_table(elephants_table);

    game_table.add_letter<game_type_t>(game_type<clobber_1xn>());
    game_table.add_letter<game_type_t>(game_type<nogo_1xn>());
    game_table.add_letter<game_type_t>(game_type<elephants>());

    //hash_func_t fn = [&](const strip& g) -> hash_t
    //{
    //    return hash_func(g);
    //};

    //hash_func_t fn = [](const strip& g) -> hash_t
    //{
    //    return get_random_hash_t();
    //};

    const size_t SIMPLE_TABLE_N = 64 * 3;
    vector<hash_t> simple_table(SIMPLE_TABLE_N);
    THROW_ASSERT(simple_table.size() == SIMPLE_TABLE_N);
    for (size_t i = 0; i < SIMPLE_TABLE_N; i++)
        simple_table[i] = get_random_hash_t();

    hash_func_t fn = [&](const strip& g) -> hash_t
    {
        hash_t hash = 0;
        const game_type_t type = g.game_type();

        const size_t N = g.size();
        for (size_t i = 0; i < N; i++)
        {
            const int& tile = g.at(i);
            hash ^= simple_table[3 * i + tile];
        }

        hash *= (MULTIPLIER * type);

        return hash;
    };

    benchmark_hash_function(fn);
}
