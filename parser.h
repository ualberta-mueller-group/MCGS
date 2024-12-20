#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "game.h"

class token_iterator;

#define PARSER_VERSION 1


//void parse(const std::string& file_name);
void parse(token_iterator& iterator);

class token_iterator
{
public:
    virtual ~token_iterator() {}

    virtual std::string get_token() = 0;
    virtual int line_number() = 0;
    virtual operator bool() = 0;
    virtual void operator++() = 0;
};

class file_token_iterator : public token_iterator
{
public:
    file_token_iterator(std::istream& stream);
    ~file_token_iterator();

    std::string get_token() override;
    int line_number() override;
    operator bool() override;
    void operator++() override;


private:
    void next_token(bool init);

    std::istream& _stream;
    std::stringstream _line_stream;
    std::string _token;

    int _line_number;

};

/*
    This can go away, instead do:
        std::stringstream stream(argv[1]);
        file_token_iterator iterator(stream);
*/
class args_token_iterator : public token_iterator
{
public:
    args_token_iterator(const std::string& args_string);
    ~args_token_iterator();

    std::string get_token() override;
    int line_number() override;
    operator bool() override;
    void operator++() override;


private:
    void next_token(bool init);

    std::stringstream _line_stream;
    std::string _token;
};


struct game_case
{
    int to_play;
    bool expected;
    std::vector<game*> games;

    void cleanup_games();
};


class parser
{
public:
    // Prevent accidental memory bugs
    // Can change these if _stream is a shared_ptr with a custom deleter
    parser() = delete;
    parser& operator=(const parser& other) = delete;
    parser(const parser& other) = delete;

    ~parser();

    bool parse_chunk(game_case& gc);

    static parser from_stdin();
    static parser from_file(const std::string& file_name);
    static parser from_string(const std::string& string);

private:
    parser(std::istream* stream, bool delete_stream, bool do_version_check);
    void close_if_file();
    void version_check(const std::string& version_string);


    bool _delete_stream;
    std::istream* _stream;

    bool _do_version_check;
    
    file_token_iterator _iterator;

    std::string _game_name;
};
