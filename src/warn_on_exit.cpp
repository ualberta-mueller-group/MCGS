#include "warn_on_exit.h"
#include "global_options.h"

#include <cassert>
#include <iostream>

using namespace std;

////////////////////////////////////////////////// Helpers
namespace {
bool is_initialized = false;

bool did_random_table_resize;
void warn_random_table_resize()
{
    if (global::silence_warnings())
        return;

    cerr << "WARNING: a random_table was resized 1 or more times during search."
            " This may affect validity of reported times."
         << endl;
}

bool did_db_dom_moves_complexity_overflow;
void warn_db_dom_moves_complexity_overflow()
{
    if (global::silence_warnings())
        return;

    cerr << "WARNING: complexity score overflowed during DB creation. This is "
            "not an error in the current version of MCGS."
         << endl;
}

} // namespace

////////////////////////////////////////////////// Exported functions
void warn_on_exit::on_random_table_resize()
{
    assert(is_initialized);

    if (did_random_table_resize)
        return;

    did_random_table_resize = true;
    warn_random_table_resize();
}

void warn_on_exit::on_db_dom_moves_complexity_overflow()
{
    assert(is_initialized);

    if (did_db_dom_moves_complexity_overflow)
        return;

    did_db_dom_moves_complexity_overflow = true;
    warn_db_dom_moves_complexity_overflow();
}

void warn_on_exit::print_exit_warnings()
{
    assert(is_initialized);

    if (did_random_table_resize)
        warn_random_table_resize();

    if (did_db_dom_moves_complexity_overflow)
        warn_db_dom_moves_complexity_overflow();
}

namespace mcgs_init {
void init_warn_on_exit()
{
    assert(!is_initialized);
    is_initialized = true;

    did_random_table_resize = false;
    did_db_dom_moves_complexity_overflow = false;
}

} // namespace mcgs_init
