#pragma once
#include <sstream>
#include <string>
#include <vector>
#include "game.h"
#include <unordered_map>
#include "game_token_parsers.h"
#include <memory>


#define FILE_PARSER_VERSION 1

//////////////////////////////////////// token_iterator

/*
   token_iterators generate string tokens from some input stream,
       for use by file_parser
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
    file_token_iterator(std::istream& stream);
    ~file_token_iterator();

    std::string get_token() const override;
    int line_number() const override;
    operator bool() const override;
    void operator++() override;

private:
    void next_token();

    std::istream& _main_stream;
    std::stringstream _line_stream;
    std::string _token;

    int _line_number;
};

////////////////////////////////////////////////// game_case

struct game_case
{
    int to_play;
    std::vector<game*> games;

    void cleanup_games();
};

////////////////////////////////////////////////// file_parser

class file_parser
{
private:
    // constructor should be private, use static functions instead
    file_parser(std::istream* stream, bool delete_stream, bool do_version_check);
    void close_if_file();
    void version_check(const std::string& version_string);
    static void add_game_parser(const std::string& game_title, game_token_parser* gp);

public:
    // Prevent accidental memory bugs
    // Can change these if _stream is a shared_ptr with a custom deleter
    file_parser() = delete;
    file_parser(const file_parser& other) = delete;
    file_parser& operator=(const file_parser& other) = delete;

    ~file_parser();

    bool parse_chunk(game_case& gc);

    static file_parser from_stdin();
    static file_parser from_file(const std::string& file_name);
    static file_parser from_string(const std::string& string);

    static void init_game_parsers();

private:

    bool get_enclosed(const char& open, const char& close, bool allow_inner);
    bool match(const char& open, const char& close, const std::string& match_name, bool allow_inner);
    bool parse_game(game_case& gc);
    void print_error_start();


    static std::unordered_map<std::string, std::shared_ptr<game_token_parser>> _game_map;
    bool _delete_stream;
    std::istream* _stream;

    bool _do_version_check;
    
    file_token_iterator _iterator;

    std::string _section_title;

    int _line_number;
    std::string _token;
};


