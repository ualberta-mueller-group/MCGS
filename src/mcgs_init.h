#pragma once
#include <cstdint>
#include <cstddef>

struct init_options
{
    init_options();

    uint64_t random_table_seed;
    static constexpr uint64_t DEFAULT_RANDOM_TABLE_SEED = 7753;

    size_t ttable_sumgame_index_bits;
    static constexpr size_t DEFAULT_TTABLE_SUMGAME_INDEX_BITS = 24;
};

void mcgs_init_all(const init_options& opts);
void mcgs_init_all();
