#pragma once
#include <cstdint>
#include <cstddef>
#include <type_traits>

template <class Int_T>
Int_T host_to_disk_impl(Int_T val)
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    using Int_Unsigned_T = std::make_unsigned_t<Int_T>;

    const Int_Unsigned_T val_unsigned = static_cast<Int_Unsigned_T>(val);

    Int_T result;
    uint8_t* result_bytes = reinterpret_cast<uint8_t*>(&result);

    for (size_t i = 0; i < sizeof(Int_T); i++)
        result_bytes[i] = static_cast<uint8_t>(val_unsigned >> (i * 8));

    return result;
}

template <class Int_T>
Int_T disk_to_host_impl(Int_T val)
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    using Int_Unsigned_T = std::make_unsigned_t<Int_T>;

    Int_Unsigned_T result(0);
    uint8_t* val_bytes = reinterpret_cast<uint8_t*>(&val);

    for (size_t i = 0; i < sizeof(Int_T); i++)
        result |= static_cast<Int_Unsigned_T>(val_bytes[i]) << (i * 8);

    return static_cast<Int_T>(result);
}

inline uint8_t host_to_disk_u8(uint8_t val)
{
    return val;
}

inline uint16_t host_to_disk_u16(uint16_t val)
{
    return host_to_disk_impl(val);
}

inline uint32_t host_to_disk_u32(uint32_t val)
{
    return host_to_disk_impl(val);
}

inline uint64_t host_to_disk_u64(uint64_t val)
{
    return host_to_disk_impl(val);
}

inline int8_t host_to_disk_i8(int8_t val)
{
    return val;
}

inline int16_t host_to_disk_i16(int16_t val)
{
    return host_to_disk_impl(val);
}

inline int32_t host_to_disk_i32(int32_t val)
{
    return host_to_disk_impl(val);
}

inline int64_t host_to_disk_i64(int64_t val)
{
    return host_to_disk_impl(val);
}

inline uint8_t disk_to_host_u8(uint8_t val)
{
    return val;
}

inline uint16_t disk_to_host_u16(uint16_t val)
{
    return disk_to_host_impl(val);
}

inline uint32_t disk_to_host_u32(uint32_t val)
{
    return disk_to_host_impl(val);
}

inline uint64_t disk_to_host_u64(uint64_t val)
{
    return disk_to_host_impl(val);
}

inline int8_t disk_to_host_i8(int8_t val)
{
    return val;
}

inline int16_t disk_to_host_i16(int16_t val)
{
    return disk_to_host_impl(val);
}

inline int32_t disk_to_host_i32(int32_t val)
{
    return disk_to_host_impl(val);
}

inline int64_t disk_to_host_i64(int64_t val)
{
    return disk_to_host_impl(val);
}

