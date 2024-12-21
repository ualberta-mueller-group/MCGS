#include "file_parser.h"
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <string>
#include <istream>
#include <fstream>
#include "cgt_basics.h"
#include "all_game_headers.h"
#include "game_token_parsers.h"

// TODO: Version command should be optional, BUT STILL CHECKED, when reading from args

using namespace std;

unordered_map<string, shared_ptr<game_token_parser>> file_parser::_game_map;


//////////////////////////////////////////////////////////// file_token_iterator

file_token_iterator::file_token_iterator(istream& stream)
    : _main_stream(stream), _line_number(0)
{
    next_token();
}

// Since the istream isn't owned by us, don't try closing it
file_token_iterator::~file_token_iterator()
{ }

string file_token_iterator::get_token() const
{
    assert(*this);
    return _token;
}

int file_token_iterator::line_number() const
{
    assert(*this);
    return _line_number;
}

file_token_iterator::operator bool() const
{
    return _token.size() > 0;
}

void file_token_iterator::operator++()
{
    assert(*this);
    next_token();
}

void file_token_iterator::next_token()
{
    _token.clear();

    // Check if current line has more tokens
    if (_line_stream && _line_stream >> _token)
    {
        return;
    }

    if (_line_stream.fail() && !_line_stream.eof())
    {
        cout << "file_token_iterator operator++ line IO error" << endl;
        assert(false);
    }

    // Scroll through the file's lines until we get a token
    string next_line;
    while (_main_stream && getline(_main_stream, next_line) && !_main_stream.fail())
    {
        _line_number++;
        _line_stream = stringstream(next_line);

        if (_line_stream && _line_stream >> _token && !_line_stream.fail())
        {
            return;
        }

        if (_line_stream.fail() && !_line_stream.eof())
        {
            cout << "file_token_iterator operator++ line IO error" << endl;
            assert(false);
        }
    }

    if (_main_stream.fail())
    {
        if (_main_stream.eof())
        {
            //cout << "file_token_iterator successfully reached EOF" << endl;
        } else if (_main_stream.bad())
        {
            cout << "file_token_iterator operator++ file IO error" << endl;
            assert(false);
        }

        _token.clear();
    }
}

//////////////////////////////////////////////////////////// helper functions

/*
    TODO: a lookup table would be much faster
        this function is slow in general...

    [ ] ( ) { } /

    These characters can only appear in opening or closing tags
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

/*
    Check if token is of the format:
        <open>...<close>

        With reserved characters only at the ends

        i.e. for open = '(' and close = ')'

            ( this is a valid string )
            ( this is not { a valid string )
            (( this is also not valid ))
            
*/
bool is_enclosed_format(const string& token, const char& open, const char& close, bool allow_inner)
{
    if (token.size() < 2)
    {
        return false;
    }

    if (token[0] != open || token.back() != close)
    {
        return false;
    }

    if (!allow_inner)
    {
        int N = token.size();
        for (int i = 1; i < N - 1; i++)
        {
            const char& c = token[i];

            if (is_reserved_char(c))
            {
                return false;
            }
        }

    }

    return true;
}

/*
   Expand given token using the token iterator until it is formatted according
       to is_enclosed_format(). Return false if match not found

    i.e. given open = '(' and close = ')', and the input:

    (1 5 3)

    the token expands like this:
    (1
    (1 5
    (1 5 3)
    at which point it is accepted

    The input:
    (1 5 3)[clobber_1xn]

    is invalid and will be rejected because the token will expand like: 
    (1
    (1 5
    (1 5 3)[clobber_1xn]
    which doesn't match
*/
bool file_parser::get_enclosed(const char& open, const char& close, bool allow_inner)
{
    token_iterator& iterator = _iterator;

    if (_token[0] != open)
    {
        return false;
    }

    if (is_enclosed_format(_token, open, close, allow_inner))
    {
        return true;
    }

    while (iterator)
    {
        string new_token = iterator.get_token();
        ++iterator;

        _token += " " + new_token;

        if (is_enclosed_format(_token, open, close, allow_inner))
        {
            return true;
        }

    }

    return false;
}

// remove first and last characters
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

// Private constructor -- use static functions instead
file_parser::file_parser(istream* stream, bool delete_stream, bool do_version_check)
    : _delete_stream(delete_stream), _stream(stream), _do_version_check(do_version_check), _iterator(*stream), _section_title(), _line_number(0)
{ }

void file_parser::close_if_file()
{
    ifstream* file = dynamic_cast<ifstream*>(_stream);

    if (file != nullptr && file->is_open())
    {
        file->close();
    }
}

void file_parser::version_check(const string& version_string)
{
    const string expected = "version " + to_string(FILE_PARSER_VERSION);

    if (version_string != expected)
    {
        cout << "Parser version mismatch. Expected \"" << expected << "\", got: \"";
        cout << version_string << "\"" << endl;

        assert(false);
    }
}

