//---------------------------------------------------------------------------
// main_test.cpp - main loop of MCGS unit test
// Imports all unit tests
//---------------------------------------------------------------------------

#include "integral_conversion_test.h"
#define RUN_OVERRIDE_TESTS false
#define SHOW_TEST_CALLS true


#if SHOW_TEST_CALLS
#include "stopwatch.h"
static stopwatch sw;
#define RUN_TEST(test_fn_call)                                                 \
    do                                                                         \
    {                                                                          \
        std::cout << "Running test: `" #test_fn_call "`" << std::flush;        \
        sw.reset();                                                            \
        sw.start();                                                            \
        test_fn_call;                                                          \
        sw.stop();                                                             \
        std::cout << " (" << sw.get_duration_ms() << " ms)" << std::endl;      \
    } while (0)
#else
#define RUN_TEST(test_fn_call)                                                 \
    do                                                                         \
    {                                                                          \
        test_fn_call;                                                          \
    } while (0)
#endif

#include <cassert>
#include <string>
#include <iostream>

#include "amazons_test.h"
#include "bit_array_test.h"
#include "cannibal_clobber_test.h"
#include "cgt_basics_test.h"
#include "cgt_dyadic_rational_test.h"
#include "cgt_game_simplification_test.h"
#include "cgt_integer_game_test.h"
#include "cgt_move_test.h"
#include "cgt_nimber_test.h"
#include "cgt_switch_test.h"
#include "cgt_up_star_test.h"
#include "cli_options_test.h"
#include "clobber_1xn_test.h"
#include "clobber_test.h"
#include "database_test.h"
#include "domineering_test.h"
#include "elephants_test.h"
#include "file_parser_test.h"
#include "find_bounds_test.h"
#include "fission_test.h"
#include "fraction_test.h"
#include "game_bounds_test.h"
#include "game_clone_test.h"
#include "game_type_test.h"
#include "gen_king_dirt_test.h"
#include "gen_toads_test.h"
#include "global_options.h"
#include "grid_game_hashes_test.h"
#include "grid_generator_test.h"
#include "grid_hash_test.h"
#include "grid_location_test.h"
#include "grid_mask_test.h"
#include "hash_test.h"
#include "hash_types_test.h"
#include "impartial_game_wrapper_test.h"
#include "impartial_minimax_test.h"
#include "impartial_sumgame_test.h"
#include "kayles_test.h"
#include "mcgs_init.h"
#include "n_bit_int_test.h"
#include "nogo_1xn_test.h"
#include "nogo_test.h"
#include "normalize_test.h"
#include "order_test.h"
#include "random_test.h"
#include "safe_arithmetic_test.h"
#include "scale_test.h"
#include "sheep_grid_generator_test.h"
#include "sheep_test.h"
#include "simple_text_hash_test.h"
#include "split_test.h"
#include "sumgame_helpers_test.h"
#include "sumgame_map_view_test.h"
#include "sumgame_test.h"
#include "thermograph_helpers_test.h"
#include "throw_assert.h"
#include "toppling_dominoes_test.h"
#include "utilities_test.h"
#include "winning_moves_test.h"

using namespace std;

namespace {

void override_tests()
{
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
    print_flag("--extra-tests", "Run extra tests which may take a long time.");
    print_flag("-h, --help", "Print this message and exit.");
}
} // namespace

int main(int argc, const char** argv)
{
    THROW_ASSERT(argc >= 1);
    mcgs_init_1(argv[0]);

    global::silence_warnings.set(true);
    global::use_db.set(false);
    mcgs_init_2();

    if (RUN_OVERRIDE_TESTS)
    {
        override_tests();
        cout << "DONE. Remember to disable override tests (at top of "
             << __FILE__ << ")" << endl;
        return 0;
    }

    bool do_extra_tests = false;

    // arg parse loop
    for (int i = 1; i < argc; i++) // skip executable name
    {
        string arg = argv[i];

        if (arg == "--extra-tests")
        {
            do_extra_tests = true;
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

    // Utility functions
    RUN_TEST(integral_conversion_test_all());
    RUN_TEST(utilities_test_all());
    RUN_TEST(random_test_all());

    RUN_TEST(fraction_test_all());
    RUN_TEST(safe_arithmetic_test_all());

    RUN_TEST(bit_array_test_all());

    // CGT utility functions
    RUN_TEST(cgt_basics_test_all());
    RUN_TEST(n_bit_int_test_all());
    RUN_TEST(cgt_move_test_all(do_extra_tests));

    // Game fundamentals
    RUN_TEST(game_type_test_all());
    RUN_TEST(order_test_all());

    RUN_TEST(hash_test_all());
    RUN_TEST(hash_types_test_all());

    RUN_TEST(grid_location_test_all());
    RUN_TEST(grid_hash_test_all());
    RUN_TEST(grid_mask_test_all());
    RUN_TEST(grid_game_hashes_test_all());

    RUN_TEST(sumgame_map_view_test_all());
    RUN_TEST(cgt_game_simplification_test_all());

    // Specific games
    RUN_TEST(cgt_nimber_test_all());
    RUN_TEST(cgt_switch_test_all());
    RUN_TEST(cgt_up_star_test_all());

    RUN_TEST(cgt_dyadic_rational_test_all());
    RUN_TEST(cgt_integer_game_test_all());

    RUN_TEST(clobber_1xn_test_all());
    RUN_TEST(clobber_test_all());
    RUN_TEST(cannibal_clobber_test_all());
    RUN_TEST(nogo_1xn_test_all());
    RUN_TEST(nogo_test_all());
    RUN_TEST(elephants_test_all());
    RUN_TEST(sumgame_test_all());
    RUN_TEST(sumgame_helpers_test_all());

    RUN_TEST(domineering_test_all());
    RUN_TEST(amazons_test_all());
    RUN_TEST(fission_test_all());
    RUN_TEST(toppling_dominoes_test_all());
    RUN_TEST(gen_toads_test_all());
    RUN_TEST(sheep_test_all());
    RUN_TEST(gen_king_dirt_test_all());

    RUN_TEST(normalize_test_all());
    RUN_TEST(split_test_all());
    RUN_TEST(game_clone_test_all());

    // Impartial games
    RUN_TEST(kayles_test_all());
    RUN_TEST(impartial_game_wrapper_test_all());
    RUN_TEST(impartial_minimax_test_all());
    RUN_TEST(impartial_sumgame_test_all());

    // Other MCGS features
    RUN_TEST(simple_text_hash_test_all());
    RUN_TEST(file_parser_test_all());

    THROW_ASSERT(argc >= 1);
    RUN_TEST(cli_options_test_all(argv[0]));
    
    RUN_TEST(scale_test_all());
    RUN_TEST(game_bounds_test_all());
    RUN_TEST(find_bounds_test_all());

    RUN_TEST(grid_generator_test_all());
    RUN_TEST(sheep_grid_generator_test_all());

    RUN_TEST(thermograph_helpers_test_all(do_extra_tests));
    RUN_TEST(database_test_all(do_extra_tests));
    RUN_TEST(test_winning_moves());

    cout << "SUCCESS" << endl;
    return 0;
}
