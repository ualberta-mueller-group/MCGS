/*
    Types and functions used for serialization. Used to enforce
    machine-indpendent binary format on disk. Prevents incompatibility from:
        1. Variable widths differing across machines
        2. Endianness differing across machines

    ibuffer/obuffer classes are wrappers of std::fstream,

    TODO: Condition 1 is still only loosely enforced, because it's not possible
    to distinguish between fixed-width integer types, and the equivalent "C
    types", i.e. int32_t and int.

*/
#pragma once
#include <climits>
#include <limits>
#include <string>
#include <type_traits>
#include <cassert>
#include <fstream>
#include <cstdint>
#include <iostream>

#include "throw_assert.h"

/*
    TODO: assert or THROW_ASSERT for this file?

    TODO: both ibuffer and obuffer give templates for generic ints,
    making it easier to read/write non-fixed width integers like int (this is
    a problem...)

    This template method is useful for the "serialize" template struct, and
    still at least enforces consistent endianness

*/

////////////////////////////////////////////////// interface i_ibuffer
class i_ibuffer
{
public:
    virtual ~i_ibuffer() {}

    virtual uint8_t read_u8() = 0;
    uint16_t read_u16();
    uint32_t read_u32();
    uint64_t read_u64();

    int8_t read_i8();
    int16_t read_i16();
    int32_t read_i32();
    int64_t read_i64();

    bool read_bool();

    template <class Enum_T>
    Enum_T read_enum();

    template <class T> // NOLINTNEXTLINE(readability-identifier-naming)
    T __read();
};

////////////////////////////////////////////////// interface i_obuffer
class i_obuffer
{
public:
    virtual ~i_obuffer() {}

    virtual void write_u8(const uint8_t& val) = 0;
    void write_u16(const uint16_t& val);
    void write_u32(const uint32_t& val);
    void write_u64(const uint64_t& val);

    void write_i8(const int8_t& val);
    void write_i16(const int16_t& val);
    void write_i32(const int32_t& val);
    void write_i64(const int64_t& val);

    void write_bool(const bool& val);

    template <class Enum_T>
    void write_enum(const Enum_T& val);

    template <class T> // NOLINTNEXTLINE(readability-identifier-naming)
    void __write(const T& val);
};

////////////////////////////////////////////////// class file_ibuffer
class file_ibuffer: public i_ibuffer
{
public:
    file_ibuffer(const std::string& file_name);
    void close();

    uint8_t read_u8() override;

private:
    static constexpr std::ifstream::openmode OPEN_MODE = std::ifstream::binary;

    std::ifstream _fs;
};

////////////////////////////////////////////////// class file_obuffer
class file_obuffer: public i_obuffer
{
public:
    file_obuffer(const std::string& file_name);
    inline void close();

    void write_u8(const uint8_t& val) override;

private:
    static constexpr std::ofstream::openmode OPEN_MODE = //
        std::ofstream::binary |                          //
        std::ofstream::trunc;                            //

    std::ofstream _fs;
};

////////////////////////////////////////////////// fmt_read/fmt_write templates
template <class T> // NOLINTNEXTLINE(readability-identifier-naming)
T __fmt_read(i_ibuffer& is)
{
    static_assert(CHAR_BIT == 8); // 8 bits per byte
    static_assert(std::is_integral_v<T>);

    constexpr unsigned int SIZE = sizeof(T);

    T val(0);
    uint8_t byte;

    for (unsigned int i = 0; i < SIZE; i++)
    {
        byte = is.read_u8();

        const T byte_longer = byte;
        val |= (byte_longer << (i * 8));
    }

    return val;
}

// Pass T by value, not reference (avoid size mismatch)
template <class T> // NOLINTNEXTLINE(readability-identifier-naming)
void __fmt_write(i_obuffer& os, T val)
{
    static_assert(CHAR_BIT == 8); // 8 bits per byte
    static_assert(std::is_integral_v<T>);

    // NOLINTNEXTLINE(readability-identifier-naming)
    using T_Unsigned = std::make_unsigned_t<T>;
    const T_Unsigned& val_uns = reinterpret_cast<const T_Unsigned&>(val);

    constexpr unsigned int SIZE = sizeof(T);

    for (unsigned int i = 0; i < SIZE; i++)
    {
        const uint8_t byte = (uint8_t) (val_uns >> (i * 8));
        os.write_u8(byte);
    }
}

//////////////////////////////////////// i_ibuffer methods
inline bool i_ibuffer::read_bool()
{
    const uint8_t val = read_u8();
    return static_cast<bool>(val);
}

template <class Enum_T>
inline Enum_T i_ibuffer::read_enum()
{
    static_assert(std::is_enum_v<Enum_T>);
    const uint8_t value = read_u8();
    return static_cast<Enum_T>(value);
}

template <class T>
inline T i_ibuffer::__read()
{
    static_assert(std::is_integral_v<T>);
    return __fmt_read<T>(*this);
}

//////////////////////////////////////// i_obuffer methods
inline void i_obuffer::write_bool(const bool& val)
{
    const uint8_t val_casted = static_cast<bool>(val);
    write_u8(val_casted);
}

template <class Enum_T>
void i_obuffer::write_enum(const Enum_T& val)
{
    static_assert(std::is_enum_v<Enum_T>);

    THROW_ASSERT(std::numeric_limits<uint8_t>::min() <= val && //
                 val <= std::numeric_limits<uint8_t>::max()    //
    );

    const uint8_t val_casted = static_cast<uint8_t>(val);
    write_u8(val_casted);
}

template <class T>
inline void i_obuffer::__write(const T& val)
{
    static_assert(std::is_integral_v<T>);
    return __fmt_write<T>(*this, val);
}

//////////////////////////////////////// file_ibuffer methods
inline void file_ibuffer::close()
{
    assert(_fs.is_open());
    _fs.close();
    assert(!_fs.is_open());
}

//////////////////////////////////////// file_obuffer methods
inline file_obuffer::file_obuffer(const std::string& file_name)
    : _fs(file_name, OPEN_MODE)
{
    assert(_fs.is_open());
}

inline void file_obuffer::close()
{
    assert(_fs.is_open());
    _fs.close();
    assert(!_fs.is_open());
}

