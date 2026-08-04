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
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <endian.h>
#include <limits>
#include <string>
#include <type_traits>
#include <cassert>
#include <fstream>
#include <cstdint>
#include <iostream>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "byte_order.h"
#include "integral_conversion.h"
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
    i_ibuffer();
    virtual ~i_ibuffer();

    uint8_t read_u8();
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

    //virtual void read_bytes_raw(void* dst, size_t n_bytes) = 0;

protected:
    size_t _remaining_unread_bytes() const;
    void _ensure_unread_bytes(size_t n_bytes);
    void _read_from_buffer(void* dst, size_t n_bytes);
    virtual void _preload_bytes(size_t n_bytes) = 0;

    const uint8_t* _buffer;
    size_t _buffer_idx;
    size_t _buffer_size;

    static void _on_buffer_not_freed();
    static void _on_bad_read();
};

////////////////////////////////////////////////// interface i_obuffer
class i_obuffer
{
public:
    i_obuffer();
    virtual ~i_obuffer();

    void write_u8(uint8_t val);
    void write_u16(uint16_t val);
    void write_u32(uint32_t val);
    void write_u64(uint64_t val);

    void write_i8(int8_t val);
    void write_i16(int16_t val);
    void write_i32(int32_t val);
    void write_i64(int64_t val);

    void write_bool(const bool& val);

    template <class Enum_T>
    void write_enum(const Enum_T& val);

    // For very large writes. Must flush previously buffered data
    //virtual void write_bytes_raw(const void* src, size_t n_bytes) = 0;

    virtual void flush() = 0;

protected:
    size_t _remaining_buffer_capacity() const;
    void _ensure_buffer_capacity(size_t n_bytes);
    void _write_to_buffer(const void* src, size_t n_bytes);
    virtual void _reserve_capacity(size_t n_bytes) = 0;

    uint8_t* _buffer;
    size_t _buffer_fill;
    size_t _buffer_size;

    static void _on_bad_write();
};


////////////////////////////////////////////////// class file_ibuffer
class file_ibuffer: public i_ibuffer
{
public:
    file_ibuffer(const std::string& file_name);
    ~file_ibuffer();

    void close();

    //void read_bytes_raw(void* dst, size_t n_bytes) override;

protected:
    void _preload_bytes(size_t n_bytes) override;

private:
    size_t _remaining_file_bytes;
    uint8_t* _physical_buffer;

    inline static constexpr size_t _BUFFER_SIZE =
        size_t(1) * 1024 * 1024; // 1 MiB

    //static constexpr std::ifstream::openmode OPEN_MODE =
    //    std::ifstream::binary | std::ifstream::in;

    //std::ifstream _fs;
    FILE* _file;
};

////////////////////////////////////////////////// class file_obuffer
class file_obuffer: public i_obuffer
{
public:
    file_obuffer(const std::string& file_name);
    ~file_obuffer();

    void close();

    //void write_bytes_raw(const void* src, size_t n_bytes) override;
    void flush() override;

protected:
    void _reserve_capacity(size_t n_bytes) override;

private:

    inline static constexpr size_t _BUFFER_SIZE =
        size_t(1) * 1024 * 1024; // 1 MiB

    //inline static constexpr std::ofstream::openmode OPEN_MODE = //
    //    std::ofstream::binary |                                 //
    //    std::ofstream::trunc |                                  //
    //    std::ofstream::out;                                     //

    //std::ofstream _fs;

    FILE* _file;
};


////////////////////////////////////////////////// class memory_ibuffer
class memory_ibuffer: public i_ibuffer
{
public:
    memory_ibuffer(const std::vector<uint8_t>& data_vec);
    ~memory_ibuffer();

    //void read_bytes_raw(void* dst, size_t n_bytes) override;

protected:
    void _preload_bytes(size_t n_bytes) override;

private:
    const std::vector<uint8_t>& _data_vec;
};

////////////////////////////////////////////////// class memory_obuffer
class memory_obuffer: public i_obuffer
{
public:
    memory_obuffer();
    ~memory_obuffer();

    std::vector<uint8_t> release_data();

    void flush() override;
    //void write_bytes_raw(const void* src, size_t n_bytes) override;

protected:
    void _reserve_capacity(size_t n_bytes) override;

private:
    std::vector<uint8_t> _data_vec;

    inline static constexpr size_t _INITIAL_BUFFER_SIZE = size_t(64);
};

//////////////////////////////////////// i_ibuffer methods
inline i_ibuffer::i_ibuffer():
    _buffer(nullptr), _buffer_idx(0), _buffer_size(0)
{
}

inline i_ibuffer::~i_ibuffer()
{
    assert(_buffer == nullptr);
}

inline uint8_t i_ibuffer::read_u8()
{
    _ensure_unread_bytes(sizeof(uint8_t));
    const uint8_t val = _buffer[_buffer_idx];
    _buffer_idx++;
    return val;
}

