/*
    file_parser class implements the main parsing logic for .test files,
    games passed through stdin, and the game string passed as a CLI argument

    To register a new game type with the parser, see
    file_parser::_init_game_parsers() in the cpp file
*/
#pragma once
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <exception>
#include <iostream>
#include <cstddef>
#include <cstdlib>


#include "file_parser_ast.h"
#include "istream_tokenizer.h"
#include "game_token_parsers.h"
#include "test_case.h"
#include "test_case_enums.h"


////////////////////////////////////////////////// file_parser

namespace file_parser_impl {

enum match_state
{
    // default "0" state
    MATCH_UNKNOWN,

    // token matches opening string
    MATCH_START,

    // token matches both opening and closing strings, and has no illegal chars
    MATCH_FULL,

    // token has illegal chars, or matches opening string but not closing string
    MATCH_ILLEGAL,

    // token not found (not an error state; the requested match doesn't occur)
    MATCH_NOT_FOUND,
};
} // namespace file_parser_impl

/*
    file_parser:
        reads input from stdin, string, or file. Use static constructor
   functions to create a file_parser i.e. from_stdin()

        call parse_chunk() to get next game_case
*/
class file_parser
{
private:
    // constructor should be private, user calls static constructor functions
    // instead
    file_parser(std::istream* stream, bool delete_stream,
                bool do_version_check);

    void _version_check(const std::string& version_string);

    // registers a new game_token_parser, now owned by callee
    static void _add_game_parser(const std::string& game_title,
                                 game_token_parser* gp);

    // called automatically by _add_game_parser
    static void _add_game_parser_impartial(
        const std::string& game_title,
        std::shared_ptr<game_token_parser>& gp_shared);

    // token-generating helper functions
    file_parser_impl::match_state _get_enclosed(const std::string& open,
                                                const std::string& close,
                                                bool allow_inner);

    bool _match(const std::string& open, const std::string& close,
                const std::string& match_name, bool allow_inner);

    // functions to handle current token
    //bool _parse_game();
    bool _parse_command();

    std::string _get_error_start();
    static std::string _get_error_start(int line_number);

public:
    // Prevent accidental memory bugs
    file_parser() = delete;
    file_parser(const file_parser& other) = delete;
    file_parser& operator=(const file_parser& other) = delete;
    file_parser(file_parser&&) = delete;
    file_parser& operator=(file_parser&&) = delete;

    ~file_parser();

    bool parse_chunk();
    std::vector<game*> get_games() const;
    int n_test_cases() const;
    command_type_enum get_test_case_type(int test_case_idx) const;
    std::shared_ptr<i_test_case> get_test_case(int test_case_idx) const;

    // const std::string& file_name() const;


    void print_ast() const;
    static game* construct_game(const std::string& title, int line_number,
                                const std::string& game_token);

    // static constructor functions
    static file_parser* from_stdin();
    static file_parser* from_file(const std::string& file_name);
    static file_parser* from_string(const std::string& string);

    // Used by unit tests to check whether a warning was printed
    bool warned_wrong_version();

    // When true, file_parser prints info to stdout as it parses the input
    static bool debug_printing;
    static bool silence_warnings;
    static bool override_assert_correct_version;

private:
    // initializes parsers for every game. Called automatically upon
    // construction of first file_parser this is where new game_token_parsers
    // are registered
    static void _init_game_parsers();

    // maps section title to its game_token_parser
    static std::unordered_map<std::string, std::shared_ptr<game_token_parser>>
        _game_map;

    std::optional<fp_chunk> _chunk;

    // input source
    istream_tokenizer _tokenizer;

    // when true, complain if input doesn't specify version
    bool _do_version_check;

    // data for current token
    std::string _section_title;
    int _line_number;
    std::string _token;

    bool _warned_wrong_version;
};

enum parser_exception_code
{
    PARSER_OK = 0,
    // Wrong version just prints warning, unless a CLI flag is set
    WRONG_VERSION_COMMAND = 1,
    MISSING_VERSION_COMMAND,
    MISSING_SECTION_TITLE,
    MISSING_SECTION_PARSER,
    DUPLICATE_GAME_PARSER,
    FAILED_MATCH,
    FAILED_GAME_TOKEN_PARSE,
    CASE_LIMIT_EXCEEDED, // shouldn't happen in practice
    EMPTY_COMMAND,
    EMPTY_CASE_COMMAND,
    FAILED_CASE_COMMAND,
    PARSE_CHUNK_CALLER_ERROR,
    BAD_COMMENT_FORMAT,
};

// Thrown on bad input
class parser_exception : public std::exception
{
public:
    parser_exception(const std::string& why, parser_exception_code code)
        : _why(why + " (Parser exception code " + std::to_string(code) + ")"),
          _code(code) {};

    ~parser_exception() {}

    const char* what() const noexcept override { return _why.c_str(); }

    parser_exception_code code() const noexcept { return _code; }

private:
    std::string _why;
    parser_exception_code _code;
};
