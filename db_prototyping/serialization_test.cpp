/*
    Prototype of serialization. Handles both polymorphic and non-polymorphic
    types. This file is self contained.

    Reading and writing of primitive data types is done through
    fmt_write/fmt_read functions:
        fmt_write_u32(some_ostream, some_uint32_t);
        int64_t some_int = fmt_read_i64(some_istream);

    Reading and writing of other objects is done through serializers:
        serializer<vector<game*>>::save(some_ostream, some_games_vec);
        vector<game*> games = serializer<vector<game*>>::load(some_istream);

    Polymorphic types, such as games, need to implement some functions:

    1. Inherit from class "serializable" (game already does this)

    2. For a type, clobber, implement:
        void clobber::save_impl(std::ostream&) const;
        static clobber* clobber::load_impl(std::istream&);

    3. In the init() function, register your type:
        make_serializable<clobber>();

    Now your type is serializable:
        serializer<clobber*>::save(some_ostream, some_clobber_ptr);
        clobber* ptr = serializer<clobber*>::load(some_istream);

        OR

        serializer<game*>::save(some_ostream, some_clobber_ptr);
        game* ptr = serializer<game*>::load(some_istream);

        OR

        serializer<serializable*>::save(some_ostream, some_clobber_ptr);
        serializable* ptr = serializer<serializable*>::load(some_istream);

        The serializer template is defined for all T*, where T is derived from
        "serializable". You may need to write serializers for types which
        don't have them

*/
#include <climits>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <set>
#include <cassert>
#include <cstdint>
#include <chrono>
#include <ratio>

namespace {

////////////////////////////////////////////////// timing

uint64_t ms_since_epoch()
{
    using namespace std::chrono;

    milliseconds t =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    return t.count();
}


////////////////////////////////////////////////// random
static_assert(sizeof(unsigned long long) >= sizeof(uint64_t));


std::mt19937_64 rng;

std::uniform_int_distribution<unsigned long long> random_dist(
    0, std::numeric_limits<unsigned long long>::max());

uint64_t __rng_seed = 0;

uint64_t random_u64()
{
    return (uint64_t) random_dist(rng);
}

uint32_t random_u32()
{
    return (uint32_t) random_dist(rng);
}

uint16_t random_u16()
{
    return (uint16_t) random_dist(rng);
}

uint8_t random_u8()
{
    return (uint8_t) random_dist(rng);
}

////////////////////////////////////////////////// misc types
typedef uint64_t hash_t;

//////////////////////////////////////// element_t
typedef std::pair<hash_t, uint32_t> element_t;

inline bool operator<(const element_t& elem1, const element_t& elem2)
{
    return elem1.first < elem2.first;
}

std::ostream& operator<<(std::ostream& os, const element_t& elem)
{
    os << '{' << elem.first << '}';
    return os;
}


////////////////////////////////////////////////// std::vector printing
template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
    const size_t size = vec.size();

    os << "[";
    for (size_t i = 0; i < size; i++)
    {
        os << vec[i];

        if (i + 1 < size)
            os << ", ";
    }
    os << "]";

    return os;
}

// dereference pointers in vector
template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T*>& vec)
{
    const size_t size = vec.size();

    os << "[";
    for (size_t i = 0; i < size; i++)
    {
        os << *vec[i];

        if (i + 1 < size)
            os << ", ";
    }
    os << "]";

    return os;
}

template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<std::shared_ptr<T>>& vec)
{
    const size_t size = vec.size();

    os << "[";
    for (size_t i = 0; i < size; i++)
    {
        os << *vec[i];

        if (i + 1 < size)
            os << ", ";
    }
    os << "]";

    return os;
}

template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<std::unique_ptr<T>>& vec)
{
    const size_t size = vec.size();

    os << "[";
    for (size_t i = 0; i < size; i++)
    {
        os << *vec[i];

        if (i + 1 < size)
            os << ", ";
    }
    os << "]";

    return os;
}

