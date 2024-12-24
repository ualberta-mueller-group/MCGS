#pragma once
#include <sstream>
#include <string>
#include <vector>
#include "game.h"
#include <unordered_map>
#include "game_token_parsers.h"
#include <memory>

// file_parser checks for a version command when reading from file or stdin
#define FILE_PARSER_VERSION_STRING "version 1.0"

// How many game_cases can be created by a "run" command
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

    virtual std::string get_token() const = 0;
    virtual int line_number() const = 0;
    virtual operator bool() const = 0;
    virtual void operator++() = 0;
};

class file_token_iterator : public token_iterator
{
public:
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

    //std::istream& _main_stream;
    std::stringstream _line_stream;
    std::string _token;

    int _line_number;
};

////////////////////////////////////////////////// game_case

enum test_outcome
{
    TEST_OUTCOME_UNKNOWN = 3,
    TEST_OUTCOME_WIN = (int) true,
    TEST_OUTCOME_LOSS = (int) false,
};


/*
    game_case: 
        Games and other data returned from parsing input with file_parser.
        Games and data are owned by the caller

    Moveable, not copyable. Caller must call cleanup_games() or release_games() before
        destructing. This is to prevent memory bugs, due to game ownership
        being unclear right now
*/
struct game_case
{
    int to_play;
    test_outcome expected_outcome;
    std::vector<game*> games;

    game_case();
    ~game_case();

    // move constructor and move assignment operator
    game_case(game_case&& other) noexcept;
    game_case& operator=(game_case&& other) noexcept;

    
    void cleanup_games(); // delete all games
    void release_games(); // reset game_case to default values without deleting games

private:
    void _move_impl(game_case&& other) noexcept;
};

////////////////////////////////////////////////// file_parser


/*
    file_parser:
        reads input from stdin, string, or file. Use static constructor functions
        to create a file_parser
*/
class file_parser
{
private:
    // constructor should be private, use static functions instead
    file_parser(std::istream* stream, bool delete_stream, bool do_version_check);
    void version_check(const std::string& version_string);
    static void add_game_parser(const std::string& game_title, game_token_parser* gp);

    bool get_enclosed(const char& open, const char& close, bool allow_inner);
    bool match(const char& open, const char& close, const std::string& match_name, bool allow_inner);
    bool parse_game();
    bool parse_command();
    void print_error_start();


public:
    // Prevent accidental memory bugs
    file_parser() = delete;
    file_parser(const file_parser& other) = delete;
    file_parser& operator=(const file_parser& other) = delete;

    ~file_parser();

    // Get next game_case. True if valid case, false if no more cases
    bool parse_chunk(game_case& gc);

    // static constructor functions
    static file_parser* from_stdin();
    static file_parser* from_file(const std::string& file_name);
    static file_parser* from_string(const std::string& string);


private:
    // initializes parsers for every game. Called automatically upon construction of first file_parser
    static void init_game_parsers();

    static std::unordered_map<std::string, std::shared_ptr<game_token_parser>> _game_map;
    
    file_token_iterator _iterator;
    bool _do_version_check;

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


