#pragma once

#include "file_parser.h"
#include <memory>

struct cli_options
{
    cli_options();
    ~cli_options();

    void cleanup();

    std::shared_ptr<file_parser> parser;
    bool dry_run; // Do dry run without running games
    bool should_exit; // Exit from main() i.e. when "--help" used
};


// Parses CLI options and returns them. May or may not include a file_parser
cli_options parse_cli_args(int _argc, char** argv);

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

