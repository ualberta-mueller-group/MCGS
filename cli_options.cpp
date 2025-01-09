#include "cli_options.h"
#include <filesystem>
#include <unistd.h>
#include <vector>
#include <iostream>
#include "file_parser.h"
#include "utilities.h"


using namespace std;

////////////////////////////////////////////////// cli_options

cli_options::cli_options() : parser(nullptr), dry_run(false),
    should_exit(false), run_tests(false), case_number(-1)
{ }

cli_options::~cli_options()
{ }

////////////////////////////////////////////////// functions

void print_flag(const string& flag_string, const string& flag_description)
{
    cout << "\t" << flag_string << endl;
    cout << "\t\t" << flag_description << endl;
}

void print_help_message(const string& exec_name)
{
    cout << "Usage: " << exec_name << " [flags] [game cases string]" << endl;
    cout << endl;

    cout << "\tReads game cases from a quoted string after [flags], if present, \
using same format as \".test\" files, but without version command. \
See info.test for explanation of game case format. \
Reading input from stdin or file will cause game cases string \
to be ignored.";

    cout << endl;
    cout << endl;

    cout << "Flags:" << endl;
    print_flag("-h, --help", "Print this message and exit");
    print_flag("--dry-run", "Skip running games");
    print_flag("--stdin", "Read game cases from stdin");
    print_flag("--file <file name>", "Read game cases from <file name>");
    print_flag("--parser-debug", "Print file_parser debug info");
    print_flag("--run-tests", "TODO");
    print_flag("--case", "TODO");
    print_flag("--silent", "TODO");
}

cli_options parse_cli_args(int _argc, char** argv, bool silent)
{
    cli_options opts;

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

        if (arg == "--silent")
        {
            silent = true;
            continue;
        }

        if (arg == "--stdin")
        {
            if (!opts.parser) 
            {
                if (!silent)
                {
                    cout << "Reading game input from stdin" << endl;
                }
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
                if (!silent)
                {
                    cout << "Reading game input from file: \"" << arg_next << "\"" << endl;
                }
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
        
        if (arg == "--reaper")
        {
            throw cli_options_exception("--reaper must be first argument");
        }

        if (arg == "--case")
        {
            if (arg_next.size() == 0)
            {
                string why = "--case but no case number";
                throw cli_options_exception(why);
            }

            if (!is_int(arg_next))
            {
                string why = "--case but case number isn't an integer";
                throw cli_options_exception(why);
            }

            arg_idx++; // consume next arg
            opts.case_number = atoi(arg_next.c_str());

            if (opts.case_number < 0)
            {
                string why = "--case but case number is negative";
                throw cli_options_exception(why);
            }

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
                if (!silent)
                {
                    cout << "Reading game input from args" << endl;
                }
                const string& input = args[argN - 1];

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

    if (opts.case_number != -1 && !opts.parser)
    {
        string why = "--case specified but no case input";
        throw cli_options_exception(why);
    }

    return opts;
}
