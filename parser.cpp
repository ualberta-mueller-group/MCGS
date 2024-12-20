#include "parser.h"
#include <iostream>
#include <cassert>

using std::string, std::ifstream, std::cout, std::cin, std::endl, std::stringstream, std::getline, std::istream;

/*
    (clobber_1xn) <-- game title
    {B win} <-- command

*/

//////////////////////////////////////////////////////////// file_token_iterator

file_token_iterator::file_token_iterator(istream& stream)
    : _stream(stream), _line_number(0)
{

    next_token(true);
}

file_token_iterator::~file_token_iterator()
{
    close_if_file();
}

string file_token_iterator::get_token()
{
    assert(*this);
    return _token;
}

int file_token_iterator::line_number()
{
    return _line_number;
}

file_token_iterator::operator bool()
{
    return _token.size() > 0;
}

void file_token_iterator::operator++()
{
    next_token(false);
}


void file_token_iterator::next_token(bool init)
{
    if (!init)
    {
        assert(*this);
    }

    _token.clear();

    // Check if current line has more tokens
    if (_line_stream && _line_stream >> _token)
    {
        return;
    }

    // Scroll through the file's lines until we get a token
    string next_line;
    while (_stream && getline(_stream, next_line) && !_stream.fail())
    {
        _line_number++;
        _line_stream = stringstream(next_line);

        if (_line_stream && _line_stream >> _token && !_line_stream.fail())
        {
            return;
        }
    }

    if (_stream.fail())
    {
        if (_stream.eof())
        {
            //cout << "file_token_iterator successfully reached EOF" << endl;
        } else if (_stream.bad())
        {
            cout << "file_token_iterator operator++ file IO error" << endl;
        }

        close_if_file();
        _token.clear();
    }
}


void file_token_iterator::close_if_file()
{
    ifstream* file = dynamic_cast<ifstream*>(&_stream);

    if (file != nullptr && file->is_open())
    {
        file->close();
    }
}

//////////////////////////////////////////////////////////// args_token_iterator

args_token_iterator::args_token_iterator(const std::string& args_string)
    : _line_stream(args_string)
{ 
    next_token(true);
}

args_token_iterator::~args_token_iterator()
{ }

string args_token_iterator::get_token()
{
    assert(*this);
    return _token;
}

int args_token_iterator::line_number()
{
    assert(*this);
    return 1;
}

args_token_iterator::operator bool()
{
    return _token.size() > 0;
}

void args_token_iterator::operator++()
{
    next_token(false);
}

void args_token_iterator::next_token(bool init)
{
    if (!init)
    {
        assert(*this);
    }

    _token.clear();

    if (_line_stream)
    {
        _line_stream >> _token;
    }
}

//////////////////////////////////////////////////////////// utility functions

/*
    TODO: a lookup table would be much faster
        this function is slow in general...

    [ ] ( ) { } /
*/
bool is_reserved_char(const char& c)
{
    if (
        c == '[' ||
        c == ']' ||
        c == '(' ||
        c == ')' ||
        c == '{' ||
        c == '}' ||
        c == '/'
        )
    {
        return true;
    }

    return false;
}


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

        if (is_reserved_char(c))
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

//void parse(const string& file_name)
void parse(token_iterator& iterator)
{

    //file_token_iterator iterator(file_name);

    /*
    while (iterator)
    {
        cout << iterator.get_token() << endl;
        ++iterator;
    }
    cout << endl;
    */

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
        if (token[0] == '[')
        {
            bool success = get_enclosed(iterator, token, '[', ']');

            if (!success)
            {
                cout << "Parser error on line " << line_number << ": Failed to match game title" << endl;
                return;
            }

            cout << "Got game title: " << token << endl;
            continue;
        }

        // Match brackets
        if (token[0] == '(')
        {
            bool success = get_enclosed(iterator, token, '(', ')');

            if (!success)
            {
                cout << "Parser error on line " << line_number << ": Failed to match bracket token" << endl;
                return;
            }

            cout << "Got bracket token: " << token << endl;
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
