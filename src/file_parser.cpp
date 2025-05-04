#include "file_parser.h"
#include <cstdlib>
#include <functional>
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
#include <ios>

/*
    NOTE: here we should usually throw instead of using assert() for many
        file_parser functions; many errors will be from bad user input rather
   than programming bugs, and exceptions also make unit testing easier

    Version command is optional, BUT STILL CHECKED, when reading from string
*/

using namespace std;

using namespace file_parser_impl;

//////////////////////////////////////////////////////////// static members
bool file_parser::debug_printing = false;
bool file_parser::silence_warnings = false;
bool file_parser::override_assert_correct_version = false;

unordered_map<string, shared_ptr<game_token_parser>> file_parser::_game_map;

//////////////////////////////////////////////////////////// file_token_iterator

file_token_iterator::file_token_iterator(istream* stream, bool delete_stream)
    : _main_stream_ptr(stream),
      _delete_stream(delete_stream),
      _line_number(-1),
      _token_buffer(),
      _token_idx(0)
{
    _token_buffer.reserve(8);
}

file_token_iterator::~file_token_iterator()
{
    _cleanup();
}

int file_token_iterator::line_number() const
{
    // Don't call without getting a valid token first
    assert(_line_number >= 0);

    return _line_number;
}

bool file_token_iterator::get_token(string& token)
{
    token.clear();

    // Check if token buffer has data
    if (_token_idx < _token_buffer.size())
    {
        const token_info& ti = _token_buffer[_token_idx];
        _token_idx++;

        _line_number = ti.line_number;
        token = ti.token_string;

        return true;
    }

    // Check if stream has more tokens
    if (_get_token_from_stream(token))
    {
        _token_buffer.emplace_back(token, _line_number);
        _token_idx++;

        assert(_token_idx == _token_buffer.size());

        return true;
    }

    return false;
}

bool file_token_iterator::_get_token_from_stream(string& token)
{
    assert(_main_stream_ptr != nullptr);
    token.clear();

    istream& main_stream = *_main_stream_ptr;

    // Check if current line has more tokens
    if (_line_stream && _line_stream >> token)
    {
        return true;
    }

    if (_line_stream.fail() && !_line_stream.eof())
    {
        throw ios_base::failure("file_token_iterator operator++ line IO error");
    }

    // Scroll through the file's lines until we get a token
    string next_line;
    while (main_stream && getline(main_stream, next_line) &&
           !main_stream.fail())
    {
        _line_number++;
        _line_stream = stringstream(next_line);

        if (_line_stream && _line_stream >> token && !_line_stream.fail())
        {
            return true;
        }

        if (_line_stream.fail() && !_line_stream.eof())
        {
            throw ios_base::failure(
                "file_token_iterator operator++ line IO error");
        }
    }

    if (main_stream.fail() && !main_stream.eof())
    {
        throw ios_base::failure("file_token_iterator operator++ file IO error");
    }

    // no remaining tokens
    return false;
}

void file_token_iterator::consume()
{
    vector<token_info> new_buffer;
    new_buffer.reserve(_token_buffer.size() - _token_idx);

    for (size_t i = _token_idx; i < _token_buffer.size(); i++)
    {
        new_buffer.push_back(_token_buffer[i]);
    }

    _token_buffer.clear();
    _token_buffer = std::move(new_buffer);
    _token_idx = 0;
}

void file_token_iterator::rewind()
{
    _token_idx = 0;
}

void file_token_iterator::_cleanup()
{
    if (_delete_stream && _main_stream_ptr != nullptr)
    {
        // close if it's a file...
        ifstream* file = dynamic_cast<ifstream*>(_main_stream_ptr);

        if (file != nullptr && file->is_open())
        {
            file->close();
        }

        delete _main_stream_ptr;
    }

    _main_stream_ptr = nullptr;
}

