#include "cli_options.h"
#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <cstdint>
#include "file_parser.h"
#include "global_options.h"
#include "utilities.h"
#include <cassert>

using namespace std;

////////////////////////////////////////////////// cli_options
cli_options::cli_options(const string& test_directory)
    : parser(nullptr),
      dry_run(false),
      should_exit(false),
      run_tests(false),
      test_directory(test_directory),
      outfile_name(cli_options::DEFAULT_TEST_OUTFILE),
      test_timeout(cli_options::DEFAULT_TEST_TIMEOUT)
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

    cout << "Misc options flags:" << endl;
    print_flag(global::random_seed.flag(),
               "Set seed for main random generator. "
               "0 means seed with current time "
               "since epoch. Default: " +
                   global::random_seed.get_default_str() + ".");

    cout << "Testing framework flags:" << endl;
    cout << endl;
    cout << "\tThese flags only have an effect when using \"--run-tests\"."
         << endl;
    cout << endl;

    print_flag("--run-tests",
               "Run all autotests. By default, reads tests from \"" +
                   string(cli_options::DEFAULT_RELATIVE_TEST_PATH) +
                   "\". Causes other input (i.e. from file, stdin etc) to be "
                   "ignored.");

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
}

} // namespace

//////////////////////////////////////// exported functions
cli_options parse_args(int argc, const char** argv, bool silent)
{
    bool print_optimizations = false;

    assert(argc >= 1);
    std::filesystem::path abs_exec_path = std::filesystem::canonical(argv[0]);
    std::filesystem::path parent_path = abs_exec_path.parent_path();
    std::filesystem::path default_test_path =
        parent_path / cli_options::DEFAULT_RELATIVE_TEST_PATH;

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

        if (arg == "--run-tests")
        {
            opts.run_tests = true;
            continue;
        }

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

            if (!is_int(arg_next))
            {
                throw cli_options_exception(
                    "Error: --test-timeout argument not an integer");
            }

            opts.test_timeout = atoi(arg_next.c_str());

            continue;
        }

        if (arg == global::clear_tt.flag())
        {
            global::clear_tt.set(true);
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

            if (!is_int(arg_next))
                throw cli_options_exception(
                    "Error: " + global::tt_sumgame_idx_bits.flag() +
                    " value not an int");

            const size_t n_index_bits = atoi(arg_next.c_str());

            global::tt_sumgame_idx_bits.set(n_index_bits);
            continue;
        }

        if (arg == global::tt_imp_sumgame_idx_bits.flag())
        {
            arg_idx++;

            if (!is_int(arg_next))
                throw cli_options_exception(
                    "Error: " + global::tt_imp_sumgame_idx_bits.flag() +
                    " value not an int");

            const size_t n_index_bits = atoi(arg_next.c_str());

            global::tt_imp_sumgame_idx_bits.set(n_index_bits);
            continue;
        }

        if (arg == global::use_db.no_flag())
        {
            global::use_db.set(false);
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

        if (arg.size() > 0 && arg.front() != '-')
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
