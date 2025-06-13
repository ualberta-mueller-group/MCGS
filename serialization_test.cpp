/*
    Prototype of serialization. General solution. This file is self-contained.

    1. Make your type inherit from i_serializable

    2. Implement the following method and static function:
        void T::serialize(std::ostream&) const;
        static T* T::deserialize(std::istream&);

    3. In the init() function, add function call:
        make_serializable<T>();
    If you don't have this call, your code will still compile, but an assert
    will fire if your object is serialized/deserialized (in practice should
    probably throw...)

    4. Now your type is serializable:
        std::fstream& fs1 = SOME BINARY FILE
        game* g = new game_a(5, 1);
        g->serialize(fs1);
        ...
        std::fstream& fs2 = SOME (OTHER?) BINARY FILE
        i_serializable* gs = i_serializable::deserialize(fs2);
        game* g_cast = dynamic_cast<game*>(gs);

--------------------------------------------------------------------------------
    How it works

    make_serializable<T>():
        - Assuming T inherits from i_serializable, this registers it with the
            system.

        - Assigns T a serializer_id (uint32_t), its unique type ID for the
            purposes of the serialization system.

        - Creates a serializer<T> and stores it in a vector, to be indexed by
            serializer_id.

    i_type_table: Generalization of game_type_t (from MCGS code), providing a
        struct instead of just an int.

        It's used to store an object's serializer_id.

    i_serializer: Abstract type declaring function bindings to your type's
        serialize/deserialize functions.

    serializer<T>: Implements i_serializer, by calling T's functions.
        Checks that:
            1. T inherits from i_serializable
            2. T's serialize/deserialize have exactly the correct signatures

    i_serializable: Base class of serializable types.

        serialize() method uses the serializer_id of an object (from its
        type_table_t*) to use the correct serializer<T> from the aforementioned
        vector. Writes the serializer_id to the ostream, before the object data.

        deserialize() static function uses the serializer_id read from an
        istream to find the correct serializer<T>, which then reads object
        data from the istream.
*/
#include <fstream>
#include <iostream>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <cstdint>

namespace {

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
        str << byte;
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
        str >> byte;
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
//////////////////////////////////////// has_serialize<T>
template <class T, class Enable = void>
struct has_serialize
{
    static constexpr bool value = false;
};

template <class T>
struct has_serialize<T,
    std::enable_if_t<
        std::is_same_v<
            void (T::*)(std::ostream&) const,
            decltype(&T::serialize)
        >,
        void
    >
>
{
    static constexpr bool value = true;
};

template <class T>
static constexpr bool has_serialize_v = has_serialize<T>::value;

//////////////////////////////////////// has_deserialize<T>
template <class T, class Enable = void>
struct has_deserialize
{
    static constexpr bool value = false;
};

template <class T>
struct has_deserialize<T,
    std::enable_if_t<
        std::is_same_v<
            T* (*)(std::istream&),
            decltype(&T::deserialize)
        >,
        void
    >
>
{
    static constexpr bool value = true;
};

template <class T>
static constexpr bool has_deserialize_v = has_deserialize<T>::value;

////////////////////////////////////////////////// i_type_table
typedef unsigned int type_num_t; // analogue of game_type_t
typedef uint32_t serializer_id; // serializable object's unique type ID

struct type_table_t
{
    type_table_t(type_num_t num): num(num), sid(0)
    {
    }

    type_num_t num;
    serializer_id sid;
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
// Forward declarations
class i_serializer;
class i_serializable;

//////////////////////////////////////// most of serialization system's data
std::vector<std::shared_ptr<i_serializer>> __serializer_list;
serializer_id __next_serializer_id = 1;

const i_serializer* get_serializer_from_list(serializer_id sid)
{
    assert(sid > 0);
    sid--;
    assert(sid < __serializer_list.size());
    return __serializer_list[sid].get();
}

template <class T>
serializer_id __get_serializer_id_impl() // to only check assert one time
{
    static_assert(std::is_base_of_v<i_serializable, T>);
    serializer_id sid = type_table<T>()->sid;
    assert(sid > 0);
    return sid;
}

template <class T>
serializer_id get_serializer_id()
{
    static_assert(std::is_base_of_v<i_serializable, T>);
    static const serializer_id SID = __get_serializer_id_impl<T>();
    return SID;
}

//////////////////////////////////////// serialization types
class i_serializer
{
public:
    i_serializer(serializer_id id): _id(id)
    {
    }

    virtual ~i_serializer()
    {
    }