//////////////////////////////////////////////////////////// helper functions
namespace {
inline bool match_state_conclusive(match_state state)
{
    return state == MATCH_NOT_FOUND || state == MATCH_FULL ||
           state == MATCH_ILLEGAL;
};

/*
    These characters can only appear in opening or closing tags
*/
bool is_reserved_char(const char& c1, const char& c2)
{
    if (                            //
        c1 == '[' ||                //
        c1 == ']' ||                //
        c1 == '(' ||                //
        c1 == ')' ||                //
        c1 == '{' ||                //
        c1 == '}' ||                //
        (c1 == '/' && c2 == '*') || //
        (c1 == '*' && c2 == '/')    //
        )                           //
    {
        return true;
    }

    return false;
}

/*
    Check if reserved characters occur in the token outside of "open" and
   "close"
*/
bool invalid_reserved_chars(const string& token, const string& open,
                            const string& close)
{
    const size_t N = token.size();
    const size_t open_size = open.size();
    const size_t close_size = close.size();

    assert(N >= open_size + close_size); // token has enough chars

    assert(N >= close_size); // no subtraction underflow

    for (size_t i = open_size; i < N - close_size; i++)
    {
        // NOTE: if we change the input format, we may need 2 or more lookahead
        // chars
        const char& c1 = token[i];
        const char& c2 = i + 1 < N - close_size ? token[i + 1] : 0;

        if (is_reserved_char(c1, c2))
        {
            return true;
        }
    }

    return false;
}

// remove first and last characters
void strip_enclosing(string& str, const string& open, const string& close)
{
    const size_t str_size = str.size();
    const size_t open_size = open.size();
    const size_t close_size = close.size();
    assert(str.size() >= open_size + close_size);

    str = str.substr(open_size, str_size - open_size - close_size);
}

} // namespace

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
    expected_outcome = TEST_RESULT_UNSPECIFIED;

    games.clear();

    comments.clear();
    hash.clear();
}

void game_case::_move_impl(game_case&& other) noexcept
{
    assert(games.size() == 0);

    to_play = std::move(other.to_play);
    expected_outcome = std::move(other.expected_outcome);
    games = std::move(other.games);
    comments = std::move(other.comments);
    hash = std::move(other.hash);

    other.release_games();
}

////////////////////////////////////////////////// file_parser

// Private constructor -- use static functions instead
file_parser::file_parser(istream* stream, bool delete_stream,
                         bool do_version_check)
    : _iterator(stream, delete_stream),
      _do_version_check(do_version_check),
      _section_title(""),
      _line_number(0),
      _token(""),
      _case_count(0),
      _next_case_idx(0),
      _warned_wrong_version(false)
{
    if (_game_map.size() == 0)
    {
        _init_game_parsers();
    }
}

void file_parser::_version_check(const string& version_string)
{
    const string& expected = FILE_PARSER_VERSION_STRING;

    if (version_string != expected)
    {
        if (!file_parser::silence_warnings)
        {
            cerr << "WARNING: Parser version mismatch. Expected \"" + expected +
                        "\", got: \"";
            cerr << version_string << "\"" << endl;
        }
        _warned_wrong_version = true;

        if (override_assert_correct_version)
        {
            throw parser_exception("Wrong version", WRONG_VERSION_COMMAND);
        }
    }
}

void file_parser::_add_game_parser(const string& game_title,
                                   game_token_parser* gp)
{
    assert(gp != nullptr);

    if (_game_map.find(game_title) != _game_map.end())
    {
        delete gp;

        string why = "Tried to add game parser \"" + game_title +
                     "\" but it already exists";
        throw parser_exception(why, DUPLICATE_GAME_PARSER);
    }

    _game_map.insert({game_title, shared_ptr<game_token_parser>(gp)});
}

