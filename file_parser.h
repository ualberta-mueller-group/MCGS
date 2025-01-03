#pragma once
#include <sstream>
#include <string>
#include <vector>
#include "game.h"
#include <unordered_map>
#include "game_token_parsers.h"
#include <memory>
#include <exception>

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

    // get current token. User should first call the bool operator to check if one exists
    virtual std::string get_token() const = 0;

    // line number of current token
    virtual int line_number() const = 0;

    // true IFF there are more tokens to read
    virtual operator bool() const = 0;

    // advance to next token
    virtual void operator++() = 0;
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

    std::string get_token() const override;
    int line_number() const override;
    operator bool() const override;
    void operator++() override;

private:
    void next_token();
    void cleanup();

    std::istream* __main_stream_ptr;
    bool _delete_stream; // do we own this stream?

    std::stringstream _line_stream;
    std::string _token;

    int _line_number;
};


////////////////////////////////////////////////// game_case

// expected outcome of a game_case
enum test_outcome
{
    TEST_OUTCOME_UNKNOWN = 3,
    TEST_OUTCOME_WIN = (int) true,
    TEST_OUTCOME_LOSS = (int) false,
};


inline std::string test_outcome_to_string(const test_outcome& outcome)
{
    switch (outcome)
    {
        case TEST_OUTCOME_UNKNOWN:
        {
            return "Unknown";
            break;
        }

        case TEST_OUTCOME_WIN:
        {
            return "Win";
            break;
        }

        case TEST_OUTCOME_LOSS:
        {
            return "Loss";
            break;
        }

        default:
        {
            std::cerr << "test_outcome_to_string() invalid input: ";
            std::cerr << outcome << std::endl;
            exit(-1); // exit instead of assert (could be due to bad file input)
        }

    }

    return "This string should not appear: see test_outcome_to_string()";
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
    test_outcome expected_outcome;
    std::vector<game*> games;

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
    bool get_enclosed(const char& open, const char& close, bool allow_inner);
    bool match(const char& open, const char& close, const std::string& match_name, bool allow_inner);

    // functions to handle current token
    bool parse_game();
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

    // When true, file_parser prints info to stdout as it parses the input
    static bool debug_printing;


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
};


// Thrown on bad input
class parser_exception : public std::exception
{
public:
    parser_exception(const std::string& why) : _why(why + " (Parser exception)") 
    { };

    ~parser_exception() {}

    const char* what() const noexcept override
    {
        return _why.c_str();
    }

private:
    std::string _why;
};

