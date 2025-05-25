#include "random.h"
#include <cstdint>
#include <random>
#include <cassert>

namespace {
bool is_initialized = false;

std::mt19937_64 rng;

// TODO should these include 0 in the range?

// u64 distribution
static_assert(sizeof(unsigned long long) >= sizeof(uint64_t));
std::uniform_int_distribution<unsigned long long> dist_64(
        1, std::numeric_limits<uint64_t>::max());

// u32 distribution
static_assert(sizeof(unsigned int) >= sizeof(uint32_t));
std::uniform_int_distribution<unsigned int> dist_32(
        1, std::numeric_limits<uint32_t>::max());

// u16 distribution
static_assert(sizeof(unsigned short) >= sizeof(uint16_t));
std::uniform_int_distribution<unsigned short> dist_16(
        1, std::numeric_limits<uint16_t>::max());

// u8 distribution
static_assert(sizeof(unsigned short) >= sizeof(uint8_t));
std::uniform_int_distribution<unsigned short> dist_8(
        1, std::numeric_limits<uint8_t>::max());

} // namespace

void set_random_seed(uint64_t seed)
{
    assert(!is_initialized);
    assert(seed != 0);

    is_initialized = true;
    rng.seed(seed);
}

uint64_t get_random_u64()
{
    assert(is_initialized);
    return (uint64_t) dist_64(rng);
}

uint32_t get_random_u32()
{
    assert(is_initialized);
    return (uint32_t) dist_32(rng);
}

uint16_t get_random_u16()
{
    assert(is_initialized);
    return (uint16_t) dist_16(rng);
}

uint8_t get_random_u8()
{
    assert(is_initialized);
    return (uint8_t) dist_8(rng);
}