////////////////////////////////////////////////// format write/read
//////////////////////////////////////// implementations
template <class T>
inline void __fmt_write(std::ostream& str, const T& val)
{
    static_assert(std::is_integral_v<T>);

    using T_Unsigned = std::make_unsigned_t<T>;
    const T_Unsigned& val_uns = reinterpret_cast<const T_Unsigned&>(val);

    constexpr size_t N_BYTES = sizeof(T);

    for (size_t i = 0; i < N_BYTES; i++)
    {
        const uint8_t byte = (uint8_t) (val_uns >> (i * 8));
        assert(str);
        str.write(reinterpret_cast<const char*>(&byte), 1);
    }
    assert(str);
}

template <class T>
inline T __fmt_read(std::istream& str)
{
    static_assert(std::is_integral_v<T>);

    constexpr size_t N_BYTES = sizeof(T);

    T val(0);

    uint8_t byte;
    T byte_longer;

    for (size_t i = 0; i < N_BYTES; i++)
    {
        assert(str);
        str.read(reinterpret_cast<char*>(&byte), 1);
        byte_longer = byte;
        val |= (byte_longer << (i * 8));
    }
    assert(str);

    return val;
}

//////////////////////////////////////// fmt_write functions
inline void fmt_write_u8(std::ostream& str, const uint8_t& val)
{
    __fmt_write<uint8_t>(str, val);
}

inline void fmt_write_u16(std::ostream& str, const uint16_t& val)
{
    __fmt_write<uint16_t>(str, val);
}

inline void fmt_write_u32(std::ostream& str, const uint32_t& val)
{
    __fmt_write<uint32_t>(str, val);
}

inline void fmt_write_u64(std::ostream& str, const uint64_t& val)
{
    __fmt_write<uint64_t>(str, val);
}

inline void fmt_write_i8(std::ostream& str, const int8_t& val)
{
    __fmt_write<int8_t>(str, val);
}

inline void fmt_write_i16(std::ostream& str, const int16_t& val)
{
    __fmt_write<int16_t>(str, val);
}

inline void fmt_write_i32(std::ostream& str, const int32_t& val)
{
    __fmt_write<int32_t>(str, val);
}

inline void fmt_write_i64(std::ostream& str, const int64_t& val)
{
    __fmt_write<int64_t>(str, val);
}

//////////////////////////////////////// fmt_read functions
inline uint8_t fmt_read_u8(std::istream& str)
{
    return __fmt_read<uint8_t>(str);
}

inline uint16_t fmt_read_u16(std::istream& str)
{
    return __fmt_read<uint16_t>(str);
}

inline uint32_t fmt_read_u32(std::istream& str)
{
    return __fmt_read<uint32_t>(str);
}

inline uint64_t fmt_read_u64(std::istream& str)
{
    return __fmt_read<uint64_t>(str);
}

inline int8_t fmt_read_i8(std::istream& str)
{
    return __fmt_read<int8_t>(str);
}

inline int16_t fmt_read_i16(std::istream& str)
{
    return __fmt_read<int16_t>(str);
}

inline int32_t fmt_read_i32(std::istream& str)
{
    return __fmt_read<int32_t>(str);
}

inline int64_t fmt_read_i64(std::istream& str)
{
    return __fmt_read<int64_t>(str);
}

////////////////////////////////////////////////// serialization type traits
// Forward declarations
class i_dynamic_serializer;
class serializable;

//////////////////////////////////////// has_serialize_impl<T>
template <class T, class Enable = void>
struct has_save_impl
{
    static constexpr bool value = false;
};

template <class T>
struct has_save_impl<T,
    std::enable_if_t<
        std::is_same_v<
            void (T::*)(std::ostream&) const,
            decltype(&T::save_impl)
        >,
        void
    >
>
{
    static constexpr bool value = true;
};

template <class T>
static constexpr bool has_save_impl_v = has_save_impl<T>::value;

//////////////////////////////////////// has_deserialize_impl<T>
template <class T, class Enable = void>
struct has_load_impl
{
    static constexpr bool value = false;
};

template <class T>
struct has_load_impl<T,
    std::enable_if_t<
        std::is_same_v<
            T* (*)(std::istream&),
            decltype(&T::load_impl)
        >,
        void
    >
>
{
    static constexpr bool value = true;
};

template <class T>
static constexpr bool has_load_impl_v = has_load_impl<T>::value;

