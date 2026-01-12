#include "file_parser2.h"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <cassert>
#include <sstream>
#include <string>
#include <istream>
#include <unordered_map>
#include <memory>
#include <vector>
#include <fstream>
#include <utility>
#include <ios>

#include "cgt_basics.h"
#include "all_game_headers.h"
#include "file_parser_ast.h"
#include "fission.h"
#include "game_case.h"
#include "game_token_parsers.h"
#include "get_winning_moves.h"
#include "parsing_utilities.h"
#include "string_to_int.h"
#include "toppling_dominoes.h"
#include "utilities.h"
#include "version_info.h"

#include "visitor_generate.h"
#include "visitor_print.h"

/*
    NOTE: here we should usually throw instead of using assert() for many
        file_parser2 functions; many errors will be from bad user input rather
   than programming bugs, and exceptions also make unit testing easier

    Version command is optional, BUT STILL CHECKED, when reading from string
*/

using namespace std;

using namespace file_parser2_impl;

//////////////////////////////////////////////////////////// static members
bool file_parser2::debug_printing = false;
bool file_parser2::silence_warnings = false;
bool file_parser2::override_assert_correct_version = false;

unordered_map<string, shared_ptr<game_token_parser>> file_parser2::_game_map;


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

////////////////////////////////////////////////// file_parser2

// Private constructor -- use static functions instead
file_parser2::file_parser2(istream* stream, bool delete_stream,
                         bool do_version_check)
    : _tokenizer(stream, delete_stream),
      _do_version_check(do_version_check),
      _section_title(""),
      _line_number(0),
      _token(""),
      _warned_wrong_version(false)
{
    if (_game_map.size() == 0)
    {
        _init_game_parsers();
    }
}

