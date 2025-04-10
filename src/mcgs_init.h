#pragma once
#include <cstdint>

struct init_options
{
    init_options();

    uint64_t random_table_seed;

    static constexpr uint64_t DEFAULT_RANDOM_TABLE_SEED = 7753;
};

void mcgs_init_all(const init_options& opts);
void mcgs_init_all();