void file_parser::add_game_parser(const string& game_title, game_token_parser* gp)
{
    auto it = _game_map.find(game_title);

    if (it != _game_map.end())
    {
        cout << "Tried to add game parser \"" << game_title << "\" but it already exists" << endl;

        delete gp;
        assert(false);
        return;
    }

    _game_map.insert({game_title, shared_ptr<game_token_parser>(gp)});
}

file_parser::~file_parser()
{
    close_if_file();

    if (_delete_stream && _stream != nullptr)
    {
        delete _stream;
        _delete_stream = false;
        _stream = nullptr;
    }
}

////////////////////////////////////////////////// more helper functions


bool file_parser::match(const char& open, const char& close, const string& match_name, bool allow_inner)
{
    assert(_token.size() > 0);

    if (_token[0] != open)
    {
        return false;
    }

    bool success = get_enclosed(open, close, false);

    if (success)
    {
        cout << "Got " << match_name << ": " << _token << endl;
        strip_enclosing(_token);
        return true;
    }

    print_error_start();
    cout << ": failed to match " << match_name << endl;

    assert(false);
    exit(-1);

    return false;
}

void file_parser::print_error_start()
{
    cout << "Parser error on line " << _line_number << ": ";
}

bool file_parser::parse_game(game_case& gc)
{
    if (_section_title.size() == 0)
    {
        print_error_start();
        cout << "game token found but section title missing" << endl;

        exit(-1);
        return false;
    }

    auto it = _game_map.find(_section_title);

    if (it == _game_map.end())
    {
        print_error_start();
        cout << "game token found, but game parser doesn't exist for section \"";
        cout << _section_title << "\"" << endl;

        exit(-1);
        return false;
    }

    game_token_parser* gp = (it->second).get();

    game* g = gp->parse_game(_token);

    if (g == nullptr)
    {
        print_error_start();
        cout << "game parser for section \"" << _section_title;
        cout << "\" failed to parse game token: \"" << _token << "\"" << endl;

        exit(-1);
        return false;
    }

    gc.games.push_back(g);

    return true;
}


//////////////////////////////////////////////////

bool file_parser::parse_chunk(game_case& gc)
{
    if (gc.games.size() != 0)
    {
        cout << "Parser error: game_case not empty at start of file_parser::parse_chunk()" << endl;
        exit(-1);
    }

    token_iterator& iterator = _iterator;

    while (iterator)
    {
        _line_number = iterator.line_number();
        _token = iterator.get_token();
        ++iterator;

        // Check file version
        if (_do_version_check)
        {
            bool success = get_enclosed('{', '}', false);
            if (!success)
            {
                print_error_start();
                cout << "Failed to match version string command" << endl;
                return false;
            }

            strip_enclosing(_token);

            version_check(_token);
            _do_version_check = false;

            continue;
        }


        // Match command
        if (match('{', '}', "command", false))
        {
            const char player = _token[0];
            if (player != 'B' && player != 'W')
            {
                print_error_start();
                cout << "Expected player B or W in command" << endl;
                exit(-1);
            }

            gc.to_play = char_to_color(player);

            return true;
        }

        // Match title
        if (match('[', ']', "section title", false))
        {
            _section_title = _token;
            continue;
        }

        // Match brackets
        if (match('(', ')', "bracket token", false))
        {
            parse_game(gc);
            continue;
        }

        // Match comment
        if (match('/', '/', "comment", true))
        {
            continue;
        }

        // Must be game token
        cout << "Got simple token: " << _token << endl;
        parse_game(gc);

    }

    return false;
}

file_parser file_parser::from_stdin()
{
    return file_parser(&cin, false, true);
}

file_parser file_parser::from_file(const string& file_name)
{
    return file_parser(new ifstream(file_name), true, true);
}

file_parser file_parser::from_string(const string& string)
{
    return file_parser(new stringstream(string), true, false);
}

void file_parser::init_game_parsers()
{
    assert(_game_map.size() == 0);

    add_game_parser("clobber_1xn",      new basic_parser<clobber_1xn>());
    add_game_parser("nogo_1xn",         new basic_parser<nogo_1xn>());
    add_game_parser("elephants",        new basic_parser<elephants>());
    add_game_parser("nim",              new basic_parser<nim>());

    add_game_parser("integer_game",     new int_parser<integer_game>());
    add_game_parser("nimber",           new int_parser<nimber>());

    add_game_parser("dyadic_rational",  new int2_parser<dyadic_rational>());
    add_game_parser("switch_game",      new int2_parser<switch_game>());

    add_game_parser("up_star",          new up_star_parser());
}