////////////////////////////////////////////////// i_type_table
typedef unsigned int type_num_t; // analogue of game_type_t

// polymorphic serializable object's unique type ID
typedef uint32_t dynamic_serializer_id;


struct type_table_t
{
    type_table_t(type_num_t num): num(num), sid(0)
    {
    }

    type_num_t num;
    dynamic_serializer_id sid;
};

type_num_t __next_type_num = 1;
std::unordered_map<std::type_index, std::shared_ptr<type_table_t>> __type_table_map;

inline type_table_t* __new_type_table()
{
    type_table_t* table = new type_table_t(__next_type_num++);
    return table;
}

type_table_t* __get_type_table(const std::type_info& tinfo)
{
    const std::type_index tidx(tinfo);

    auto it = __type_table_map.find(tidx);
    if (it != __type_table_map.end())
        return it->second.get();

    type_table_t* table = __new_type_table();
    __type_table_map.insert({tidx, std::shared_ptr<type_table_t>(table)});

    return table;
}

class i_type_table // analogue of i_game_type
{
public:
    i_type_table(): _table(nullptr)
    {
    }

    virtual ~i_type_table()
    {
    }

    type_table_t* type_table() const
    {
        if (_table != nullptr)
        {
            assert(__get_type_table(typeid(*this)) == _table);
            return _table;
        }

        _table = __get_type_table(typeid(*this));
        return _table;
    }

private:
    mutable type_table_t* _table;
};

template <class T>
type_table_t* type_table()
{
    static_assert(std::is_base_of_v<i_type_table, T>);
    static type_table_t* table = __get_type_table(typeid(T));
    return table;
}

////////////////////////////////////////////////// serialization

//////////////////////////////////////// most of serialization system's data
std::vector<std::shared_ptr<i_dynamic_serializer>> __dynamic_serializers;
dynamic_serializer_id __next_dynamic_serializer_id = 1;

const i_dynamic_serializer* get_dynamic_serializer(dynamic_serializer_id sid)
{
    assert(sid > 0);
    sid--;
    assert(sid < __dynamic_serializers.size());
    return __dynamic_serializers[sid].get();
}

template <class T>
dynamic_serializer_id __get_dynamic_serializer_id_impl() // to only check assert one time
{
    static_assert(std::is_base_of_v<serializable, T>);
    dynamic_serializer_id sid = type_table<T>()->sid;
    assert(sid > 0);
    return sid;
}

template <class T>
dynamic_serializer_id get_dynamic_serializer_id()
{
    static_assert(std::is_base_of_v<serializable, T>);
    static const dynamic_serializer_id SID = __get_dynamic_serializer_id_impl<T>();
    return SID;
}

//////////////////////////////////////// serialization types
class i_dynamic_serializer
{
public:
    i_dynamic_serializer(dynamic_serializer_id id): _id(id)
    {
    }

    virtual ~i_dynamic_serializer()
    {
    }

    dynamic_serializer_id get_id() const
    {
        return _id;
    }

    virtual void save(std::ostream&, const serializable*) const = 0;
    virtual serializable* load(std::istream&) const = 0;

private:
    const dynamic_serializer_id _id;
};

class serializable: public i_type_table
{
public:
    virtual ~serializable()
    {
    }

    inline dynamic_serializer_id get_dynamic_serializer_id() const
    {
        return type_table()->sid;
    }

    inline static dynamic_serializer_id extract_dynamic_serializer_id_from_stream(std::istream& str)
    {
        static_assert(std::is_same_v<dynamic_serializer_id, uint32_t>);
        dynamic_serializer_id sid = fmt_read_u32(str);
        return sid;
    }

    inline static void insert_dynamic_serializer_id_to_stream(std::ostream& str, dynamic_serializer_id sid)
    {
        static_assert(std::is_same_v<dynamic_serializer_id, uint32_t>);
        fmt_write_u32(str, sid);
    }

    void __save(std::ostream& str) const
    {
        const dynamic_serializer_id sid = get_dynamic_serializer_id();
        assert(sid > 0);
        const i_dynamic_serializer* ser = get_dynamic_serializer(sid);
        assert(ser != nullptr);

        insert_dynamic_serializer_id_to_stream(str, sid);
        ser->save(str, this);
    }