inline uint16_t i_ibuffer::read_u16()
{
    uint16_t val;
    _read_from_buffer(&val, sizeof(uint16_t));
    return disk_to_host_u16(val);
}

inline uint32_t i_ibuffer::read_u32()
{
    uint32_t val;
    _read_from_buffer(&val, sizeof(uint32_t));
    return disk_to_host_u32(val);
}

inline uint64_t i_ibuffer::read_u64()
{
    uint64_t val;
    _read_from_buffer(&val, sizeof(uint64_t));
    return disk_to_host_u64(val);
}

inline int8_t i_ibuffer::read_i8()
{
    _ensure_unread_bytes(sizeof(int8_t));
    const int8_t val = _buffer[_buffer_idx];
    _buffer_idx++;
    return val;
}

inline int16_t i_ibuffer::read_i16()
{
    int16_t val;
    _read_from_buffer(&val, sizeof(int16_t));
    return disk_to_host_i16(val);
}

inline int32_t i_ibuffer::read_i32()
{
    int32_t val;
    _read_from_buffer(&val, sizeof(int32_t));
    return disk_to_host_i32(val);
}

inline int64_t i_ibuffer::read_i64()
{
    int64_t val;
    _read_from_buffer(&val, sizeof(int64_t));
    return disk_to_host_i64(val);
}

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

inline size_t i_ibuffer::_remaining_unread_bytes() const
{
    return _buffer_size - _buffer_idx;
}

inline void i_ibuffer::_ensure_unread_bytes(size_t n_bytes)
{
    assert(_buffer != nullptr);
    assert(_buffer_idx <= _buffer_size);

    if (_remaining_unread_bytes() >= n_bytes) [[likely]]
        return;

    _preload_bytes(n_bytes);
    assert(_remaining_unread_bytes() >= n_bytes);
}

inline void i_ibuffer::_read_from_buffer(void* dst, size_t n_bytes)
{
    _ensure_unread_bytes(n_bytes);
    std::memcpy(dst, _buffer + _buffer_idx, n_bytes);
    _buffer_idx += n_bytes;
}

//////////////////////////////////////// i_obuffer methods
inline i_obuffer::i_obuffer()
    : _buffer(nullptr), _buffer_fill(0), _buffer_size(0)
{
}

inline i_obuffer::~i_obuffer()
{
    assert(_buffer == nullptr);
}

inline void i_obuffer::write_u8(uint8_t val)
{
    //_ensure_buffer_capacity(sizeof(uint8_t));
    if (_buffer_fill == _buffer_size)
        flush();
    _buffer[_buffer_fill] = val;
    _buffer_fill++;
}

inline void i_obuffer::write_u16(uint16_t val)
{
    val = host_to_disk_u16(val);
    _write_to_buffer(&val, sizeof(uint16_t));
}

inline void i_obuffer::write_u32(uint32_t val)
{
    val = host_to_disk_u32(val);
    _write_to_buffer(&val, sizeof(uint32_t));
}

inline void i_obuffer::write_u64(uint64_t val)
{
    val = host_to_disk_u64(val);
    _write_to_buffer(&val, sizeof(uint64_t));
}

inline void i_obuffer::write_i8(int8_t val)
{
    _ensure_buffer_capacity(sizeof(int8_t));
    _buffer[_buffer_fill] = val;
    _buffer_fill++;
}

inline void i_obuffer::write_i16(int16_t val)
{
    val = host_to_disk_i16(val);
    _write_to_buffer(&val, sizeof(int16_t));
}

inline void i_obuffer::write_i32(int32_t val)
{
    val = host_to_disk_i32(val);
    _write_to_buffer(&val, sizeof(int32_t));
}

inline void i_obuffer::write_i64(int64_t val)
{
    val = host_to_disk_i64(val);
    _write_to_buffer(&val, sizeof(int64_t));
}

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

inline size_t i_obuffer::_remaining_buffer_capacity() const
{
    return _buffer_size - _buffer_fill;
}

inline void i_obuffer::_ensure_buffer_capacity(size_t n_bytes)
{
    if (_remaining_buffer_capacity() >= n_bytes) [[likely]]
        return;

    _reserve_capacity(n_bytes);
    assert(_remaining_buffer_capacity() >= n_bytes);
}

inline void i_obuffer::_write_to_buffer(const void* src, size_t n_bytes)
{
    _ensure_buffer_capacity(n_bytes);
    std::memcpy(_buffer + _buffer_fill, src, n_bytes);
    _buffer_fill += n_bytes;
}


//////////////////////////////////////// file_ibuffer methods
inline file_ibuffer::file_ibuffer(const std::string& file_name)
{
    // Allocate buffer
    _physical_buffer = new uint8_t[_BUFFER_SIZE];
    _buffer = _physical_buffer;
    _buffer_idx = 0;
    _buffer_size = 0;

    // Open file with no buffering
    _file = fopen(file_name.c_str(), "rb");
    assert(_file != nullptr);
    setvbuf(_file, nullptr, _IONBF, 0);

    // Read file size
#if defined(_WIN32) || defined(_WIN64)
    const int fd = _fileno(_file);
    assert(fd != -1);

    struct _stat64 st;
    const int fstat_status = _fstat64(fd, &st);
    assert(fstat_status == 0);

    _remaining_file_bytes = st.st_size;
#else
    const int fd = fileno(_file);
    assert(fd != -1);

    struct stat st;
    const int fstat_status = fstat(fd, &st);
    assert(fstat_status == 0);

    _remaining_file_bytes = st.st_size;
#endif

}

