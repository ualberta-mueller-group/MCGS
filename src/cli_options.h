#pragma once

#include "file_parser.h"
#include <memory>

/*
    Variables resulting from command line options. "parser" may be nullptr
*/
struct cli_options
{
    cli_options(const std::string& test_directory);
    ~cli_options();

    /*
        Parses CLI options and returns them. May or may not include a file_parser
        When "silent" is true, don't print to stdout (useful for unit testing)
    */
    static cli_options parse_args(int argc, const char** argv, bool silent = false);

    std::shared_ptr<file_parser> parser;
    bool dry_run;     // Do dry run without running games
    bool should_exit; // Exit from main() i.e. when "--help" used

    bool run_tests; // Run autotests

    std::string test_directory;
    std::string outfile_name;        // CSV output file
    unsigned long long test_timeout; // ms

    static constexpr const char* DEFAULT_RELATIVE_TEST_PATH = "input/autotests";
    static constexpr const char* DEFAULT_TEST_OUTFILE = "out.csv";
    static constexpr const unsigned long long DEFAULT_TEST_TIMEOUT = 500;

    uint64_t random_table_seed;
    static constexpr uint64_t DEFAULT_RANDOM_TABLE_SEED = 7753;

    // Static members for optimization options
    struct optimize
    {
        static std::string get_summary();

        // subgame_split
    private:
        static bool _subgame_split;
    public:
        static bool subgame_split() {return _subgame_split;}
        static constexpr bool DEFAULT_SUBGAME_SPLIT = true;

        // simplify_basic_cgt
    private:
        static bool _simplify_basic_cgt;
    public:
        static bool simplify_basic_cgt() {return _simplify_basic_cgt;}
        static constexpr bool DEFAULT_SIMPLIFY_BASIC_CGT = true;

        // tt_sumgame_idx_bits
    private:
        static size_t _tt_sumgame_idx_bits;
    public:
        static size_t tt_sumgame_idx_bits() {return _tt_sumgame_idx_bits;}
        static constexpr size_t DEFAULT_TT_SUMGAME_IDX_BITS = 28;

        friend cli_options;
    };
};


// Thrown on bad input
class cli_options_exception : public std::exception
{
public:
    cli_options_exception(const std::string& why)
        : _why(why + " (CLI options exception)") {};

    ~cli_options_exception() {}

    const char* what() const noexcept override { return _why.c_str(); }

private:
    std::string _why;
};