    static serializable* __load(std::istream& str)
    {
        assert(str);

        const dynamic_serializer_id sid = extract_dynamic_serializer_id_from_stream(str);
        assert(sid > 0);

        const i_dynamic_serializer* ser = get_dynamic_serializer(sid);
        assert(ser != nullptr);

        return ser->load(str);
    }
};

// Created when registering a serializable object in the init() function
template <class T>
class dynamic_serializer: public i_dynamic_serializer
{
    static_assert(std::is_base_of_v<serializable, T>);
    static_assert(has_save_impl_v<T>);
    static_assert(has_load_impl_v<T>);

public:
    dynamic_serializer(dynamic_serializer_id sid): i_dynamic_serializer(sid)
    {
    }

    virtual ~dynamic_serializer()
    {
    }

    void save(std::ostream& str, const serializable* obj) const override
    {
        assert(obj->get_dynamic_serializer_id() == get_dynamic_serializer_id<T>());

        const T* obj_casted = static_cast<const T*>(obj);
        assert(dynamic_cast<const T*>(obj) == obj_casted);

        obj_casted->save_impl(str);
    }

    serializable* load(std::istream& str) const override
    {
        serializable* obj = T::load_impl(str);
        assert(obj->get_dynamic_serializer_id() == get_dynamic_serializer_id<T>());
        return obj;
    }
};

// Register a type to enable serialization
template <class T>
void make_serializable()
{
    static_assert(std::is_base_of_v<serializable, T>);

    type_table_t* table = type_table<T>();
    assert(table->sid == 0);

    dynamic_serializer_id sid = __next_dynamic_serializer_id++;
    table->sid = sid;

    __dynamic_serializers.emplace_back(new dynamic_serializer<T>(sid));
    assert(get_dynamic_serializer(sid)->get_id() == sid);
}

////////////////////////////////////////////////// game
class game: public serializable
{
public:
    virtual ~game()
    {
    }

    virtual void print(std::ostream& os) const = 0;
};

std::ostream& operator<<(std::ostream& os, const game& g)
{
    g.print(os);
    return os;
}

////////////////////////////////////////////////// manual serialization
template <class T, class Enable = void>
struct serializer
{
    static_assert(false, "Not implemented!");
    static void save(std::ostream& os, const T& val);
    static T load(std::istream& is);
};

//////////////////////////////////////// pointers

// Only matches pointer types derived from serializable
template <class T>
struct serializer<T*,
    std::enable_if_t<
        std::is_base_of_v<serializable, T>,
        void
    >
>
{
    static void save(std::ostream& os, const serializable* ptr)
    {
        ptr->__save(os);
    }

    static T* load(std::istream& is)
    {
        serializable* ptr = serializable::__load(is);
        assert(dynamic_cast<T*>(ptr) != nullptr);
        return static_cast<T*>(ptr);
    }
};

//////////////////////////////////////// shared_ptr<T>
template <class T>
struct serializer<std::shared_ptr<T>>
{
    static void save(std::ostream& os, const std::shared_ptr<T>& ptr)
    {
        serializer<T*>::save(os, ptr.get());
    }

    static std::shared_ptr<T> load(std::istream& is)
    {
        T* ptr = serializer<T*>::load(is);
        return std::shared_ptr<T>(ptr);
    }
};

//////////////////////////////////////// unique_ptr<T>
template <class T>
struct serializer<std::unique_ptr<T>>
{
    static void save(std::ostream& os, const std::unique_ptr<T>& ptr)
    {
        serializer<T*>::save(os, ptr.get());
    }

    static std::unique_ptr<T> load(std::istream& is)
    {
        T* ptr = serializer<T*>::load(is);
        return std::unique_ptr<T>(ptr);
    }
};

//////////////////////////////////////// std::string
template <>
struct serializer<std::string>
{
    static void save(std::ostream& os, const std::string& str)
    {
        const size_t size = str.size();
        fmt_write_u32(os, size);

        for (size_t i = 0; i < size; i++)
            fmt_write_i8(os, str[i]);
    }

