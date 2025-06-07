#pragma once
/*
   Global random number generator.

    IMPORTANT: care must be taken when using the global random generator to
    seed other random generators
*/

#include <cstdint>

// To be called by mcgs_init
void set_random_seed(uint64_t seed);

// Must have called set_random_seed first
uint64_t get_random_u64();
uint32_t get_random_u32();
uint16_t get_random_u16();
uint8_t get_random_u8();
