#include "mcgs_init.h"
#include "global_options.h"
#include "init_grid_hash_mask.h"
#include "throw_assert.h"
#include <cassert>
#include "cli_options.h"

#include "init_serialization.h"
#include "init_random.h"
#include "init_hashing.h"
#include "init_sumgame.h"
#include "init_impartial_sumgame.h"
#include "init_database.h"
#include "cgt_basics.h"
#include "type_table.h"

namespace {
bool already_initialized = false;
} // namespace


void mcgs_init_1()
{
    mcgs_init::init_color_tables();
}

void mcgs_init_2(const cli_options& opts)
{
    THROW_ASSERT(!already_initialized);
    already_initialized = true;

    mcgs_init::init_grid_hash_mask();

    /*
       Locks further modification of type_table_t fields which require
       explicit initialization from the programmer. Other fields i.e. game type
       IDs and serialization IDs which are automatically assigned will still
       be initialized later upon first use
    */
    type_table_t::set_initialized();

    mcgs_init::init_serialization();
    mcgs_init::init_random();
    mcgs_init::init_hashing();
    mcgs_init::init_sumgame(global::tt_sumgame_idx_bits());
    mcgs_init::init_impartial_sumgame(global::tt_imp_sumgame_idx_bits());

    const init_database_enum init_type =
        global::use_db.get() ? opts.init_database_type : INIT_DATABASE_NONE;

    mcgs_init::init_database(opts.db_file_name, init_type,
                             opts.db_config_string);
}

void mcgs_init_2()
{
    cli_options opts("");
    mcgs_init_2(opts);
}