    static std::string load(std::istream& is)
    {
        std::string str;

        const size_t size = fmt_read_u32(is);
        str.reserve(size);

        for (size_t i = 0; i < size; i++)
            str.push_back(fmt_read_i8(is));

        return str;
    }
};

//////////////////////////////////////// std::vector<T>
template <class T>
struct serializer<std::vector<T>>
{
    static void save(std::ostream& os, const std::vector<T>& vec)
    {
        const size_t size = vec.size();
        fmt_write_u32(os, size);

        for (size_t i = 0; i < size; i++)
            serializer<T>::save(os, vec[i]);
    }

    static std::vector<T> load(std::istream& is)
    {
        std::vector<T> vec;

        const size_t size = fmt_read_u32(is);
        vec.reserve(size);

        for (size_t i = 0; i < size; i++)
            vec.emplace_back(serializer<T>::load(is));

        return vec;
    }
};

////////////////////////////////////////////////// games
class game_a: public game
{
public:
    game_a(int val): val(val)
    {
    }

    void print(std::ostream& os) const override
    {
        os << "game_a: " << val;
    }

    void save_impl(std::ostream& str) const
    {
        fmt_write_i32(str, val);
    }

    static game_a* load_impl(std::istream& str)
    {
        int32_t val = fmt_read_i32(str);
        return new game_a(val);
    }

    int val;
};

std::ostream& operator<<(std::ostream& os, const game_a& g)
{
    os << "<" << g.val << ">";
    return os;
}

class game_b: public game
{
public:

    game_b(int x, int y, int z): x(x), y(y), z(z)
    {
    }

    void print(std::ostream& os) const override
    {
        os << "game_b: [";
        os << x << " ";
        os << y << " ";
        os << z << "]";
    }

    void save_impl(std::ostream& os) const
    {
        fmt_write_i32(os, x);
        fmt_write_i32(os, y);
        fmt_write_i32(os, z);
    }

    static game_b* load_impl(std::istream& is)
    {
        int x = fmt_read_i32(is);
        int y = fmt_read_i32(is);
        int z = fmt_read_i32(is);

        return new game_b(x, y, z);
    }

    int x, y, z;
};

std::ostream& operator<<(std::ostream& os, const game_b& g)
{
    os << '[';
    os << g.x << ' ';
    os << g.y << ' ';
    os << g.z;
    os << ']';

    return os;
}


class game_c: public game
{
public:

    game_c(const std::string& board): _board(board)
    {
    }

    virtual ~game_c()
    {
    }

    void print(std::ostream& os) const override
    {
        os << "game_c: \"" << _board << "\"";
    }

    void save_impl(std::ostream& os) const
    {
        serializer<std::string>::save(os, _board);
    }

    static game_c* load_impl(std::istream& is)
    {
        std::string board = serializer<std::string>::load(is);
        return new game_c(board);
    }

private:

    friend std::ostream& operator<<(std::ostream& os, const game_c& g);
    std::string _board;
};

std::ostream& operator<<(std::ostream& os, const game_c& g)
{
    os << "game_c: \"";
    os << g._board;
    os << "\"";

    return os;
}

} // namespace

//////////////////////////////////////////////////

void init()
{
    make_serializable<game_a>();
    make_serializable<game_b>();
    make_serializable<game_c>();
}

std::fstream open_file(const std::string& filename, bool trunc)
{
    std::fstream::openmode om =
        std::fstream::binary |
        std::fstream::in     |
        std::fstream::out;

    if (trunc)
        om |= std::fstream::trunc;

    std::fstream fs(filename, om);

    assert(fs.is_open());
    return fs;
}


/*
int main()
{
    init();

    // Write
    {
        vector<unique_ptr<game>> games;

        games.push_back(unique_ptr<game>(new game_a(5)));
        games.push_back(unique_ptr<game>(new game_b(1, 6, 2)));
        games.push_back(unique_ptr<game>(new game_c("This is some text")));

        std::fstream fs = open_file("data.bin", true);

        serializer<vector<unique_ptr<game>>>::save(fs, games);

        fs.close();
    }

    // Read
    {
        std::fstream fs = open_file("data.bin", false);

        vector<unique_ptr<game>> games = serializer<vector<unique_ptr<game>>>::load(fs);

        cout << games << endl;

        fs.close();
    }

}
*/


