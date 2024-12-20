#pragma once
#include <fstream>
#include <sstream>
#include <string>

class token_iterator;


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
    void close_if_file();

    std::istream& _stream;
    std::stringstream _line_stream;
    std::string _token;

    int _line_number;

};

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