void file_parser2::_version_check(const string& version_string)
{
    const string& expected = FILE_PARSER_VERSION_STRING;

    if (version_string != expected)
    {
        if (!file_parser2::silence_warnings)
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

void file_parser2::_add_game_parser(const string& game_title,
                                   game_token_parser* gp)
{
    assert(gp != nullptr);

    auto it = _game_map.insert({game_title, shared_ptr<game_token_parser>(gp)});

    if (!it.second)
    {
        string why = "Tried to add game parser \"" + game_title +
                     "\" but it already exists";
        throw parser_exception(why, DUPLICATE_GAME_PARSER);
    }

    _add_game_parser_impartial(game_title, it.first->second);
}

void file_parser2::_add_game_parser_impartial(
    const string& game_title, shared_ptr<game_token_parser>& gp_shared)
{
    assert(gp_shared.get() != nullptr);

    string impartial_title = "impartial " + game_title;
    shared_ptr<game_token_parser> impartial_parser(
        new impartial_game_token_parser_wrapper(gp_shared));

    auto it = _game_map.insert({impartial_title, impartial_parser});
    if (!it.second)
    {
        string why = "Tried to add impartial game parser for \"" + game_title +
                     "\" but it already exists";
        throw parser_exception(why, DUPLICATE_GAME_PARSER);
    }
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
match_state file_parser2::_get_enclosed(const string& open, const string& close,
                                       bool allow_inner)
{
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
    while (_tokenizer.get_token(new_token))
    {
        assert(!match_state_conclusive(state));

        _token += new_token;

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
bool file_parser2::_match(const string& open, const string& close,
                         const string& match_name, bool allow_inner)
{
    assert(_token.size() > 0);

    string token_copy = _token;

    match_state state = _get_enclosed(open, close, allow_inner);
    assert(match_state_conclusive(state));

    if (state == MATCH_FULL)
    {
        if (file_parser2::debug_printing)
        {
            cout << "Got " << match_name << ": " << _token << endl;
        }
        strip_enclosing(_token, open, close);

        _tokenizer.consume();
        return true;
    }

    if (state == MATCH_NOT_FOUND)
    {
        _tokenizer.rewind();
        _token = token_copy;
        return false;
    }

    assert(state == MATCH_ILLEGAL);

    string why = _get_error_start() + "failed to match " + match_name;
    throw parser_exception(why, FAILED_MATCH);

    return false;
}

// parse the current token using a game_token_parser, add result to game_cases
//bool file_parser2::_parse_game()
//{
//    if (_section_title.size() == 0)
//    {
//        string why =
//            _get_error_start() + "game token found but section title missing";
//        throw parser_exception(why, MISSING_SECTION_TITLE);
//
//        return false;
//    }
//
//    auto it = _game_map.find(_section_title);
//
//    if (it == _game_map.end())
//    {
//        string why =
//            _get_error_start() +
//            "game token found, but game parser doesn't exist for section \"";
//        why += _section_title + "\"";
//        throw parser_exception(why, MISSING_SECTION_PARSER);
//
//        return false;
//    }
//
//    game_token_parser* gp = (it->second).get();
//
//    for (int i = 0; i < FILE_PARSER_MAX_CASES; i++)
//    {
//        game* g = nullptr;
//
//        try
//        {
//            g = gp->parse_game(_token);
//        }
//        catch (exception& e)
//        {
//            cerr << _get_error_start() << e.what();
//            throw e;
//        }
//
//        if (g == nullptr)
//        {
//
//            // This won't leak memory when caught because the game_cases
//            // will be cleaned up when the file_parser2 is destructed
//            string why = _get_error_start() + "game parser for section \"" +
//                         _section_title;
//            why += "\" failed to parse game token: \"" + _token + "\"";
//            throw parser_exception(why, FAILED_GAME_TOKEN_PARSE);
//
//            return false;
//        }
//        else
//        {
//            _cases[i].games.push_back(g);
//
//            // Update hash. Should include section title for every token
//            string hashable_chunk = _section_title + _token;
//            _cases[i].hash.update(hashable_chunk);
//        }
//    }
//
//    return true;
//}


//bool file_parser2::_parse_command()
//{
//    assert(_case_count == 0);
//
//    vector<run_command_t> command_list;
//    bool success = get_run_command_list(_token, command_list);
//
//    if (!success)
//    {
//        string why = "failed to parse run command: \"" + _token + "\"";
//        throw parser_exception(why, FAILED_CASE_COMMAND);
//    }
//
//    if (command_list.empty())
//    {
//        string why = "run command with no cases";
//        throw parser_exception(why, EMPTY_CASE_COMMAND);
//    }
//
//    if (command_list.size() > FILE_PARSER_MAX_CASES)
//    {
//        string why =
//            _get_error_start() + "run command has too many cases, maximum is: ";
//        why += to_string(FILE_PARSER_MAX_CASES);
//        throw parser_exception(why, CASE_LIMIT_EXCEEDED);
//    }
//
//    const size_t n_commands = command_list.size();
//    for (size_t i = 0; i < n_commands; i++)
//    {
//        run_command_t& rc = command_list[i];
//        game_case& gc = _cases[i];
//
//        gc.impartial = rc.player == EMPTY;
//        gc.to_play = rc.player;
//        gc.expected_value = rc.expected_value;
//
//        _case_count++;
//    }
//
//    return true;
//}

namespace {
i_fp_expr_command* get_fp_expr_run_command_solve_bw(const int line_number,
                                      const vector<string>& string_tokens,
                                      size_t& idx)
{
    const size_t N = string_tokens.size();

    if (!(idx < N))
        return nullptr;

    const string& player_token = string_tokens[idx];
    idx++;

    ebw player = EMPTY;

    if (player_token == "B")
        player = BLACK;
    if (player_token == "W")
        player = WHITE;

    if (player == EMPTY)
        return nullptr;

    minimax_outcome_enum expected_outcome = MINIMAX_OUTCOME_NONE;

    if (idx < N && !is_comma(string_tokens[idx]))
    {

        const string& outcome_token = string_tokens[idx];
        idx++;

        if (outcome_token == "win")
            expected_outcome = MINIMAX_OUTCOME_WIN;
        if (outcome_token == "loss")
            expected_outcome = MINIMAX_OUTCOME_LOSS;

        // TODO proper parser exception
        THROW_ASSERT(expected_outcome != MINIMAX_OUTCOME_NONE);
    }

    return new fp_expr_command_solve_bw(line_number, player, expected_outcome);
}

i_fp_expr_command* get_fp_expr_run_command_solve_n(const int line_number,
                                     const vector<string>& string_tokens,
                                     size_t& idx)
{
    const size_t N = string_tokens.size();

    if (!(idx < N))
        return nullptr;

    const string& player_token = string_tokens[idx];
    idx++;

    if (player_token != "N")
        return nullptr;

    std::optional<int> expected_nimber;

    if (idx < N && !is_comma(string_tokens[idx]))
    {
        const string& outcome_token = string_tokens[idx];
        idx++;

        expected_nimber = str_to_i_opt(outcome_token);

        // TODO proper parser exception
        THROW_ASSERT(expected_nimber.has_value() && expected_nimber.value() >= 0);
    }

    return new fp_expr_command_solve_n(line_number, expected_nimber);
}

i_fp_expr_command* get_fp_expr_run_command_winning_moves(const int line_number,
                                     const vector<string>& string_tokens,
                                     size_t& idx)
{
    const size_t N = string_tokens.size();

    if (!(idx + 2 < N))
        return nullptr;

    const string& winning_token = string_tokens[idx];
    const string& moves_token = string_tokens[idx + 1];
    const string& player_token = string_tokens[idx + 2];
    idx += 3;

    if (winning_token != "winning" || moves_token != "moves")
        return nullptr;

    optional<ebw> player;

    if (player_token == "B")
        player = BLACK;
    if (player_token == "W")
        player = WHITE;
    if (player_token == "N")
        player = EMPTY;

    if (!player.has_value())
        return nullptr;

    optional<vector<string>> expected_moves = vector<string>();
    assert(expected_moves.has_value());

    for (; idx < N; idx++)
    {
        const string& move_string = string_tokens[idx];

        if (is_comma(move_string))
            break;

        expected_moves->emplace_back(move_string);
    }

    const size_t n_move_strings = expected_moves.value().size();

    // Expected result unspecified
    if (n_move_strings == 0)
        expected_moves.reset();

    if (n_move_strings == 1)
    {
        assert(expected_moves.has_value());

        // Careful! This reference is invalidated while still in scope
        const string& move_string = expected_moves->back();

        if (move_string == "None")
        {
            expected_moves->pop_back();
            assert(expected_moves->empty());
        }
    }

    // Moves are sorted further down the line
    return new fp_expr_command_winning_moves(line_number, player.value(),
                                             expected_moves);
}

#ifdef CALL_PARSE_FN_MACRO
#error Macro already defined
#endif

#define CALL_PARSE_FN_MACRO(fn) \
    expr = fn(line_number, string_tokens, idx); \
    if (expr != nullptr) \
    { \
        chunk.add_command_expr(expr); \
        return true; \
    } \
    idx = idx_start; \
    static_assert(true)

bool get_fp_expr_run_command(const int line_number,
                             const vector<string>& string_tokens, size_t& idx,
                             fp_chunk& chunk)
{
    const size_t idx_start = idx;

    i_fp_expr_command* expr = nullptr;

    CALL_PARSE_FN_MACRO(get_fp_expr_run_command_solve_bw);
    CALL_PARSE_FN_MACRO(get_fp_expr_run_command_solve_n);
    CALL_PARSE_FN_MACRO(get_fp_expr_run_command_winning_moves);

    return false;
}
#undef CALL_PARSE_FN_MACRO


} // namespace

// actually parses block...
bool file_parser2::_parse_command()
{
    vector<string> string_tokens = get_string_tokens(_token, {','});
    const size_t N = string_tokens.size();

    if (N == 0)
        return true;

    size_t i = 0;
    while (i < N)
    {
        if (!get_fp_expr_run_command(_line_number, string_tokens, i,
                                     _chunk.value()))
            return false;

        if (i < N)
        {
            bool got_comma = consume_mandatory_comma(string_tokens, i);
            if (!got_comma || (got_comma && i >= N))
                return false;
        }
    }

    return true;
}

// get start of parser error text (including line number)
string file_parser2::_get_error_start()
{
    return _get_error_start(_line_number);
}

string file_parser2::_get_error_start(int line_number)
{
    return "Parser error on line " + to_string(line_number) + ": ";
}

file_parser2::~file_parser2()
{
}

bool file_parser2::parse_chunk()
{
    if (!_chunk.has_value())
        _chunk.emplace();
    assert(_chunk.has_value());

    _chunk->clear_content_exprs();
    _chunk->clear_command_exprs();

    while (_tokenizer.get_token(_token))
    {
        _line_number = _tokenizer.line_start();
        _tokenizer.consume();

        if (_tokenizer.is_whitespace())
            continue;

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

            bool ok = _parse_command();
            THROW_ASSERT(ok);

            // the only command is a "run" command, so just return a case.
            // OK if no games were read yet -- this is a "0" game

            return true;
        }

        // Match title
        if (_match("[", "]", "section title", false))
        {
            //_section_title = _token;
            _chunk->add_content_expr(new fp_expr_title(_line_number, _token));
            continue;
        }

        // Match brackets
        if (_match("(", ")", "bracket token", false))
        {
            //_parse_game();
            _chunk->add_content_expr(new fp_expr_game(_line_number, _token, true));
            continue;
        }

        // Match comment
        if (_match("/*", "*/", "comment", true))
        {
            _chunk->add_content_expr(new fp_expr_comment(_line_number, _token));
            continue;
        }

        // Must be game token
        if (file_parser2::debug_printing)
        {
            cout << "Got simple token: " << _token << endl;
        }
        //_parse_game();
        _chunk->add_content_expr(new fp_expr_game(_line_number, _token, false));
    }

    // no more cases
    return false;
}

std::vector<game*> file_parser2::get_games() const
{
    assert(_chunk.has_value());
    visitor_generate visitor;

    return visitor.get_games(_chunk.value());
}

int file_parser2::n_test_cases() const
{
    assert(_chunk.has_value());
    return _chunk->n_command_exprs();
}

command_type_enum file_parser2::get_test_case_type(int test_case_idx) const
{
    assert(_chunk.has_value() && test_case_idx < n_test_cases());
    return _chunk.value().get_command_expr(test_case_idx).get_command_type();
}

std::shared_ptr<i_test_case> file_parser2::get_test_case(int test_case_idx) const
{
    visitor_generate visitor;
    i_test_case* test_case = visitor.get_test_case(_chunk.value(), test_case_idx);

    return shared_ptr<i_test_case>(test_case);
}

void file_parser2::print_ast() const
{
    fp_visitor_print visitor;
    visitor.visit_chunk(_chunk.value());
}

game* file_parser2::construct_game(const std::string& title, int line_number,
                                   const std::string& game_token)
{
    if (title.size() == 0)
    {
        string why = _get_error_start(line_number) +
                     "game token found but section title missing";
        throw parser_exception(why, MISSING_SECTION_TITLE);
    }

    auto it = _game_map.find(title);

    if (it == _game_map.end())
    {
        string why =
            _get_error_start(line_number) +
            "game token found, but game parser doesn't exist for section \"";
        why += title + "\"";
        throw parser_exception(why, MISSING_SECTION_PARSER);
    }

    game_token_parser* gp = (it->second).get();

    game* g = nullptr;

    try
    {
        g = gp->parse_game(game_token);
    }
    catch (exception& e)
    {
        cerr << _get_error_start(line_number) << e.what();
        throw e;
    }

    if (g == nullptr)
    {
        // This won't leak memory when caught because the game_cases
        // will be cleaned up when the file_parser2 is destructed
        string why = _get_error_start(line_number) +
                     "game parser for section \"" + title;
        why += "\" failed to parse game token: \"" + game_token + "\"";
        throw parser_exception(why, FAILED_GAME_TOKEN_PARSE);
    }
    else
    {
        return g;
    }
}

file_parser2* file_parser2::from_stdin()
{
    return new file_parser2(&cin, false, false);
}

file_parser2* file_parser2::from_file(const string& file_name)
{
    ifstream* stream = new ifstream(file_name);

    if (!stream->is_open())
    {
        delete stream;
        throw ios_base::failure("file_parser2 failed to open file \"" +
                                file_name + "\"");
    }

    return new file_parser2(stream, true, true);
}

file_parser2* file_parser2::from_string(const string& str)
{
    return new file_parser2(new stringstream(str), true, false);
}

bool file_parser2::warned_wrong_version()
{
    return _warned_wrong_version;
}

/*
    When implementing a new game, you must register its game parser here

    i.e:
        add_game_parser("clobber_1xn", new basic_parser<clobber_1xn>());

    Will cause games in the section denoted by "[clobber_1xn]" to be
        created as the clobber_1xn class.

    Additionally, this will automatically create the section
        "[impartial clobber_1xn]", whose games are the result of wrapping the
        game produced by the parser passed to add_game_parser, with an
        impartial_game_wrapper.
*/
void file_parser2::_init_game_parsers()
{
    // TODO this file shouldn't include every game header. Define this function
    // in another cpp file...
    assert(_game_map.size() == 0);

    _add_game_parser("clobber_1xn", new basic_parser<clobber_1xn>());
    _add_game_parser("nogo_1xn", new basic_parser_with_check<nogo_1xn>());
    _add_game_parser("elephants", new basic_parser<elephants>());

    _add_game_parser("integer_game", new int_parser<integer_game>());
    _add_game_parser("nimber", new int_parser<nimber>());

    _add_game_parser("dyadic_rational", new dyadic_rational_parser());
    _add_game_parser("switch_game", new switch_game_parser());

    _add_game_parser("up_star", new up_star_parser());

    _add_game_parser("nogo", new basic_parser_with_check<nogo>());
    _add_game_parser("clobber", new basic_parser<clobber>());
    _add_game_parser("domineering", new basic_parser<domineering>());
    _add_game_parser("amazons", new basic_parser<amazons>());
    _add_game_parser("fission", new basic_parser<fission>());
    _add_game_parser("toppling_dominoes", new basic_parser<toppling_dominoes>());
    _add_game_parser("gen_toads", new basic_parameterized_game_parser<gen_toads>);
    _add_game_parser("sheep", new basic_parser<sheep>());

    _add_game_parser("kayles", new int_parser<kayles>());
}