/*
   Expand given token using the token iterator until it matches <open>...<close>
   or no match is possible.

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
match_state file_parser::_get_enclosed(const string& open, const string& close,
                                       bool allow_inner)
{
    token_iterator& iterator = _iterator;
    match_state state = MATCH_UNKNOWN;
    const size_t n_enclosing_chars = open.size() + close.size();

    std::function<void()> update_match_state = [&]() -> void
    {
        assert(!match_state_conclusive(state));

        if (state == MATCH_UNKNOWN)
        {
            if (_token.size() < open.size())
            {
                return;
            }
            state = string_starts_with(_token, open) ? MATCH_START
                                                     : MATCH_NOT_FOUND;

            if (state == MATCH_START)
            {
                update_match_state(); // could have full match now...
            }

            return;
        }

        if (state == MATCH_START)
        {
            if (_token.size() >= n_enclosing_chars &&
                string_ends_with(_token, close))
            {
                state = MATCH_FULL;

                if (!allow_inner && invalid_reserved_chars(_token, open, close))
                {
                    state = MATCH_ILLEGAL;
                }
            }

            return;
        }

        assert(false);
    };

    update_match_state();
    if (match_state_conclusive(state))
    {
        return state;
    }

    string new_token;
    while (iterator.get_token(new_token))
    {
        assert(!match_state_conclusive(state));

        _token += " " + new_token;

        update_match_state();
        if (match_state_conclusive(state))
        {
            return state;
        }
    }

    // Now handle EOF
    assert(state == MATCH_START || state == MATCH_UNKNOWN);

    if (state == MATCH_START)
    {
        return MATCH_ILLEGAL;
    }

    return MATCH_NOT_FOUND;
}

/*
    Expand current token to match the format: <open><content><close>

    On success: Returns true, consumes file tokens used, and
        assigns _token to <content> (i.e. strips enclosing characters)

    On failure: If illegal input, throws an exception. If no illegal input and
   match doesn't occur, returns false, rewinds the token stream, and leaves
   _token as it was before
*/
bool file_parser::_match(const string& open, const string& close,
                         const string& match_name, bool allow_inner)
{
    assert(_token.size() > 0);

    string token_copy = _token;

    match_state state = _get_enclosed(open, close, allow_inner);
    assert(match_state_conclusive(state));

    if (state == MATCH_FULL)
    {
        if (file_parser::debug_printing)
        {
            cout << "Got " << match_name << ": " << _token << endl;
        }
        strip_enclosing(_token, open, close);

        _iterator.consume();
        return true;
    }

    if (state == MATCH_NOT_FOUND)
    {
        _iterator.rewind();
        _token = token_copy;
        return false;
    }

    assert(state == MATCH_ILLEGAL);

    string why = _get_error_start() + "failed to match " + match_name;
    throw parser_exception(why, FAILED_MATCH);

    return false;
}

// parse the current token using a game_token_parser, add result to game_cases
bool file_parser::_parse_game()
{
    if (_section_title.size() == 0)
    {
        string why =
            _get_error_start() + "game token found but section title missing";
        throw parser_exception(why, MISSING_SECTION_TITLE);

        return false;
    }

    auto it = _game_map.find(_section_title);

    if (it == _game_map.end())
    {
        string why =
            _get_error_start() +
            "game token found, but game parser doesn't exist for section \"";
        why += _section_title + "\"";
        throw parser_exception(why, MISSING_SECTION_PARSER);

        return false;
    }

    game_token_parser* gp = (it->second).get();

    for (int i = 0; i < FILE_PARSER_MAX_CASES; i++)
    {
        game* g = gp->parse_game(_token);

        if (g == nullptr)
        {

            // This won't leak memory when caught because the game_cases
            // will be cleaned up when the file_parser is destructed
            string why = _get_error_start() + "game parser for section \"" +
                         _section_title;
            why += "\" failed to parse game token: \"" + _token + "\"";
            throw parser_exception(why, FAILED_GAME_TOKEN_PARSE);

            return false;
        }
        else
        {
            _cases[i].games.push_back(g);

            // Update hash. Should include section title for every token
            string hashable_chunk = _section_title + _token;
            _cases[i].hash.update(hashable_chunk);
        }
    }

    return true;
}

void file_parser::_validate_command(const string& token_copy)
{
    // First strip whitespace from the input token, where appropriate
    stringstream stream(token_copy);
    string got = "";

    {
        bool first_chunk = true;

        string chunk;
        while (stream >> chunk)
        {
            if (!first_chunk)
            {
                got += " ";
            }
            first_chunk = false;

            got += chunk;
        }
    }

    // Generate expected format
    string expected = "";
    for (int i = 0; i < _case_count; i++)
    {
        game_case& gc = _cases[i];

        string player(1, color_char(gc.to_play));
        test_result result = gc.expected_outcome;

        expected += player;

        if (result != TEST_RESULT_UNSPECIFIED)
        {
            expected += " ";
            expected += ((result == TEST_RESULT_WIN) ? "win" : "loss");
        }

        if (i + 1 < _case_count)
        {
            expected += ", ";
        }
    }

    if (got != expected)
    {
        string why = _get_error_start();
        why += "invalid command format";
        throw parser_exception(why, parser_exception_code::FAILED_CASE_COMMAND);
    }
}

