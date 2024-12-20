#include "parser.h"
#include <iostream>
#include <cassert>
#include <string>

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

        _token.clear();
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

void strip_enclosing(string& str)
{
    assert(str.size() >= 2);

    str.pop_back();
    str = str.substr(1);
}

//////////////////////////////////////////////////////////// game_case

void game_case::cleanup_games()
{
    for (game* g : games)
    {
        delete g;
    }

    games.clear();
}

////////////////////////////////////////////////// parser implementation


parser::~parser()
{
    close_if_file();

    if (_delete_stream && _stream != nullptr)
    {
        delete _stream;
        _delete_stream = false;
        _stream = nullptr;
    }
}


bool parser::parse_chunk(game_case& gc)
{
    assert(gc.games.size() == 0);

    token_iterator& iterator = _iterator;

    while (iterator)
    {
        int line_number = iterator.line_number();
        string token = iterator.get_token();
        ++iterator;

        // Check file version
        if (_do_version_check)
        {
            bool success = get_enclosed(iterator, token, '{', '}');
            if (!success)
            {
                cout << "Parser error on line " << line_number << ": Failed to match version string command" << endl;
                return false;
            }

            strip_enclosing(token);

            version_check(token);
            _do_version_check = false;

            continue;
        }


        // Match command
        if (token[0] == '{')
        {
            bool success = get_enclosed(iterator, token, '{', '}');

            if (!success)
            {
                cout << "Parser error on line " << line_number << ": Failed to match command" << endl;
                return false;
            }

            cout << "Got command: " << token << endl;

            strip_enclosing(token);

            return true;
            //continue;
        }

        // Match title
        if (token[0] == '[')
        {
            bool success = get_enclosed(iterator, token, '[', ']');

            if (!success)
            {
                cout << "Parser error on line " << line_number << ": Failed to match game title" << endl;
                return false;
            }

            cout << "Got game title: " << token << endl;

            _game_name = token;
            continue;
        }

        // Match brackets
        if (token[0] == '(')
        {
            bool success = get_enclosed(iterator, token, '(', ')');

            if (!success)
            {
                cout << "Parser error on line " << line_number << ": Failed to match bracket token" << endl;
                return false;
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


    return false;

}


parser parser::from_stdin()
{
    return parser(&std::cin, false, true);
}

parser parser::from_file(const std::string& file_name)
{
    return parser(new ifstream(file_name), true, true);
}

parser parser::from_string(const std::string& string)
{
    return parser(new stringstream(string), true, false);
}


parser::parser(std::istream* stream, bool delete_stream, bool do_version_check)
    : _delete_stream(delete_stream), _stream(stream), _do_version_check(do_version_check), _iterator(*stream), _game_name()
{

}


void parser::close_if_file()
{
    ifstream* file = dynamic_cast<ifstream*>(_stream);

    if (file != nullptr && file->is_open())
    {
        file->close();
    }
}

void parser::version_check(const string& version_string)
{
    const string expected = "version " + std::to_string(PARSER_VERSION);

    if (version_string != expected)
    {
        cout << "Parser version mismatch. Expected \"" << expected << "\", got: \"";
        cout << version_string << "\"" << endl;

        assert(false);
    }
}