namespace {


////////////////////////////////////////////////// helper functions

void check_no_collisions(const std::vector<element_t>& vec)
{
    std::unordered_set<hash_t> hashes;

    const size_t size = vec.size();
    for (size_t i = 0; i < size; i++)
    {
        auto it = hashes.insert(vec[i].first);
        if (!it.second)
            throw std::logic_error("Hash collision");
    }
}

inline bool binary_search(const std::vector<element_t>& bucket, const hash_t& query_hash, hash_t& found_idx)
{
    found_idx = 0;

    const size_t N = bucket.size();

    if (N == 0)
        return false;

    size_t low = 0;
    size_t high = N - 1;

    while (low <= high)
    {
        const size_t idx = (low + high) / 2;
        found_idx = low;

        const hash_t elem_hash = bucket[idx].first;

        if (elem_hash < query_hash)
        {
            low = idx + 1;
            found_idx = low;
        }
        else if (elem_hash > query_hash)
        {
            if (idx == 0)
                return false;

            high = idx - 1;
        } else
        {
            found_idx = idx;
            return true;
        }
    }

    return false;
}

template <class T>
bool is_sorted(const std::vector<T>& vec)
{
    const size_t N = vec.size();

    if (N < 2)
        return true;

    for (size_t i = 0; i < N - 1; i++)
        if (!(vec[i] < vec[i + 1]))
            return false;

    return true;
}

class index
{
public:
    index(size_t n_bits)
        : _sum(0),
          _n_bits(n_bits),
          _bit_mask(hash_t(-1) << (sizeof(hash_t) * CHAR_BIT - n_bits)),
          _n_buckets(1 << n_bits)
    {
        assert(0 < n_bits && n_bits < sizeof(hash_t) * CHAR_BIT);

        _buckets.resize(_n_buckets);

        for (size_t i = 0; i < _n_buckets; i++)
            _buckets[i].reserve(21);
    }

    void insert(const element_t& elem)
    {
        const hash_t hash = elem.first;
        const hash_t bucket_idx = _hash_to_bucket_idx(hash);
        std::vector<element_t>& bucket = _buckets[bucket_idx];

        size_t idx = 0;
        bool found = binary_search(bucket, hash, idx);

        if (!found)
            _sum++;

        bucket.insert(bucket.begin() + idx, elem);

        //std::cout << bucket << std::endl;
        //assert(is_sorted(bucket));
    }

    inline int get_sum() const
    {
        return _sum;
    }

private:

    inline hash_t _hash_to_bucket_idx(const hash_t& hash) const
    {
        return hash >> ((sizeof(hash_t) * CHAR_BIT) - _n_bits);
    }

    int _sum;

    const size_t _n_bits;
    const hash_t _bit_mask;
    const size_t _n_buckets;
    std::vector<std::vector<element_t>> _buckets;
};

int test_unordered_map(const std::vector<element_t>& elements)
{
    std::unordered_map<hash_t, uint32_t> m;

    const size_t N = elements.size();

    m.reserve(N);

    int sum = 0;

    for (size_t i = 0; i < N; i++)
    {
        auto it = m.insert(elements[i]);
        sum += it.second;
    }

    return sum;
}

int test_index(const std::vector<element_t>& elements)
{
    index m(22);

    const size_t N = elements.size();

    for (size_t i = 0; i < N; i++)
        m.insert(elements[i]);

    return m.get_sum();
}
} // namespace


using namespace std;

int main()
{
    rng.seed(std::time(0));

    const uint64_t n_items = 80000000;

    vector<element_t> elements;
    elements.reserve(n_items);

    for (size_t i = 0; i < n_items; i++)
        elements.emplace_back(random_u64(), random_u32());

    //check_no_collisions(elements);

    int s = 0;
    //////////////////////////////////////////////////
    const uint64_t start = ms_since_epoch();

    s = test_unordered_map(elements);
    //s = test_index(elements);

    const uint64_t end = ms_since_epoch();
    /////////////////////////////////////////////////

    cout << s << endl;

    cout << "TIME: " << (end - start) << " ms" << endl;

    return 0;
}
