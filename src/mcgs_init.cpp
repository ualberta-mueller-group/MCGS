#include "mcgs_init.h"
#include "global_options.h"
#include "throw_assert.h"
#include <cassert>
#include "hashing_init.h"
#include "sumgame_init.h"

namespace {
bool already_initialized = false;
} // namespace

void mcgs_init_all(const cli_options& opts)
{
    THROW_ASSERT(!already_initialized);
    already_initialized = true;

    mcgs_init::init_hashing(*global::random_table_seed);
    mcgs_init::init_ttable_sumgame(*global::tt_sumgame_idx_bits);
}

void mcgs_init_all()
{
    cli_options opts("");
    mcgs_init_all(opts);
}
