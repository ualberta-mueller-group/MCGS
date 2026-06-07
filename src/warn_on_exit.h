/*
    Print warnings the first time some bad (but not fatal) event happens, then
    print them again on exit.

    Warnings are not printed when `--silence-warnings` is specified from the
    CLI.
*/
#pragma once

// Create on stack after calling "mcgs_init" functions
class warn_on_exit
{
public:
    ~warn_on_exit() { print_exit_warnings(); }

    static void on_random_table_resize();
    static void on_db_dom_moves_complexity_overflow();

    static void print_exit_warnings();
};

namespace mcgs_init {
void init_warn_on_exit();
} // namespace mcgs_init
