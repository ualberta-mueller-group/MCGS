#include "mcgs_init.h"
#include "throw_assert.h"
#include <cassert>
#include "hashing_init.h"
#include "sumgame_init.h"

init_options::init_options()
    : random_table_seed(DEFAULT_RANDOM_TABLE_SEED),
    ttable_sumgame_index_bits(DEFAULT_TTABLE_SUMGAME_INDEX_BITS)
{
}

namespace {
bool already_initialized = false;
} // namespace

void mcgs_init_all(const init_options& opts)
{
    THROW_ASSERT(!already_initialized);
    already_initialized = true;

    mcgs_init::init_hashing(opts.random_table_seed);
    mcgs_init::init_ttable_sumgame(opts.ttable_sumgame_index_bits);
}

void mcgs_init_all()
{
    init_options opts;
    mcgs_init_all(opts);
}

