#include "iobuffer.h"

#include <algorithm>
#include <cstdint>
#include <execution>
#include <filesystem>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include "safe_arithmetic.h"
#include "throw_assert.h"
#include "utilities.h"

namespace {

template <size_t n_bytes, size_t alignment>
struct aligned_array
{
    aligned_array()
    {
        _data = new (std::align_val_t(alignment)) uint8_t[n_bytes];
    }

    ~aligned_array()
    {
        ::operator delete[] (_data, std::align_val_t(alignment));
    }

    operator const uint8_t*() const
    {
        return _data;
    }

    operator uint8_t*() { return _data; }

    inline static constexpr size_t N_BYTES = n_bytes;

private:
    uint8_t* _data;
};

aligned_array<size_t(1) * 1024 * 1024, sizeof(intmax_t)> data_buffer;


template <class Int_T>
Int_T host_to_network(const Int_T val_host)
{
    static_assert(std::is_integral_v<Int_T>);

    // NOLINTNEXTLINE(readability-identifier-naming)
    using Int_Unsigned_T = std::make_unsigned_t<Int_T>;
    const Int_Unsigned_T val_host_unsigned = static_cast<Int_Unsigned_T>(val_host);

    Int_T val_network(0);
    uint8_t* val_network_bytes = reinterpret_cast<uint8_t*>(&val_network);

    for (size_t i = 0; i < sizeof(Int_T); i++)
        val_network_bytes[i] = static_cast<uint8_t>(val_host_unsigned >> (i * 8));

    return val_network;
}

template <class Int_T>
Int_T network_to_host(const Int_T val_network)
{
    static_assert(std::is_integral_v<Int_T>);

    // NOLINTNEXTLINE(readability-identifier-naming)
    using Int_Unsigned_T = std::make_unsigned_t<Int_T>;

    const uint8_t* val_network_bytes = reinterpret_cast<const uint8_t*>(&val_network);

    Int_Unsigned_T val_host(0);

    for (size_t i = 0; i < sizeof(Int_T); i++)
        val_host |= (static_cast<Int_Unsigned_T>(val_network_bytes[i]) << (i * 8));

    return static_cast<Int_T>(val_host);
}

template <class Int_T>
void write_integral_array_impl(const Int_T* src, size_t n_elements, i_obuffer& os)
{
    static_assert(std::is_integral_v<Int_T> && sizeof(Int_T) > 1);

    constexpr size_t BUFFER_SIZE = data_buffer.N_BYTES;
    constexpr size_t MAX_ELEMENTS_PER_BATCH = BUFFER_SIZE / sizeof(Int_T);
    static_assert(MAX_ELEMENTS_PER_BATCH > 0);

    if (src == nullptr)
    {
        assert(n_elements == 0);
        return;
    }

    uint8_t* buffer_u8 = data_buffer;
    Int_T* buffer_int_t = reinterpret_cast<Int_T*>(buffer_u8);

    size_t idx = 0;
    while (idx < n_elements)
    {
        const size_t n_batch_elements =
            std::min(MAX_ELEMENTS_PER_BATCH, n_elements - idx);
        const size_t n_batch_bytes = n_batch_elements * sizeof(Int_T);

        std::transform(src + idx, src + idx + n_batch_elements, buffer_int_t,
                       host_to_network<Int_T>);

        os.write_bytes(buffer_u8, n_batch_bytes);
        idx += n_batch_elements;
    }
}

template <class Int_T>
void read_integral_array_impl(Int_T* dst, size_t n_elements, i_ibuffer& is)
{
    static_assert(std::is_integral_v<Int_T> && sizeof(Int_T) > 1);

    if (dst == nullptr)
    {
        assert(n_elements == 0);
        return;
    }

    is.read_bytes(dst, n_elements * sizeof(Int_T));
    std::transform(dst, dst + n_elements, dst, network_to_host<Int_T>);
}

} // namespace

////////////////////////////////////////////////// i_ibuffer methods
void i_ibuffer::read_integral_array(uint8_t* dst, size_t n_elements)
{
    read_bytes(dst, n_elements);
}

void i_ibuffer::read_integral_array(uint16_t* dst, size_t n_elements)
{
    read_integral_array_impl(dst, n_elements, *this);
}

