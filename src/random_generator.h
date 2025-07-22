/*
    TODO:
        - Make sure this file is efficient
        - Change random.h to use random_generator defined in this file.
*/
#pragma once
#include <cstdint>
#include <random>
#include "utilities.h"

////////////////////////////////////////////////// asserts
static_assert(sizeof(unsigned long long) >= sizeof(uint64_t));
static_assert(sizeof(unsigned int) >= sizeof(uint32_t));
static_assert(sizeof(unsigned short) >= sizeof(uint16_t));
static_assert(sizeof(unsigned short) >= sizeof(uint8_t));

////////////////////////////////////////////////// class random_generator
class random_generator
{
private:
public:
    random_generator(uint64_t seed)
        : _rng(seed != 0 ? seed : ms_since_epoch()),
         _dist_u64(0, UINT64_MAX),
         _dist_u32(0, UINT32_MAX),
         _dist_u16(0, UINT16_MAX),
         _dist_u8(0, UINT8_MAX),
         _dist_i64(INT64_MIN, INT64_MAX),
         _dist_i32(INT32_MIN, INT32_MAX),
         _dist_i16(INT16_MIN, INT16_MAX),
         _dist_i8(INT8_MIN, INT8_MAX)
    {
    }

    uint64_t get_u64(uint64_t min = 0, uint64_t max = UINT64_MAX);
    uint32_t get_u32(uint32_t min = 0, uint32_t max = UINT32_MAX);
    uint16_t get_u16(uint16_t min = 0, uint16_t max = UINT16_MAX);
    uint8_t get_u8(uint8_t min = 0, uint8_t max = UINT8_MAX);

    int64_t get_i64(int64_t min = INT64_MIN, int64_t max = INT64_MAX);
    int32_t get_i32(int32_t min = INT32_MIN, int32_t max = INT32_MAX);
    int16_t get_i16(int16_t min = INT16_MIN, int16_t max = INT16_MAX);
    int8_t get_i8(int8_t min = INT8_MIN, int8_t max = INT8_MAX);

    std::mt19937_64& get_rng();

private:
    std::mt19937_64 _rng;

    std::uniform_int_distribution<unsigned long long> _dist_u64;
    std::uniform_int_distribution<unsigned int> _dist_u32;
    std::uniform_int_distribution<unsigned short> _dist_u16;
    std::uniform_int_distribution<unsigned short> _dist_u8;

    std::uniform_int_distribution<long long> _dist_i64;
    std::uniform_int_distribution<int> _dist_i32;
    std::uniform_int_distribution<short> _dist_i16;
    std::uniform_int_distribution<short> _dist_i8;
};

////////////////////////////////////////////////// random_generator methods
inline std::mt19937_64& random_generator::get_rng()
{
    return _rng;
}

//////////////////////////////////////// Unsigned values
inline uint64_t random_generator::get_u64(uint64_t min, uint64_t max)
{
    _dist_u64 = decltype(_dist_u64)(min, max);
    return _dist_u64(_rng);
}

inline uint32_t random_generator::get_u32(uint32_t min, uint32_t max)
{
    _dist_u32 = decltype(_dist_u32)(min, max);
    return _dist_u32(_rng);
}

inline uint16_t random_generator::get_u16(uint16_t min, uint16_t max)
{
    _dist_u16 = decltype(_dist_u16)(min, max);
    return _dist_u16(_rng);
}

inline uint8_t random_generator::get_u8(uint8_t min, uint8_t max)
{
    _dist_u8 = decltype(_dist_u8)(min, max);
    return _dist_u8(_rng);
}

//////////////////////////////////////// Signed values
inline int64_t random_generator::get_i64(int64_t min, int64_t max)
{
    _dist_i64 = decltype(_dist_i64)(min, max);
    return _dist_i64(_rng);
}

inline int32_t random_generator::get_i32(int32_t min, int32_t max)
{
    _dist_i32 = decltype(_dist_i32)(min, max);
    return _dist_i32(_rng);
}

inline int16_t random_generator::get_i16(int16_t min, int16_t max)
{
    _dist_i16 = decltype(_dist_i16)(min, max);
    return _dist_i16(_rng);
}

inline int8_t random_generator::get_i8(int8_t min, int8_t max)
{
    _dist_i8 = decltype(_dist_i8)(min, max);
    return _dist_i8(_rng);
}
