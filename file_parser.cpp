#include "file_parser.h"
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <string>
#include <istream>
#include <fstream>
#include <utility>
#include "cgt_basics.h"
#include "all_game_headers.h"
#include "game_token_parsers.h"
#include "utilities.h"


/*
    NOTE: must use exit() instead of assert(false) for many file_parser functions;
        many errors will be from bad user input rather than programming bugs
    

    TODO: Version command should be optional, BUT STILL CHECKED, when reading from args
*/

using namespace std;

unordered_map<string, shared_ptr<game_token_parser>> file_parser::_game_map;


//////////////////////////////////////////////////////////// file_token_iterator

file_token_iterator::file_token_iterator(istream* stream, bool delete_stream)
    : __main_stream_ptr(stream), _delete_stream(delete_stream), _line_number(0)
{
    next_token();
}

file_token_iterator::~file_token_iterator()
{
    cleanup();
}

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
    assert(__main_stream_ptr != nullptr);
    _token.clear();

    istream& _main_stream = *__main_stream_ptr;

    // Check if current line has more tokens
    if (_line_stream && _line_stream >> _token)
    {
        return;
    }

    if (_line_stream.fail() && !_line_stream.eof())
    {
        cerr << "file_token_iterator operator++ line IO error" << endl;
        exit(-1);
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
            cerr << "file_token_iterator operator++ line IO error" << endl;
            exit(-1);
        }
    }

    if (_main_stream.fail())
    {
        if (_main_stream.eof())
        {
            //cout << "file_token_iterator successfully reached EOF" << endl;
        //} else if (_main_stream.bad())
        } else
        {
            cerr << "file_token_iterator operator++ file IO error" << endl;
            exit(-1);
        }

        _token.clear();
    }
}

