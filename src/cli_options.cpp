#include "cli_options.h"
#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <type_traits>
#include <vector>
#include <iostream>
#include <memory>
#include <exception>
#include <cstdint>
#include "file_parser.h"
#include "global_options.h"
#include "init_database.h"
#include "string_to_int.h"
#include "utilities.h"
#include <cassert>

using namespace std;

////////////////////////////////////////////////// cli_options
cli_options::cli_options(const string& test_directory)
    : parser(nullptr),
      dry_run(false),
      should_exit(false),
      gen_experiments(false),
      run_tests(false),
      //run_tests_stdin(false),
      use_player(false),
      //print_winning_moves(false),
      test_directory(test_directory),
      outfile_name(cli_options::DEFAULT_TEST_OUTFILE),
      test_timeout(cli_options::DEFAULT_TEST_TIMEOUT),
      play_log_name(),
      db_file_name(cli_options::DEFAULT_RELATIVE_DB_FILE),
      init_database_type(INIT_DATABASE_AUTO),
      db_config_string()
{
}

cli_options::~cli_options()
{
}

////////////////////////////////////////////////// helper functions
namespace {
// Format explanation of options, i.e. when using --help
void print_flag(const string& flag_string, const string& flag_description)
{
    cout << "\t" << flag_string << endl;
    cout << "\t\t" << flag_description << endl;
    cout << endl;
}

void print_help_message(const string& exec_name)
{
    cout << "Usage: " << exec_name << " [flags] [input string]" << endl;
    cout << endl;

    cout << "\tReads input from a quoted string after [flags], "
            "if present, using same syntax as \".test\" files. "
            "See input/info.test for explanation of input syntax.";

    cout << endl;
    cout << endl;

    cout << "Flags:" << endl;
    print_flag("-h, --help", "Print this message and exit.");

    print_flag("--file <file name>", "Read input from <file name>. "
                                     "Input must start with version command. "
                                     "Causes [input string] to be ignored.");

    //print_flag("--print-winning-moves",
    //           "Instead of solving, show winning moves for input sums.");

    print_flag("--play-mcgs",
               "Play games against MCGS. Uses [input string] "
               "or --file to specify games. Games to play must have a run "
               "command, but the specified color of the player to play is "
               "ignored.");

    print_flag("--no-color", "Disable color printing for --play-mcgs.");

    print_flag("--play-log <file name>", "When specified, --play-mcgs "
                                         "logs the game to the specified file");

    print_flag("--stdin",
               "Read input from stdin. Causes [input string] to be ignored.");

    cout << "Optimization flags:" << endl;
    cout << endl;
    cout << "\tThese flags toggle optimizations on/off." << endl;
    cout << endl;

    print_flag(global::simplify_basic_cgt.no_flag(),
               "Don't simplify basic CGT games (integer_game, dyadic_rational, "
               "up_star, switch_game, nimber).");

    print_flag(global::tt_sumgame_idx_bits.flag() + " <# index bits>",
               "How many index bits to use for sumgame's transposition table. "
               "0 disables transposition table. Default: " +
                   global::tt_sumgame_idx_bits.get_default_str() + ".");

    print_flag(
        global::tt_imp_sumgame_idx_bits.flag() + " <# index bits>",
        "How many index bits to use for impartial sumgame's transposition "
        "table. Must be at least 1. Default: " +
            global::tt_imp_sumgame_idx_bits.get_default_str() + ".");

    print_flag(global::use_db.no_flag(), "Disable database usage.");

    print_flag("--db-file-load <file name>", "Load database file. Default is "
                                             "database.bin.");

    print_flag("--db-file-create <file name> <config string>",
               "Create and populate a new database file. See README for "
               "details on config string syntax.");

    // print_flag(global::play_split.no_flag(), "Don't split games after "
    //                                          "playing a move.");

    print_flag(global::play_normalize.no_flag(), "Don't normalize subgames "
                                                 "after playing a move.");

    print_flag(global::dedupe_movegen.no_flag(), "Don't skip move generators "
                                                 "for duplicate subgames.");

    cout << "Misc options flags:" << endl;
    print_flag(global::random_seed.flag(),
               "Set seed for main random generator. "
               "0 means seed with current time "
               "since epoch. Default: " +
                   global::random_seed.get_default_str() + ".");

    print_flag(global::experiment_seed.flag(),
               "Set seed for experiment data "
               "generation. 0 means seed with current time. Default: " +
                   global::experiment_seed.get_default_str());

    //print_flag(global::alt_imp_search.flag(),
    //           "Enable alternative search algorithm for impartial games.");

    cout << "Testing framework flags:" << endl;
    cout << endl;
    cout << "\tThese flags only have an effect when using \"--run-tests\"."
         << endl;
    cout << endl;

    print_flag("--gen-experiments", "Generate .test file for ICGA paper. See "
                                    "gen_experiments.cpp");

    print_flag("--run-tests",
               "Run all autotests. By default, reads tests from \"" +
                   string(cli_options::DEFAULT_RELATIVE_TEST_PATH) +
                   "\". Causes other input (i.e. from file, stdin etc) to be "
                   "ignored.");

    //print_flag("--run-tests-stdin", "Like --run-tests, but read from stdin.");

    print_flag("--test-dir <directory name>",
               "Sets input directory for --run-tests. Default is \"" +
                   string(cli_options::DEFAULT_RELATIVE_TEST_PATH) + "\".");

    print_flag("--out-file <file name>",
               "Name of CSV output file resulting "
               "from --run-tests. Default is \"" +
                   string(cli_options::DEFAULT_TEST_OUTFILE) + "\".");

    print_flag("--test-timeout <timeout in ms>",
               "Set timeout duration for tests, in \
milliseconds. Timeout of 0 means tests never time out. Default is " +
                   to_string(cli_options::DEFAULT_TEST_TIMEOUT) + ".");

    print_flag(global::clear_tt.flag(),
               "Clear ttable between test runs. Default: " +
                   global::clear_tt.get_default_str() + ".");

    print_flag(global::count_sums.flag(), "Count unique sums found during "
               "search. Only applies to partizan solve commands i.e. {B} or "
               "{W}, but not {N}. Will slow down search somewhat.");

    // Remove these? Keep them in this separate section instead?
    cout << "Debugging flags:" << endl;

    print_flag("--dry-run",
               "Skip running games. Has no effect when using \"--run-tests\". "
               "Instead, set the test timeout low (i.e. 1).");

    print_flag("--print-optimizations",
               "Print optimization summary to stdout, then quit.");

    print_flag("--parser-debug", "Print file_parser debug info.");

    print_flag("--assert-correct-version",
               "Quit if a file version is found which doesn't match.");

    print_flag(global::silence_warnings.flag(),
               "Don't print warnings to "
               "stderr, i.e. random_table resize");

    print_flag(global::print_ttable_size.flag(),
               "Print ttable size to stdout.");

    print_flag(global::print_db_info.flag(),
               "Print verbose database info to stdout. Includes metadata of "
               "loaded database file");
}

} // namespace

