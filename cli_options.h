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

cli_options parse_cli_args(int _argc, char** argv);