// parse current token as a command
bool file_parser::_parse_command()
{
    const string token_copy = _token;

    // previous cases should have been consumed and reset
    assert(_case_count == 0);

    /*
        The version command is handled elsewhere right now, so the only command
            we have is to run games
    */

    if (file_parser::debug_printing)
    {
        cout << "PARSING COMMAND " << _token << endl;
    }

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
    size_t chunk_idx = 0;

    int to_play = EMPTY;
    test_result expected_outcome = TEST_RESULT_UNSPECIFIED;

    // check if chunks[i] is some allowed word
    auto chunk_is_allowed =
        [&chunks](size_t i, const vector<string>& allowed_words) -> bool
    {
        // valid index?
        // if (i < 0 || !(i < chunks.size()))
        if (!(i < chunks.size()))
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
        expected_outcome = TEST_RESULT_UNSPECIFIED;

        // player always comes first
        if (chunk_is_allowed(chunk_idx, {"B", "W"}))
        {
            const char c = chunks[chunk_idx][0];
            chunk_idx++;
            to_play = char_to_color(c);
        }
        else
        {
            return false;
        }

        // optional outcome
        if (chunk_is_allowed(chunk_idx, {"win", "loss"}))
        {
            const string& chunk = chunks[chunk_idx];
            chunk_idx++;
            expected_outcome =
                (chunk == "win") ? TEST_RESULT_WIN : TEST_RESULT_LOSS;
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
            string why = _get_error_start() +
                         "run command has too many cases, maximum is: ";
            why += to_string(FILE_PARSER_MAX_CASES);
            throw parser_exception(why, CASE_LIMIT_EXCEEDED);

            return false;
        }

        game_case& gc = _cases[_case_count];
        gc.to_play = to_play;
        gc.expected_outcome = expected_outcome;

        // Update hash
        string player_string = string(1, gc.to_play);
        string hashable_chunk = "PLAYER" + player_string;
        gc.hash.update(hashable_chunk);

        _case_count++;
    }

    // chunks remain but aren't part of a case...
    if (chunk_idx < chunks.size())
    {
        string why = _get_error_start() + "failed to parse case command";
        throw parser_exception(why, FAILED_CASE_COMMAND);

        return false;
    }

    if (_case_count == 0)
    {
        string why = _get_error_start() + "\"run\" command with no cases";
        throw parser_exception(why, EMPTY_CASE_COMMAND);
    }

    _validate_command(token_copy);

    return true;
}

// print start of parser error text (including line number)
string file_parser::_get_error_start()
{
    return "Parser error on line " + to_string(_line_number) + ": ";
}

file_parser::~file_parser()
{
    for (int i = 0; i < FILE_PARSER_MAX_CASES; i++)
    {
        // calling cleanup() is okay because caller-consumed games were
        // std::moved
        _cases[i].cleanup_games();
    }
}