void i_ibuffer::read_integral_array(uint32_t* dst, size_t n_elements)
{
    read_integral_array_impl(dst, n_elements, *this);
}

void i_ibuffer::read_integral_array(uint64_t* dst, size_t n_elements)
{
    read_integral_array_impl(dst, n_elements, *this);
}

void i_ibuffer::read_integral_array(int8_t* dst, size_t n_elements)
{
    read_bytes(dst, n_elements);
}

void i_ibuffer::read_integral_array(int16_t* dst, size_t n_elements)
{
    read_integral_array_impl(dst, n_elements, *this);
}

void i_ibuffer::read_integral_array(int32_t* dst, size_t n_elements)
{
    read_integral_array_impl(dst, n_elements, *this);
}

void i_ibuffer::read_integral_array(int64_t* dst, size_t n_elements)
{
    read_integral_array_impl(dst, n_elements, *this);
}

uint16_t i_ibuffer::read_u16()
{
    return __read<uint16_t>();
}

uint32_t i_ibuffer::read_u32()
{
    return __read<uint32_t>();
}

uint64_t i_ibuffer::read_u64()
{
    return __read<uint64_t>();
}

int8_t i_ibuffer::read_i8()
{
    return __read<int8_t>();
}

int16_t i_ibuffer::read_i16()
{
    return __read<int16_t>();
}

int32_t i_ibuffer::read_i32()
{
    return __read<int32_t>();
}

int64_t i_ibuffer::read_i64()
{
    return __read<int64_t>();
}

void i_ibuffer::_on_bad_read()
{
    std::cerr << "Bad read from ibuffer!" << std::endl;
    std::abort();
}

////////////////////////////////////////////////// i_obuffer methods
void i_obuffer::write_integral_array(const uint8_t* src, size_t n_elements)
{
    write_bytes(src, n_elements);
}

void i_obuffer::write_integral_array(const uint16_t* src, size_t n_elements)
{
    write_integral_array_impl(src, n_elements, *this);
}

void i_obuffer::write_integral_array(const uint32_t* src, size_t n_elements)
{
    write_integral_array_impl(src, n_elements, *this);
}

void i_obuffer::write_integral_array(const uint64_t* src, size_t n_elements)
{
    write_integral_array_impl(src, n_elements, *this);
}

void i_obuffer::write_integral_array(const int8_t* src, size_t n_elements)
{
    write_bytes(src, n_elements);
}

void i_obuffer::write_integral_array(const int16_t* src, size_t n_elements)
{
    write_integral_array_impl(src, n_elements, *this);
}

void i_obuffer::write_integral_array(const int32_t* src, size_t n_elements)
{
    write_integral_array_impl(src, n_elements, *this);
}

void i_obuffer::write_integral_array(const int64_t* src, size_t n_elements)
{
    write_integral_array_impl(src, n_elements, *this);
}

void i_obuffer::write_u16(const uint16_t& val)
{
    __write<uint16_t>(val);
}

void i_obuffer::write_u32(const uint32_t& val)
{
    __write<uint32_t>(val);
}

void i_obuffer::write_u64(const uint64_t& val)
{
    __write<uint64_t>(val);
}

void i_obuffer::write_i8(const int8_t& val)
{
    __write<int8_t>(val);
}

void i_obuffer::write_i16(const int16_t& val)
{
    __write<int16_t>(val);
}

void i_obuffer::write_i32(const int32_t& val)
{
    __write<int32_t>(val);
}

void i_obuffer::write_i64(const int64_t& val)
{
    __write<int64_t>(val);
}

void i_obuffer::_on_bad_write()
{
    std::cerr << "Bad write to obuffer!" << std::endl;
    std::abort();
}

////////////////////////////////////////////////// file_ibuffer methods
file_ibuffer::file_ibuffer(const std::string& file_name) : _fs(file_name, OPEN_MODE)
{
    THROW_ASSERT(std::filesystem::exists(file_name),
                 "Input file \"" + file_name + "\" not found!");

    THROW_ASSERT(_fs.is_open(),
                 "Failed to open input file \"" + file_name + "\"!");
}

