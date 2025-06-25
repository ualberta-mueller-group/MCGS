/*
    TODO: assert or THROW_ASSERT for this file?

    TODO: both ibuffer and obuffer give templates for generic ints,
    making it easier to read/write non-fixed width integers like int (this is
    a problem...)

    This template method is useful for the "serialize" template struct, and
    still at least enforces consistent endianness

*/
#pragma once
#include <climits>
#include <string>
#include <type_traits>
#include <cassert>
#include <fstream>
#include <cstdint>
#include <iostream>

////////////////////////////////////////////////// fmt_write/fmt_read

// Pass T by value, not reference (avoid size mismatch)
template <class T> // NOLINTNEXTLINE(readability-identifier-naming)
inline void __fmt_write(std::ostream& os, T val)
{
    static_assert(CHAR_BIT == 8); // 8 bits per byte
    static_assert(std::is_integral_v<T>);

    // NOLINTNEXTLINE(readability-identifier-naming)
    using T_Unsigned = std::make_unsigned_t<T>;
    const T_Unsigned& val_uns = reinterpret_cast<const T_Unsigned&>(val);

    constexpr unsigned int SIZE = sizeof(T);

    for (unsigned int i = 0; i < SIZE; i++)
    {
        assert(os);

        uint8_t byte = (uint8_t) (val_uns >> (i * 8));

        // Don't use insertion operator (control codes mess up the data)
        os.write((const char*) &byte, 1);
    }

    assert(os);
}

template <class T> // NOLINTNEXTLINE(readability-identifier-naming)
inline T __fmt_read(std::istream& is)
{
    static_assert(CHAR_BIT == 8); // 8 bits per byte
    static_assert(std::is_integral_v<T>);

    constexpr unsigned int SIZE = sizeof(T);

    T val(0);
    uint8_t byte;

    for (unsigned int i = 0; i < SIZE; i++)
    {
        assert(is);

        // Don't use extraction operator (control codes mess up the data)
        is.read((char*) &byte, 1);

        const T byte_longer = byte;
        val |= (byte_longer << (i * 8));
    }

    assert(is || is.eof());

    return val;
}

////////////////////////////////////////////////// class ibuffer
class ibuffer
{
public:
    ibuffer(const std::string& file_name);

    void close();

    uint8_t read_u8();
    uint16_t read_u16();
    uint32_t read_u32();
    uint64_t read_u64();

    int8_t read_i8();
    int16_t read_i16();
    int32_t read_i32();
    int64_t read_i64();

    template <class T> // NOLINTNEXTLINE(readability-identifier-naming)
    T __read()
    {
        return __fmt_read<T>(_fs);
    }

private:
    static constexpr std::ifstream::openmode OPEN_MODE = std::ifstream::binary;

    std::ifstream _fs;
};

////////////////////////////////////////////////// ibuffer methods
inline ibuffer::ibuffer(const std::string& file_name): _fs(file_name, OPEN_MODE)
{
    assert(_fs.is_open());
}

inline void ibuffer::close()
{
    assert(_fs.is_open());
    _fs.close();
    assert(!_fs.is_open());
}

////////////////////////////////////////////////// class obuffer
class obuffer
{
public:
    obuffer(const std::string& file_name);

    inline void close();

    void write_u8(const uint8_t& val);
    void write_u16(const uint16_t& val);
    void write_u32(const uint32_t& val);
    void write_u64(const uint64_t& val);

    void write_i8(const int8_t& val);
    void write_i16(const int16_t& val);
    void write_i32(const int32_t& val);
    void write_i64(const int64_t& val);

    template <class T> // NOLINTNEXTLINE(readability-identifier-naming)
    void __write(const T& val)
    {
        __fmt_write(_fs, val);
    }

private:
    static constexpr std::ofstream::openmode OPEN_MODE = //
        std::ofstream::binary |                          //
        std::ofstream::trunc;                            //

    std::ofstream _fs;
};


////////////////////////////////////////////////// obuffer methods
inline obuffer::obuffer(const std::string& file_name): _fs(file_name, OPEN_MODE)
{
    assert(_fs.is_open());
}

inline void obuffer::close()
{
    assert(_fs.is_open());
    _fs.close();
    assert(!_fs.is_open());
}
