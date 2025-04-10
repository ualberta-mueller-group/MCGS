//---------------------------------------------------------------------------
// main_test.cpp - main loop of MCGS unit test
// Imports all unit tests
//---------------------------------------------------------------------------

#include "hash_eval.h"
const bool RUN_OVERRIDE_TESTS = true;

#include <cassert>
#include <string>
#include <iostream>

#include "mcgs_init.h"

#include "cgt_basics_test.h"
#include "cgt_dyadic_rational_test.h"
#include "cgt_integer_game_test.h"
#include "cgt_move_test.h"
#include "cgt_nimber_test.h"
#include "cgt_switch_test.h"
#include "cgt_up_star_test.h"
#include "clobber_1xn_test.h"
#include "nogo_1xn_test.h"
#include "sumgame_test.h"
#include "elephants_test.h"

#include "split_test.h"
#include "file_parser_test.h"
#include "cli_options_test.h"
#include "simple_text_hash_test.h"

#include "scale_test.h"
#include "game_bounds_test.h"
#include "find_bounds_test.h"
#include "utilities_test.h"
#include "game_type_test.h"
#include "safe_arithmetic_test.h"
#include "fraction_test.h"
#include "sumgame_map_view_test.h"
#include "custom_traits_test.h"
#include "cgt_game_simplification_test.h"
#include "hash_test.h"

using std::cout, std::endl, std::string;

namespace {
void override_tests()
{
    hash_test_all();
}

void print_flag(const string& flag_string, const string& flag_description)
{
    cout << "\t" << flag_string << endl;
    cout << "\t\t" << flag_description << endl;
    cout << endl;
}

void print_usage(const char* exec_name)
{
    cout << "Usage: " << exec_name << " [flags]" << endl;

    cout << "\tRuns unit tests. On successful completion, \"SUCCESS\" should "
            "be printed.";
    cout << endl;

    cout << "Flags:" << endl;
    print_flag("--no-slow-tests", "Skip running tests which take longer.");
    print_flag("-h, --help", "Print this message and exit.");
}
} // namespace

int main(int argc, const char** argv)
{
    init_options opts;
    opts.random_table_seed = 0;
    mcgs_init_all(opts);

    if (RUN_OVERRIDE_TESTS)
    {
        override_tests();
        cout << "DONE. Remember to disable override tests (at top of "
                "main_test.cpp)"
             << endl;
        return 0;
    }

    bool do_slow_tests = true;

    // arg parse loop
    for (int i = 1; i < argc; i++) // skip executable name
    {
        string arg = argv[i];

        if (arg == "--no-slow-tests")
        {
            do_slow_tests = false;
            continue;
        }

        if (arg == "-h" || arg == "--help")
        {
            assert(argc >= 1);
            print_usage(argv[0]);
            return 0;
        }

        assert(argc >= 1);
        print_usage(argv[0]);
        return 0;
    }

    cgt_basics_test_all();
    cgt_dyadic_rational_test_all();
    cgt_integer_game_test_all();
    cgt_move_test_all();
    cgt_nimber_test_all();
    cgt_switch_test_all();
    cgt_up_star_test_all();

    utilities_test_all();
    game_type_test_all();
    safe_arithmetic_test_all();
    fraction_test_all();

    split_test_all();

    if (do_slow_tests)
    {
        clobber_1xn_test_all();
        nogo_1xn_test_all();
        elephants_test_all(); // takes several seconds
        sumgame_test_all();
    }

    file_parser_test_all();
    cli_options_test_all();
    simple_text_hash_test_all();

    scale_test_all();
    game_bounds_test_all();
    find_bounds_test_all();

    sumgame_map_view_test_all();

    // NOTE: this test runs at compile time, not run time
    custom_traits_test_all();

    cgt_game_simplification_test_all();

    hash_test_all();

    cout << "SUCCESS" << endl;
}