    serializer_id get_id() const
    {
        return _id;
    }

    virtual void serialize(std::ostream&, const i_serializable*) const = 0;
    virtual i_serializable* deserialize(std::istream&) const = 0;

private:
    const serializer_id _id;
};

class i_serializable: public i_type_table
{
public:
    virtual ~i_serializable()
    {
    }

    inline serializer_id get_serializer_id() const
    {
        return type_table()->sid;
    }

    inline static serializer_id extract_serializer_id_from_stream(std::istream& str)
    {
        static_assert(std::is_same_v<serializer_id, uint32_t>);
        serializer_id sid = fmt_read_u32(str);
        return sid;
    }

    inline static void insert_serializer_id_to_stream(std::ostream& str, serializer_id sid)
    {
        static_assert(std::is_same_v<serializer_id, uint32_t>);
        fmt_write_u32(str, sid);
    }

    void serialize(std::ostream& str) const
    {
        const serializer_id sid = get_serializer_id();
        assert(sid > 0);
        const i_serializer* ser = get_serializer_from_list(sid);
        assert(ser != nullptr);

        insert_serializer_id_to_stream(str, sid);
        ser->serialize(str, this);
    }

    static i_serializable* deserialize(std::istream& str)
    {
        assert(str);

        const serializer_id sid = extract_serializer_id_from_stream(str);
        assert(sid > 0);

        const i_serializer* ser = get_serializer_from_list(sid);
        assert(ser != nullptr);

        return ser->deserialize(str);
    }
};

// Created when registering a serializable object in the init() function
template <class T>
class serializer: public i_serializer
{
    static_assert(std::is_base_of_v<i_serializable, T>);
    static_assert(has_serialize_v<T>);
    static_assert(has_deserialize_v<T>);

public:
    serializer(serializer_id sid): i_serializer(sid)
    {
    }

    virtual ~serializer()
    {
    }

    void serialize(std::ostream& str, const i_serializable* obj) const override
    {
        assert(obj->get_serializer_id() == get_serializer_id<T>());

        const T* obj_casted = static_cast<const T*>(obj);
        assert(dynamic_cast<const T*>(obj) == obj_casted);

        obj_casted->serialize(str);
    }

    i_serializable* deserialize(std::istream& str) const override
    {
        i_serializable* obj = T::deserialize(str);
        assert(obj->get_serializer_id() == get_serializer_id<T>());
        return obj;
    }
};

// Register a type to enable serialization
template <class T>
void make_serializable()
{
    static_assert(std::is_base_of_v<i_serializable, T>);

    type_table_t* table = type_table<T>();
    assert(table->sid == 0);

    serializer_id sid = __next_serializer_id++;
    table->sid = sid;

    __serializer_list.emplace_back(new serializer<T>(sid));
    assert(get_serializer_from_list(sid)->get_id() == sid);
}

////////////////////////////////////////////////// games
class game: public i_serializable
{
public:
    virtual ~game()
    {
    }
};

class game_a: public game
{
public:
    game_a(int val): val(val)
    {
    }

    void serialize(std::ostream& str) const
    {
        fmt_write_i32(str, val);
    }

    static game_a* deserialize(std::istream& str)
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

    void serialize(std::ostream& os) const
    {
        fmt_write_i32(os, x);
        fmt_write_i32(os, y);
        fmt_write_i32(os, z);
    }

    static game_b* deserialize(std::istream& is)
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

} // namespace


//////////////////////////////////////////////////

using namespace std;

void init()
{
    make_serializable<game_a>();
    make_serializable<game_b>();
}

int main()
{
    init();

    game* ga = new game_a(134);
    game* gb = new game_b(4, 0, 3);

    std::fstream fs1("data.bin",
                    std::fstream::trunc  |
                    std::fstream::in     |
                    std::fstream::out    |
                    std::fstream::binary
                );

    assert(fs1.is_open());

    ga->serialize(fs1);
    gb->serialize(fs1);
    fs1.close();


    std::fstream fs2("data.bin",
                     std::fstream::in    |
                     std::fstream::binary
                    );
    assert(fs2.is_open());

    i_serializable* g1s = i_serializable::deserialize(fs2);
    i_serializable* g2s = i_serializable::deserialize(fs2);


    game_a* g1 = dynamic_cast<game_a*>(g1s);
    assert(g1 != nullptr);

    game_b* g2 = dynamic_cast<game_b*>(g2s);
    assert(g2 != nullptr);


    cout << "Got back: " << *g1 << " " << *g2 << endl;


    fs2.close();

    return 0;
}