inline file_ibuffer::~file_ibuffer()
{
    close();
}

inline void file_ibuffer::close()
{
    if (_physical_buffer != nullptr)
    {
        delete[] _physical_buffer;

        _physical_buffer = nullptr;
        _buffer = nullptr;
        _buffer_idx = 0;
        _buffer_size = 0;
    }

    if (_file != nullptr)
    {
        fclose(_file);
        _file = nullptr;
    }
}

inline void file_ibuffer::_preload_bytes(size_t n_bytes)
{
    assert(_buffer != nullptr);

    // Move buffer contents
    const size_t n_unread = _remaining_unread_bytes();

    if (n_unread > 0)
        std::memmove(_physical_buffer, _physical_buffer + _buffer_idx, n_unread);

    // Read data from file
    const size_t remaining_capacity = _BUFFER_SIZE - n_unread;

    const size_t bytes_wanted = std::min(remaining_capacity, _remaining_file_bytes);

    const size_t bytes_got =
        fread(reinterpret_cast<char*>(_physical_buffer + n_unread), 1,
              bytes_wanted, _file);

    _remaining_file_bytes -= bytes_got;

    _buffer_size = n_unread + bytes_got;
    _buffer_idx = 0;
}

//////////////////////////////////////// file_obuffer methods
inline file_obuffer::file_obuffer(const std::string& file_name)
{
    _buffer = new uint8_t[_BUFFER_SIZE];
    _buffer_fill = 0;
    _buffer_size = _BUFFER_SIZE;

    //_fs.rdbuf()->pubsetbuf(nullptr, 0);
    //_fs.open(file_name, OPEN_MODE);
    //assert(_fs.is_open());

    _file = fopen(file_name.c_str(), "wb");
    assert(_file != nullptr);

    const int buf_status = setvbuf(_file, nullptr, _IONBF, 0);
    assert(buf_status == 0);

}

inline file_obuffer::~file_obuffer()
{
    close();
}

inline void file_obuffer::close()
{
    if (_buffer != nullptr)
    {
        flush();

        delete[] _buffer;
        _buffer = nullptr;
        _buffer_fill = 0;
        _buffer_size = 0;
    }

    //if (_fs.is_open())
    //{
    //    _fs.flush();
    //    _fs.close();
    //}

    if (_file != nullptr)
    {
        fclose(_file);
        _file = nullptr;
    }
}

inline void file_obuffer::flush()
{
    assert(_buffer != nullptr);

    //_fs.write((const char*) _buffer, _buffer_fill);
    fwrite((const char*) _buffer, 1, _buffer_fill, _file);
    _buffer_fill = 0;
}

inline void file_obuffer::_reserve_capacity(size_t n_bytes)
{
    flush();
}

////////////////////////////////////////////////// memory_ibuffer methods
inline memory_ibuffer::memory_ibuffer(const std::vector<uint8_t>& data_vec)
    : _data_vec(data_vec)
{
    _buffer = data_vec.data();
    _buffer_idx = 0;
    _buffer_size = data_vec.size();
}

inline memory_ibuffer::~memory_ibuffer()
{
    _buffer = nullptr;
    _buffer_idx = 0;
    _buffer_size = 0;
}

inline void memory_ibuffer::_preload_bytes(size_t n_bytes)
{
}

////////////////////////////////////////////////// memory_obuffer methods
inline memory_obuffer::memory_obuffer(): _data_vec(_INITIAL_BUFFER_SIZE)
{
    _buffer = _data_vec.data();
    _buffer_fill = 0;
    _buffer_size = _data_vec.size();
}

inline memory_obuffer::~memory_obuffer()
{
    _buffer = nullptr;
    _buffer_fill = 0;
    _buffer_size = 0;
}

inline std::vector<uint8_t> memory_obuffer::release_data()
{
    std::vector<uint8_t> data = std::move(_data_vec);
    data.resize(_buffer_fill);
    data.shrink_to_fit();

    _data_vec = std::vector<uint8_t>(_INITIAL_BUFFER_SIZE);
    _buffer = _data_vec.data();
    _buffer_fill = 0;
    _buffer_size = _INITIAL_BUFFER_SIZE;

    return data;
}

inline void memory_obuffer::flush()
{
}

inline void memory_obuffer::_reserve_capacity(size_t n_bytes)
{
    const size_t new_size = _buffer_size * 2;
    THROW_ASSERT(new_size > _buffer_size);

    _data_vec.resize(new_size);
    _buffer = _data_vec.data();
    _buffer_size = new_size;
}