//////////////////////////////////////// exported functions
cli_options parse_args(int argc, const char** argv, bool silent)
{
    bool print_optimizations = false;

    assert(argc >= 1);

#ifndef __EMSCRIPTEN__
    std::filesystem::path abs_exec_path = std::filesystem::canonical(argv[0]);
    std::filesystem::path parent_path = abs_exec_path.parent_path();
    std::filesystem::path default_test_path =
        parent_path / cli_options::DEFAULT_RELATIVE_TEST_PATH;
#else
    std::filesystem::path default_test_path =
        cli_options::DEFAULT_RELATIVE_TEST_PATH;
#endif

    cli_options opts(default_test_path.string());

    if (argc == 1)
    {
        if (!silent)
        {
            print_help_message(argv[0]);
        }
        opts.should_exit = true;
        return opts;
    }

    vector<string> args;
    for (int i = 0; i < argc; i++)
    {
        args.push_back(string(argv[i]));
    }

    const int arg_n = args.size(); // more correct than using argc

    /*
        TODO break this loop into functions. Maybe make them members of a class
            so they can share arg_idx etc, and have a loop that gets functions
            from an unordered_map<string, function>.
    */
    int arg_idx = 0;
    for (arg_idx = 1; arg_idx < arg_n; arg_idx++) // skip "./MCGS"
    {
        const string& arg = args[arg_idx];
        const string& arg_next = (arg_idx + 1) < arg_n ? args[arg_idx + 1] : "";

        if (arg == "--stdin")
        {
            if (!opts.parser)
            {
                // cout << "Reading game input from stdin" << endl;
                opts.parser =
                    shared_ptr<file_parser>(file_parser::from_stdin());
            }

            continue;
        }

        if (arg == "--file")
        {
            arg_idx++; // consume the file name

            if (arg_next.size() == 0)
            {
                throw cli_options_exception(
                    "Error: got --file but no file path");
            }

            if (!opts.parser)
            {
                // cout << "Reading game input from file: \"" << arg_next <<
                // "\"" << endl;
                opts.parser =
                    shared_ptr<file_parser>(file_parser::from_file(arg_next));
            }

            continue;
        }

        //if (arg == "--print-winning-moves")
        //{
        //    opts.print_winning_moves = true;
        //    continue;
        //}

        if (arg == "--play-mcgs")
        {
            opts.use_player = true;
            continue;
        }

        if (arg == "--play-log")
        {
            arg_idx++;

            if (arg_next.size() == 0)
            {
                throw cli_options_exception(
                    "Error: Got --play-log but no file path");
            }

            opts.play_log_name = arg_next;
            continue;
        }

        if (arg == "--no-color")
        {
            global::player_color.set(false);
            continue;
        }

        if (arg == "-h" || arg == "--help")
        {
            if (!silent)
            {
                print_help_message(args[0]);
            }
            opts.should_exit = true;
            break;
        }

        if (arg == "--dry-run")
        {
            opts.dry_run = true;
            continue;
        }

        if (arg == "--print-optimizations")
        {
            print_optimizations = true;
            opts.should_exit = true;
            continue;
        }

        if (arg == "--parser-debug")
        {
            file_parser::debug_printing = true;
            continue;
        }

        if (arg == "--assert-correct-version")
        {
            file_parser::override_assert_correct_version = true;
            continue;
        }

        if (arg == global::silence_warnings.flag())
        {
            global::silence_warnings.set(true);
            continue;
        }

        if (arg == global::print_ttable_size.flag())
        {
            global::print_ttable_size.set(true);
            continue;
        }

        if (arg == global::print_db_info.flag())
        {
            global::print_db_info.set(true);
            continue;
        }

        if (arg == "--gen-experiments")
        {
            opts.gen_experiments = true;
            continue;
        }

        if (arg == "--run-tests")
        {
            opts.run_tests = true;
            continue;
        }

        //if (arg == "--run-tests-stdin")
        //{
        //    opts.run_tests_stdin = true;
        //    continue;
        //}

        if (arg == "--test-dir")
        {
            arg_idx++;

            if (arg_next.size() == 0)
            {
                throw cli_options_exception(
                    "Error: got --test-dir but no directory");
            }

            opts.test_directory = arg_next;
            continue;
        }

        if (arg == "--out-file")
        {
            arg_idx++;

            if (arg_next.size() == 0)
            {
                throw cli_options_exception(
                    "Error: Got --out-file but no file path");
            }

            opts.outfile_name = arg_next;
            continue;
        }

        if (arg == "--test-timeout")
        {
            arg_idx++;

            if (arg_next.size() == 0)
            {
                throw cli_options_exception(
                    "Error: got --test-timeout but no timeout");
            }
            
            try
            {
                static_assert(std::is_same_v<unsigned long long,
                                             decltype(opts.test_timeout)>);

                opts.test_timeout = str_to_ull(arg_next);
            }
            catch (const exception& exc)
            {
                throw cli_options_exception("Error: --test-timeout argument "
                        "not an unsigned integer, or out of range");
            }

            continue;
        }

        if (arg == global::clear_tt.flag())
        {
            global::clear_tt.set(true);
            continue;
        }

        if (arg == global::count_sums.flag())
        {
            global::count_sums.set(true);
            continue;
        }

        // OPTIMIZATION TOGGLES

        if (arg == global::simplify_basic_cgt.no_flag())
        {
            global::simplify_basic_cgt.set(false);
            continue;
        }

        if (arg == global::tt_sumgame_idx_bits.flag())
        {
            arg_idx++;

            if (arg_next.size() == 0)
            {
                throw cli_options_exception("Error: got " +
                                            global::tt_sumgame_idx_bits.flag() +
                                            " but no value");
            }

            static_assert(sizeof(size_t) >= sizeof(unsigned short));
            unsigned short n_index_bits;

            try
            {
                n_index_bits = str_to_ush(arg_next);
            }
            catch (const exception& exc)
            {
                throw cli_options_exception(
                    "Error: " + global::tt_sumgame_idx_bits.flag() +
                    " value not an unsigned integer, or out of range");
            }

            global::tt_sumgame_idx_bits.set(n_index_bits);
            continue;
        }

        if (arg == global::tt_imp_sumgame_idx_bits.flag())
        {
            arg_idx++;

            if (arg_next.size() == 0)
            {
                throw cli_options_exception("Error: got " +
                                            global::tt_imp_sumgame_idx_bits.flag() +
                                            " but no value");
            }

            static_assert(sizeof(size_t) >= sizeof(unsigned short));
            unsigned short n_index_bits;

            try
            {
                n_index_bits = str_to_ush(arg_next);
            }
            catch (const exception& exc)
            {
                throw cli_options_exception(
                    "Error: " + global::tt_imp_sumgame_idx_bits.flag() +
                    " value not an unsigned integer, or out of range");
            }

            global::tt_imp_sumgame_idx_bits.set(n_index_bits);
            continue;
        }

        if (arg == global::use_db.no_flag())
        {
            global::use_db.set(false);
            continue;
        }

        if (arg == "--db-file-load")
        {
            arg_idx++;

            if (arg_next.empty())
                throw cli_options_exception(
                    "Error: --db-file-load but no file name given");

            if (opts.init_database_type != INIT_DATABASE_AUTO)
                throw cli_options_exception(
                    "Error: --db-file-load used, but another \"--db-file*\" "
                    "option already used");

            opts.init_database_type = INIT_DATABASE_LOAD;
            opts.db_file_name = arg_next;

            continue;
        }

        if (arg == "--db-file-create")
        {
            const int arg_idx_config_string = arg_idx + 2;
            arg_idx += 2;

            if (arg_next.empty())
                throw cli_options_exception(
                    "Error: --db-file-create but no file name given");

            if (arg_idx_config_string >= arg_n)
                throw cli_options_exception(
                    "Error: --db-file-create but no config string given");

            if (opts.init_database_type != INIT_DATABASE_AUTO)
                throw cli_options_exception(
                    "Error: --db-file-create used, but another \"--db-file*\" "
                    "option already used");

            opts.init_database_type = INIT_DATABASE_CREATE;
            opts.db_file_name = arg_next;
            opts.db_config_string = args[arg_idx_config_string];

            continue;
        }


        // if (arg == global::play_split.no_flag())
        //{
        //     global::play_split.set(false);
        //     continue;
        // }

        if (arg == global::play_normalize.no_flag())
        {
            global::play_normalize.set(false);
            continue;
        }

        if (arg == global::dedupe_movegen.no_flag())
        {
            global::dedupe_movegen.set(false);
            continue;
        }

        // MISC OPTIONS
        if (arg == global::random_seed.flag())
        {
            const std::string& flag_text = global::random_seed.flag();

            arg_idx++;
            if (arg_next.size() == 0)
                throw cli_options_exception("Error: got " + flag_text +
                                            " but no seed value");

            const char* str = arg_next.c_str();
            char* str_end = nullptr;
            const char* expected_end = str + arg_next.size();
            int base = 0;

            const uint64_t seed = strtoull(str, &str_end, base);

            if (errno == ERANGE)
            {
                errno = 0;
                throw cli_options_exception("Error: " + flag_text +
                                            " value out of range: \"" +
                                            arg_next + "\"");
            }

            if (str_end != expected_end)
            {
                throw cli_options_exception("Error: " + flag_text +
                                            " value bad format: \"" + arg_next +
                                            "\"");
            }

            global::random_seed.set(seed);

            continue;
        }

        if (arg == global::experiment_seed.flag())
        {
            arg_idx++;

            const char* arg_next_ptr = arg_next.c_str();
            char* end = nullptr;

            const uint64_t seed = strtoull(arg_next_ptr, &end, 10);

            global::experiment_seed.set(seed);
            continue;
        }

        //if (arg == global::alt_imp_search.flag())
        //{
        //    global::alt_imp_search.set(true);
        //    continue;
        //}

        if (arg.size() >= 0 &&                                  //
            LOGICAL_IMPLIES(arg.size() > 0, arg.front() != '-') //
        )
        {
            // the rest of args is input to the file_parser

            // for now it should be quoted, so there should only be one arg at
            // this point...
            if (arg_idx != arg_n - 1)
            {
                string why = "Unexpected arg count: ";
                why += "did you forget to quote game input passed as args?";
                throw cli_options_exception(why);
            }

            if (!opts.parser)
            {
                // cout << "Reading game input from args" << endl;
                const string& input = args[arg_idx];

                opts.parser =
                    shared_ptr<file_parser>(file_parser::from_string(input));
            }

            break;
        }

        {
            string why = "Error: unrecognized flag. See ";
            why += "\"" + args[0] + " --help\" or ";
            why += "\"" + args[0] + " -h\"";

            throw cli_options_exception(why);
        }
    }

    if (print_optimizations)
    {
        cout << "Options: " << endl;
        cout << global_option_base::get_summary_all() << endl;
    }

    return opts;
}