/*
    Parses next part of the file until a case is found. Returns true and
        std::moves case to "gc" if case is
        valid, otherwise returns false indicating no more cases
*/
bool file_parser::parse_chunk(game_case& gc)
{
    if (gc.games.size() != 0)
    {
        throw parser_exception("Parser error: caller's game_case not empty at "
                               "start of file_parser::parse_chunk()",
                               PARSE_CHUNK_CALLER_ERROR);
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
        // calling cleanup() is okay because caller-consumed cases were
        // std::moved
        _cases[i].cleanup_games();
    }

    token_iterator& iterator = _iterator;

    while (iterator.get_token(_token))
    {
        _line_number = iterator.line_number();
        iterator.consume();

        // Check version (for file)
        if (_do_version_check)
        {
            // bool success = get_enclosed('{', '}', false);
            bool success = _match("{", "}", "command", false);

            if (!success || _token.find("version") != 0)
            {
                string why =
                    _get_error_start() + "Failed to match version command";
                throw parser_exception(why, MISSING_VERSION_COMMAND);

                return false;
            }

            _version_check(_token);
            _do_version_check = false;

            continue;
        }

        // Match command
        if (_match("{", "}", "command", false))
        {
            if (_token.find("version") == 0)
            {
                _version_check(_token);
                continue;
            }

            _parse_command();

            // the only command is a "run" command, so just return a case.
            // OK if no games were read yet -- this is a "0" game

            assert(_next_case_idx == 0);
            assert(_case_count > 0);

            gc = std::move(_cases[_next_case_idx]);
            _next_case_idx++;

            return true;
        }

        // Match title
        if (_match("[", "]", "section title", false))
        {
            _section_title = _token;
            continue;
        }

        // Match brackets
        if (_match("(", ")", "bracket token", false))
        {
            _parse_game();
            continue;
        }

        // Match comment
        if (_match("/*", "*/", "comment", true))
        {
            // next lines assume the case number is 1 digit
            static_assert(FILE_PARSER_MAX_CASES < 10);

            // ignore comments starting with "_"
            if (_token.size() > 0 && _token[0] != '_')
            {
                // i.e. "#0", "#1" to only include a comment in a specific case
                if (_token[0] == '#')
                {
                    // Get case number...
                    int case_idx = -1;

                    if (_token.size() >= 2)
                    {
                        case_idx = (int) (_token[1] - '0');
                    }

                    if (                                            //
                        !(case_idx >= 0 && case_idx <= 9)           //
                        || case_idx >= FILE_PARSER_MAX_CASES        //
                        || (_token.size() >= 3 && _token[2] != ' ') //
                        )                                           //
                    {
                        string why = "Comment with '#' missing or bad number";
                        throw parser_exception(why, BAD_COMMENT_FORMAT);
                    }

                    string remaining =
                        _token.size() > 2 ? _token.substr(2) : "";
                    _cases[case_idx].comments += remaining;
                }
                else
                {

                    for (int i = 0; i < FILE_PARSER_MAX_CASES; i++)
                    {
                        _cases[i].comments += _token;
                    }
                }
            }

            continue;
        }

        // Must be game token
        if (file_parser::debug_printing)
        {
            cout << "Got simple token: " << _token << endl;
        }
        _parse_game();
    }

    // no more cases
    return false;
}

file_parser* file_parser::from_stdin()
{
    return new file_parser(&cin, false, false);
}

file_parser* file_parser::from_file(const string& file_name)
{
    ifstream* stream = new ifstream(file_name);

    if (!stream->is_open())
    {
        delete stream;
        throw ios_base::failure("file_parser failed to open file \"" +
                                file_name + "\"");
    }

    return new file_parser(stream, true, true);
}

file_parser* file_parser::from_string(const string& string)
{
    return new file_parser(new stringstream(string), true, false);
}

bool file_parser::warned_wrong_version()
{
    return _warned_wrong_version;
}

/*
    When implementing a new game, you must register its game parser here

    i.e:
        add_game_parser("clobber_1xn", new basic_parser<clobber_1xn>());

    Will cause games in the section denoted by "[clobber_1xn]" to be
        created as the clobber_1xn class.

*/
void file_parser::_init_game_parsers()
{
    assert(_game_map.size() == 0);

    _add_game_parser("clobber_1xn", new basic_parser<clobber_1xn>());
    _add_game_parser("nogo_1xn", new basic_parser<nogo_1xn>());
    _add_game_parser("elephants", new basic_parser<elephants>());

    _add_game_parser("integer_game", new int_parser<integer_game>());
    _add_game_parser("nimber", new int_parser<nimber>());

    _add_game_parser("dyadic_rational", new dyadic_rational_parser());
    _add_game_parser("switch_game", new switch_game_parser());

    _add_game_parser("up_star", new up_star_parser());

    _add_game_parser("nogo", new basic_parser<nogo>());
}
