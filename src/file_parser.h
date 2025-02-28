#pragma once
#include <array>
#include <sstream>
#include <string>
#include <vector>
#include "game.h"
#include <unordered_map>
#include "game_token_parsers.h"
#include <memory>
#include <exception>
#include "simple_text_hash.h"

// file_parser checks for a version command when reading from file or stdin
#define FILE_PARSER_VERSION_STRING "version 1.0"

// How many game_cases can be created by a single "run" command, i.e. "{B win, W loss}"
#define FILE_PARSER_MAX_CASES 2


//////////////////////////////////////// token_iterator

/*
   token_iterators generate string tokens from some input stream, and
    remember line numbers. For use by file_parser
*/
class token_iterator
{
public:
    virtual ~token_iterator() {}

    // get next token, writing it into "token". Returns true iff result is valid
    virtual bool get_token(std::string& token) = 0;

    // line number of previous token returned by get_token()
    virtual int line_number() const = 0;
};

class file_token_iterator : public token_iterator
{
public:

    /*
        if delete_stream is true, the stream is owned by the file_token_iterator;
            i.e. stream might be std::cin and delete_stream will be false,
            or stream might be some std::ifstream and delete_stream will be true
    */
    file_token_iterator(std::istream* stream, bool delete_stream);
    ~file_token_iterator();

    bool get_token(std::string& token) override;
    int line_number() const override;

private:
    void cleanup();

    std::istream* __main_stream_ptr;
    bool _delete_stream; // do we own this stream?

    std::stringstream _line_stream;

    int _line_number;
};


////////////////////////////////////////////////// game_case

// expected outcome of a game_case
enum test_result
{
    TEST_RESULT_UNSPECIFIED = 3,
    TEST_RESULT_WIN = (int) true,
    TEST_RESULT_LOSS = (int) false,
};


inline std::string test_result_to_string(const test_result& outcome)
{
    switch (outcome)
    {
        case TEST_RESULT_UNSPECIFIED:
        {
            return "Unspecified";
            break;
        }

        case TEST_RESULT_WIN:
        {
            return "Win";
            break;
        }

        case TEST_RESULT_LOSS:
        {
            return "Loss";
            break;
        }

        default:
        {
            std::cerr << "test_result_to_string() invalid input: ";
            std::cerr << outcome << std::endl;
            exit(-1); // exit instead of assert (could be due to bad file input)
        }

    }

    
    std::cerr << "This string should not appear: see test_result_to_string()" << std::endl;
    exit(-1);
}

/*
    game_case: 
        Games and other data returned from parsing input with file_parser.
        Games and data within are owned by the caller

    Moveable, not copyable. Caller must call cleanup_games() or release_games() before
        destructing. This is to prevent memory bugs, due to game ownership
        being unclear right now
*/
struct game_case
{
    ebw to_play;
    test_result expected_outcome;
    std::vector<game*> games;
    std::string comments;
    simple_text_hash hash;


    game_case();
    ~game_case();

    // move constructor and move assignment operator
    game_case(game_case&& other) noexcept;
    game_case& operator=(game_case&& other) noexcept;
    
    void cleanup_games(); // delete all games
    void release_games(); // release ownership of games, and reset self to default values

private:
    void _move_impl(game_case&& other) noexcept;
};


////////////////////////////////////////////////// file_parser

/*
    file_parser:
        reads input from stdin, string, or file. Use static constructor functions
        to create a file_parser i.e. from_stdin()

        call parse_chunk() to get next game_case
*/
class file_parser
{
private:
    // constructor should be private, user calls static constructor functions instead
    file_parser(std::istream* stream, bool delete_stream, bool do_version_check);

    void version_check(const std::string& version_string);

    // registers a new game_token_parser, now owned by callee
    static void add_game_parser(const std::string& game_title, game_token_parser* gp);

    // token-generating helper functions
    bool get_enclosed(const std::string& open, const std::string& close, bool allow_inner);
    bool match(const std::string& open, const std::string& close, const std::string& match_name, bool allow_inner);

    // functions to handle current token
    bool parse_game();

    void validate_command(const std::string& token_copy);
    bool parse_command();

    std::string get_error_start();

public:
    // Prevent accidental memory bugs
    file_parser() = delete;
    file_parser(const file_parser& other) = delete;
    file_parser& operator=(const file_parser& other) = delete;

    ~file_parser();

    /*  Get next game_case. True if valid case, false if no more cases.
            Caller must first clean up previous game_case
    */
    bool parse_chunk(game_case& gc);

    // static constructor functions
    static file_parser* from_stdin();
    static file_parser* from_file(const std::string& file_name);
    static file_parser* from_string(const std::string& string);

    // Used by unit tests to check whether a warning was printed
    bool warned_wrong_version();

    // When true, file_parser prints info to stdout as it parses the input
    static bool debug_printing;
    static bool silence_warnings;


private:
    // initializes parsers for every game. Called automatically upon construction of first file_parser
    // this is where new game_token_parsers are registered
    static void init_game_parsers();

    // maps section title to its game_token_parser
    static std::unordered_map<std::string, std::shared_ptr<game_token_parser>> _game_map;
    
    // input source
    file_token_iterator _iterator;

    // when true, complain if input doesn't specify version
    bool _do_version_check;

    // data for current token
    std::string _section_title;
    int _line_number;
    std::string _token;

    /*
        Because the "run" command comes AFTER the games are specified,
            and because games are mutable, the parser has to store multiple
            copies of the game list, i.e. {B, W} requires two separate but
            equal game lists
    */
    game_case _cases[FILE_PARSER_MAX_CASES];
    int _case_count; // number of cases created by a previous parse_chunk() call
    int _next_case_idx; // next case to consume from a previous parse

    bool _warned_wrong_version;
};


enum parser_exception_code
{
    PARSER_OK = 0,
    //WRONG_VERSION_COMMAND = 1, // Wrong version just prints warning
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
        : _why(why + " (Parser exception code " + std::to_string(code) + ")"), _code(code)
    { };

    ~parser_exception() {}

    const char* what() const noexcept override
    {
        return _why.c_str();
    }

    parser_exception_code code() const noexcept
    {
        return _code;
    }

private:
    std::string _why;
    parser_exception_code _code;
    //parser_exception_code code;
};

