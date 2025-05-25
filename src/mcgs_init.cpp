#include "mcgs_init.h"
#include "global_options.h"
#include "throw_assert.h"
#include <cassert>
#include "cli_options.h"

#include "debug_print.h"
#include "init_hashing.h"
#include "init_sumgame.h"
#include "init_impartial_sumgame.h"
#include "init_random.h"

namespace {
bool already_initialized = false;
} // namespace

void mcgs_init_all(const cli_options& opts)
{
    THROW_ASSERT(!already_initialized);
    already_initialized = true;

    mcgs_init::init_random();
    mcgs_init::init_debug_print(global::debug_file());
    mcgs_init::init_hashing();
    mcgs_init::init_sumgame(global::tt_sumgame_idx_bits());
    mcgs_init::init_impartial_sumgame(global::tt_imp_sumgame_idx_bits());
}

void mcgs_init_all()
{
    cli_options opts("");
    mcgs_init_all(opts);
}