void file_token_iterator::cleanup()
{
    if (_delete_stream && __main_stream_ptr != nullptr)
    {
        // close if it's a file...
        ifstream* file = dynamic_cast<ifstream*>(__main_stream_ptr);

        if (file != nullptr && file->is_open())
        {
            file->close();
        }

        delete __main_stream_ptr;
    }

    __main_stream_ptr = nullptr;
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


// remove first and last characters
void strip_enclosing(string& str)
{
    assert(str.size() >= 2);

    str.pop_back();
    str = str.substr(1);
}

//////////////////////////////////////////////////////////// game_case


game_case::game_case()
{
    release_games(); // sets variables to defaults
}

game_case::~game_case()
{
    // Caller should have cleaned up games...
    assert(games.size() == 0);
}

void game_case::_move_impl(game_case&& other) noexcept
{
    assert(games.size() == 0);

    to_play = std::move(other.to_play);
    expected_outcome = std::move(other.expected_outcome);
    games = std::move(other.games);

    other.release_games();
}

game_case::game_case(game_case&& other) noexcept
{
    _move_impl(std::forward<game_case>(other));
}

game_case& game_case::operator=(game_case&& other) noexcept
{
    _move_impl(std::forward<game_case>(other));

    return *this;
}

void game_case::cleanup_games()
{
    for (game* g : games)
    {
        delete g;
    }

    release_games();
}


// resets game_case, releasing ownership of its games without deleting them
void game_case::release_games()
{
    to_play = EMPTY;
    expected_outcome = TEST_OUTCOME_UNKNOWN;

    games.clear();
}

////////////////////////////////////////////////// file_parser

// Private constructor -- use static functions instead
file_parser::file_parser(istream *stream, bool delete_stream, bool do_version_check)
    : _iterator(stream, delete_stream), _do_version_check(do_version_check),
      _section_title(""), _line_number(0), _token(""), _case_count(0), _next_case_idx(0)
{
    if (_game_map.size() == 0)
    {
        init_game_parsers();
    }
}

void file_parser::version_check(const string& version_string)
{
    const string& expected = FILE_PARSER_VERSION_STRING;

    if (version_string != expected)
    {
        cerr << "Parser version mismatch. Expected \"" << expected << "\", got: \"";
        cerr << version_string << "\"" << endl;


        exit(-1);
    }
}

void file_parser::add_game_parser(const string& game_title, game_token_parser* gp)
{
    if (_game_map.find(game_title) != _game_map.end())
    {
        cerr << "Tried to add game parser \"" << game_title << "\" but it already exists" << endl;

        delete gp;
        exit(-1);
        return;
    }

    _game_map.insert({game_title, shared_ptr<game_token_parser>(gp)});
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

/*
    Try to match the current token to specified "enclosed format". Strips enclosing
        characters

    returns false if no match, true if match, and quits program if
        illegal input (i.e. match should happen but doesn't due to bad user input)
*/
bool file_parser::match(const char& open, const char& close, const string& match_name, bool allow_inner)
{
    assert(_token.size() > 0);

    if (_token[0] != open)
    {
        return false;
    }

    bool success = get_enclosed(open, close, allow_inner);

    if (success)
    {
        cout << "Got " << match_name << ": " << _token << endl;
        strip_enclosing(_token);
        return true;
    }

    print_error_start();
    cerr << ": failed to match " << match_name << endl;

    exit(-1);

    return false;
}

// parse the current token using a game_token_parser, add result to game_cases
bool file_parser::parse_game()
{
    if (_section_title.size() == 0)
    {
        print_error_start();
        cerr << "game token found but section title missing" << endl;

        exit(-1);
        return false;
    }

    auto it = _game_map.find(_section_title);

    if (it == _game_map.end())
    {
        print_error_start();
        cerr << "game token found, but game parser doesn't exist for section \"";
        cerr << _section_title << "\"" << endl;

        exit(-1);
        return false;
    }

    game_token_parser* gp = (it->second).get();

    for (int i = 0; i < FILE_PARSER_MAX_CASES; i++)
    {
        game* g = gp->parse_game(_token);

        if (g == nullptr)
        {
            print_error_start();
            cerr << "game parser for section \"" << _section_title;
            cerr << "\" failed to parse game token: \"" << _token << "\"" << endl;

            exit(-1);
            return false;
        }

        _cases[i].games.push_back(g);
    }

    return true;
}

// parse current token as a command
bool file_parser::parse_command()
{
    // previous cases should have been consumed and reset
    assert(_case_count == 0);

    /*
        The version command is handled elsewhere right now, so the only command
            we have is to run games
    */

    cout << "PARSING COMMAND " << _token << endl;

    // remove commas, then split by whitespace
    {
        const int N = _token.size();

        for (int i = 0; i < N; i++)
        {
            if (_token[i] == ',')
            {
                _token[i] = ' ';
            }
        }
    }

    vector<string> chunks = split_string(_token);
    int chunk_idx = 0;

    int to_play = EMPTY;
    test_outcome expected_outcome = TEST_OUTCOME_UNKNOWN;


    // check if chunks[i] is some allowed word
    auto chunk_is_allowed = [&chunks](int i, const vector<string>& allowed_words) -> bool
    {
        // valid index?
        if (i < 0 || !(i < chunks.size()))
        {
            return false;
        }

        const string& chunk = chunks[i];

        for (const string& allowed : allowed_words)
        {
            if (chunk == allowed)
            {
                return true;
            }
        }

        return false;
    };

    // extract a case out of "chunks"
    auto get_case = [&]() -> bool
    {
        to_play = EMPTY;
        expected_outcome = TEST_OUTCOME_UNKNOWN;

        // player always comes first
        if (chunk_is_allowed(chunk_idx, {"B", "W"}))
        {
            const char c = chunks[chunk_idx][0];
            chunk_idx++;
            to_play = char_to_color(c);
        } else
        {
            return false;
        }

        // optional outcome
        if (chunk_is_allowed(chunk_idx, {"win", "loss"}))
        {
            const string& chunk = chunks[chunk_idx];
            chunk_idx++;
            expected_outcome = (chunk == "win") ? TEST_OUTCOME_WIN : TEST_OUTCOME_LOSS;
        }

        return true;
    };


    // max 2 cases (run command allows 1 or 2 runs)
    for (int i = 0; i < FILE_PARSER_MAX_CASES; i++)
    {
        // are there remaining unconsumed chunks?
        if (!(chunk_idx < chunks.size()))
        {
            break;
        }

        // chunks remain but don't make up a case
        if (!get_case())
        {
            break;
        }

        if (_case_count >= FILE_PARSER_MAX_CASES)
        {
            print_error_start();
            cerr << "run command has too many cases, maximum is: ";
            cerr << FILE_PARSER_MAX_CASES << endl;

            exit(-1);
            return false;
        }

        game_case& gc = _cases[_case_count];
        gc.to_play = to_play;
        gc.expected_outcome = expected_outcome;

        _case_count++;
    }

    // chunks remain but aren't part of a case...
    if (chunk_idx < chunks.size())
    {
        print_error_start();
        cerr << "failed to parse case command" << endl;

        exit(-1);
        return false;
    }

    return true;
}

// print start of parser error text (including line number)
void file_parser::print_error_start()
{
    cerr << "Parser error on line " << _line_number << ": ";
}

file_parser::~file_parser()
{
    for (int i = 0; i < FILE_PARSER_MAX_CASES; i++)
    {
        // calling cleanup() is okay because caller-consumed games were std::moved
        _cases[i].cleanup_games();
    }
}


/*
    Parses next part of the file until a case is found. Returns true and 
        moves case to "gc" if case is
        valid, otherwise returns false indicating no more cases
*/
bool file_parser::parse_chunk(game_case& gc)
{
    if (gc.games.size() != 0)
    {
        cerr << "Parser error: caller's game_case not empty at start of file_parser::parse_chunk()" << endl;
        exit(-1);
    }

    // Check if there's already a case from the previous parse
    if (_next_case_idx < _case_count)
    {
        gc = std::move(_cases[_next_case_idx]);
        _next_case_idx++;
        return true;
    }

    // No remaining cases
    _next_case_idx = 0;
    _case_count = 0;
    for (int i = 0; i < FILE_PARSER_MAX_CASES; i++)
    {
        // calling cleanup() is okay because caller-consumed cases were std::moved
        _cases[i].cleanup_games();
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
                cerr << "Failed to match version string command" << endl;

                exit(-1);
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
            parse_command();

            // the only command is a "run" command, so just return a case.
            // OK if no games were read yet
            gc = std::move(_cases[_next_case_idx]);
            _next_case_idx++;

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
            parse_game();
            continue;
        }

        // Match comment
        if (match('/', '\\', "comment", true))
        {
            continue;
        }

        // Must be game token
        cout << "Got simple token: " << _token << endl;
        parse_game();

    }

    return false;
}

file_parser* file_parser::from_stdin()
{
    return new file_parser(&cin, false, true);
}

file_parser* file_parser::from_file(const string& file_name)
{
    ifstream* stream = new ifstream(file_name);

    if (!stream->is_open())
    {
        cerr << "file_parser failed to open file \"" << file_name << "\"" << endl;
        exit(-1);
    }

    return new file_parser(stream, true, true);
}

file_parser* file_parser::from_string(const string& string)
{
    return new file_parser(new stringstream(string), true, false);
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
