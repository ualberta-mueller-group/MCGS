#pragma once

#include "file_parser.h"
#include <memory>

namespace cli_options_global {

extern bool do_simplification;
} // namespace cli_options_global 

/*
    Variables resulting from command line options. "parser" may be nullptr
*/
struct cli_options
{
    cli_options(const std::string& test_directory);
    ~cli_options();

    std::shared_ptr<file_parser> parser;
    bool dry_run; // Do dry run without running games
    bool should_exit; // Exit from main() i.e. when "--help" used

    bool run_tests; // Run autotests

    std::string test_directory;
    std::string outfile_name; // CSV output file
    unsigned long long test_timeout; // ms


    static constexpr const char* default_relative_test_path = "input/autotests";
    static constexpr const char* default_test_outfile = "out.csv";
    static constexpr const unsigned long long default_test_timeout = 500;
};


/*
    Parses CLI options and returns them. May or may not include a file_parser

    When "silent" is true, dont print to stdout (useful for unit testing)
*/
cli_options parse_cli_args(int _argc, const char** argv, bool silent = false);

// Thrown on bad input
class cli_options_exception : public std::exception
{
public:
    cli_options_exception(const std::string& why) : _why(why + " (CLI options exception)") 
    { };

    ~cli_options_exception() {}

    const char* what() const noexcept override
    {
        return _why.c_str();
    }




private:
    std::string _why;
};

