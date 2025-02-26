#include "cli_options.h"
#include <filesystem>
#include <unistd.h>
#include <vector>
#include <iostream>
#include "file_parser.h"
#include "utilities.h"

using namespace std;

bool do_simplification = true;

////////////////////////////////////////////////// cli_options

cli_options::cli_options(const string& test_directory) : parser(nullptr), dry_run(false),
    should_exit(false), run_tests(false),
    test_directory(test_directory),
    outfile_name(cli_options::default_test_outfile),
    test_timeout(cli_options::default_test_timeout)

{ }

cli_options::~cli_options()
{ }

////////////////////////////////////////////////// functions

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

    cout << "\tReads input from a quoted string after [flags], if present, \
using same syntax as \".test\" files. \
See input/info.test for explanation of input syntax.";

    cout << endl;
    cout << endl;

    cout << "Flags:" << endl;
    print_flag("-h, --help", "Print this message and exit.");

    print_flag("--file <file name>", "Read input from <file name>. Input must start \
with version command. Causes [input string] to be ignored.");

    print_flag("--stdin", "Read input from stdin. Causes [input string] to be ignored.");

    cout << "Testing framework flags:" << endl;
    cout << endl;
    cout << "\tThese flags only have an effect when using \"--run-tests\"." << endl;
    cout << endl;

    print_flag("--run-tests", "Run all autotests. By default, reads tests from \""
+ string(cli_options::default_relative_test_path) + "\". Causes other input (i.e. from file, stdin etc) to be ignored.");

    print_flag("--test-dir <directory name>", "Sets input directory for --run-tests. Default is \""
+ string(cli_options::default_relative_test_path) + "\".");

    print_flag("--out-file <file name>", "Name of CSV output file resulting from --run-tests. \
Default is \"" + string(cli_options::default_test_outfile) + "\".");

    print_flag("--test-timeout <timeout in ms>", "Set timeout duration for tests, in \
milliseconds. Timeout of 0 means tests never time out. Default is " + to_string(cli_options::default_test_timeout) + ".");

    // Remove these? Keep them in this separate section instead?
    cout << "Debugging flags:" << endl;
    print_flag("--dry-run", "Skip running games. Has no effect when using \"--run-tests\". Instead, set the test timeout low (i.e. 1).");

    print_flag("--parser-debug", "Print file_parser debug info.");
    print_flag("--no-simplify", "Don't do simplification in sumgame::solve functions");
}

cli_options parse_cli_args(int _argc, const char** argv, bool silent)
{
    assert(_argc >= 1);
    std::filesystem::path abs_exec_path = std::filesystem::canonical(argv[0]);
    std::filesystem::path parent_path = abs_exec_path.parent_path();
    std::filesystem::path default_test_path = parent_path / cli_options::default_relative_test_path;

    cli_options opts(default_test_path.string());

    if (_argc == 1)
    {
        if (!silent)
        {
            print_help_message(argv[0]);
        }
        opts.should_exit = true;
        return opts;
    }



    vector<string> args;
    for (int i = 0; i < _argc; i++)
    {
        args.push_back(string(argv[i]));
    }

    const int argN = args.size(); // more correct than using argc

    /*
        TODO break this loop into functions. Maybe make them members of a class
            so they can share arg_idx etc, and have a loop that gets functions 
            from an unordered_map<string, function>.
    */
    int arg_idx = 0;
    for (arg_idx = 1; arg_idx < argN; arg_idx++) // skip "./MCGS"
    {
        const string& arg = args[arg_idx];
        const string& arg_next = (arg_idx + 1) < argN ? args[arg_idx + 1] : "";

        if (arg == "--stdin")
        {
            if (!opts.parser) 
            {
                //cout << "Reading game input from stdin" << endl;
                opts.parser = shared_ptr<file_parser>(file_parser::from_stdin());
            }

            continue;
        }

        if (arg == "--file")
        {
            arg_idx++; // consume the file name

            if (arg_next.size() == 0)
            {
                throw cli_options_exception("Error: got --file but no file path");
            }

            if (!opts.parser)
            {
                //cout << "Reading game input from file: \"" << arg_next << "\"" << endl;
                opts.parser = shared_ptr<file_parser>(file_parser::from_file(arg_next));
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

        if (arg == "--parser-debug")
        {
            file_parser::debug_printing = true;
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
                throw cli_options_exception("Error: got --test-dir but no directory");
            }

            opts.test_directory = arg_next;
            continue;
        }

        if (arg == "--out-file")
        {
            arg_idx++;

            if (arg_next.size() == 0)
            {
                throw cli_options_exception("Error: Got --out-file but no file path");
            }

            opts.outfile_name = arg_next;
            continue;
        }

        if (arg == "--test-timeout")
        {
            arg_idx++;

            if (arg_next.size() == 0)
            {
                throw cli_options_exception("Error: got --test-timeout but no timeout");
            }

            if (!is_int(arg_next)) 
            {
                throw cli_options_exception("Error: --test-timeout argument not an integer");
            }

            opts.test_timeout = atoi(arg_next.c_str());

            continue;
        }

        if (arg == "--no-simplify")
        {
            arg_idx++;
            do_simplification = false;
            continue;
        }
        
        if (arg.size() > 0 && arg.front() != '-')
        {
            // the rest of args is input to the file_parser

            // for now it should be quoted, so there should only be one arg at this point...
            if (arg_idx != argN - 1)
            {
                string why = "Unexpected arg count: ";
                why += "did you forget to quote game input passed as args?"; 
                throw cli_options_exception(why);
            }

            if (!opts.parser)
            {
                //cout << "Reading game input from args" << endl;
                const string& input = args[arg_idx];

                opts.parser = shared_ptr<file_parser>(file_parser::from_string(input));
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

    return opts;
}
