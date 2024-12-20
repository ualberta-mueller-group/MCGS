#include "parser.h"
#include <fstream>
#include <iostream>
#include <cassert>
#include <sstream>
#include <string>
#include <istream>

using std::string, std::ifstream, std::cout, std::cin, std::endl, std::stringstream, std::getline;

/*
    (clobber_1xn) <-- game title
    {B win} <-- command

*/

//////////////////////////////////////////////////////////// token_iterator


class token_iterator
{
public:
    virtual ~token_iterator() {}

    virtual operator bool() = 0;
    virtual void operator++() = 0;
    virtual string get_token() = 0;
    virtual int line_number() = 0;
};

class file_token_iterator : public token_iterator
{
public:
    file_token_iterator(const string& file_name);
    ~file_token_iterator();

    operator bool() override;
    void operator++() override;
    string get_token() override;
    int line_number() override;


private:
    ifstream _file_stream;
    stringstream _line_stream;
    string _token;

    int _line_number;

};

file_token_iterator::file_token_iterator(const string& file_name)
    : _file_stream(file_name), _line_number(0)
{
    if (!_file_stream.is_open())
    {
        cout << "Failed to open file: " << file_name << endl;
        return;
    }

    ++(*this);
}


file_token_iterator::~file_token_iterator()
{
    if (_file_stream.is_open())
    {
        _file_stream.close();
    }
}

void file_token_iterator::operator++()
{
    _token.clear();

    // Check if current line has more tokens
    if (_line_stream && _line_stream >> _token)
    {
        return;
    }

    // Scroll through the file's lines until we get a token
    string next_line;
    while (_file_stream && getline(_file_stream, next_line) && !_file_stream.fail())
    {
        _line_number++;
        _line_stream = stringstream(next_line);

        if (_line_stream && _line_stream >> _token && !_line_stream.fail())
        {
            return;
        }
    }

    if (_file_stream.fail())
    {
        if (_file_stream.eof())
        {
            //cout << "file_token_iterator successfully reached EOF" << endl;
        } else if (_file_stream.bad())
        {
            cout << "file_token_iterator operator++ file IO error" << endl;
        }

        _file_stream.close();
        _token.clear();
    }
}


file_token_iterator::operator bool()
{
    return _token.size() > 0;
}

int file_token_iterator::line_number()
{
    return _line_number;
}

string file_token_iterator::get_token()
{
    return _token;
}

//////////////////////////////////////////////////////////// utility functions





bool is_enclosed_format(const string& token, const char& open, const char& close)
{
    if (token.size() < 2)
    {
        return false;
    }

    if (token[0] != open || token.back() != close)
    {
        return false;
    }

    int N = token.size();
    for (int i = 1; i < N - 1; i++)
    {
        const char& c = token[i];

        // TODO check for reserved characters instead
        if (c == open || c == close)
        {
            return false;
        }
    }

    return true;
}

bool get_enclosed(token_iterator& iterator, string& token, const char& open, const char& close)
{
    if (token[0] != open)
    {
        return false;
    }

    if (is_enclosed_format(token, open, close))
    {
        return true;
    }

    while (iterator)
    {
        string new_token = iterator.get_token();
        ++iterator;

        token += " " + new_token;

        if (is_enclosed_format(token, open, close))
        {
            return true;
        }

    }

    return false;
}



//////////////////////////////////////////////////////////// main code

void parse(const string& file_name)
{

    file_token_iterator iterator(file_name);


    while (iterator)
    {

        int line_number = iterator.line_number();
        string token = iterator.get_token();
        ++iterator;


        // Match command
        if (token[0] == '{')
        {
            bool success =  get_enclosed(iterator, token, '{', '}');

            if (!success)
            {
                cout << "Parser error on line " << line_number << ": Failed to match command" << endl;
                return;
            }

            cout << "Got command: " << token << endl;
            continue;
        }

        // Match title
        if (token[0] == '(')
        {
            bool success = get_enclosed(iterator, token, '(', ')');

            if (!success)
            {
                cout << "Parser error on line " << line_number << ": Failed to match game title" << endl;
                return;
            }

            cout << "Got game title: " << token << endl;
            continue;
        }

        // Match quotes
        if (token[0] == '"')
        {
            bool success = get_enclosed(iterator, token, '"', '"');

            if (!success)
            {
                cout << "Parser error on line " << line_number << ": Failed to match quotes" << endl;
                return;
            }

            cout << "Got quotes: " << token << endl;
            continue;
        }

        // Match comment
        if (token[0] == '/')
        {
            bool success = get_enclosed(iterator, token, '/', '/');

            if (!success)
            {
                cout << "Parser error on line " << line_number << ": Failed to match comment" << endl;
            }

            cout << "Got comment: " << token << endl;
            continue;
        }

        // Must be game token
        cout << "Got simple token: " << token << endl;
    }


}
