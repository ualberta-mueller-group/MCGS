#include "file_parser_new.h"

#include <cctype>
#include <iostream>
#include "cgt_basics.h"
#include "search_utils.h"
#include "test_case_enums.h"
#include "throw_assert.h"
#include "utilities.h"
#include "string_to_int.h"
#include "visitor_print.h"

//////////////////////////////////////////////////

//////////////////////////////////////// i_fp_expr methods
i_fp_expr::i_fp_expr(int line_no) : _line_no(line_no)
{
}

int i_fp_expr::get_line_no() const
{
    return _line_no;
}

//////////////////////////////////////// i_fp_expr_content methods
i_fp_expr_content::i_fp_expr_content(int line_no)
    : i_fp_expr(line_no)
{
}

//////////////////////////////////////// i_fp_expr_command methods
i_fp_expr_command::i_fp_expr_command(int line_no, command_type_enum command_type)
    : i_fp_expr(line_no),
      _command_type()
{
}

command_type_enum i_fp_expr_command::get_command_type() const
{
    return _command_type;
}

//////////////////////////////////////// fp_expr_title methods
fp_expr_title::fp_expr_title(int line_no, const std::string& title)
    : i_fp_expr_content(line_no),
      _title(title)
{
}

void fp_expr_title::accept(i_fp_visitor& visitor) const
{
    visitor.visit(*this);
}

const std::string& fp_expr_title::get_title() const
{
    return _title;
}

//////////////////////////////////////// fp_expr_game methods
fp_expr_game::fp_expr_game(int line_no, const std::string& game_token,
                           bool is_bracketed)
    : i_fp_expr_content(line_no),
      _game_token(game_token),
      _is_bracketed(is_bracketed)
{
    THROW_ASSERT(LOGICAL_IMPLIES(string_contains_whitespace(_game_token),
                                 _is_bracketed));
}

void fp_expr_game::accept(i_fp_visitor& visitor) const
{
    visitor.visit(*this);
}

const std::string& fp_expr_game::get_game_token() const
{
    return _game_token;
}

bool fp_expr_game::is_bracketed() const
{
    return _is_bracketed;
}


//////////////////////////////////////// fp_expr_comment methods
fp_expr_comment::fp_expr_comment(int line_no,
                                        const std::string& comment_string)
    : i_fp_expr_content(line_no),
      _comment(comment_string)
{
    // Default is shared
    _comment_type = FP_EXPR_COMMENT_TYPE_SHARED;
    _number = -1;

    // Silent comment?
    if (checked_is_element(_comment, 0, '_'))
    {
        // TODO proper parser error
        THROW_ASSERT(
            LOGICAL_IMPLIES(_comment.size() > 1, std::isspace(_comment[1])));

        size_t n_skip_chars = 1;
        if (_comment.size() > 1)
            n_skip_chars++;

        _comment_type = FP_EXPR_COMMENT_TYPE_SILENT;
        _comment = _comment.substr(n_skip_chars);
        return;
    }

    // Numbered comment?
    if (!checked_is_element(_comment, 0, '#'))
        return;

    std::string comment_number_string;
    size_t n_skip_chars = 1;

    // Get digits
    for (size_t i = 1; i < _comment.size(); i++)
    {
        const char c = _comment[i];
        n_skip_chars++;

        if (std::isspace(c))
            break;

        comment_number_string.push_back(c);
    }

    std::optional<int> comment_number_int = str_to_i_opt(comment_number_string);

    if (!comment_number_int.has_value() || comment_number_int.value() < 0)
    {
        THROW_ASSERT(false); // TODO make this throw a parser exception

        //throw parser_exception("Comment with '#' missing or bad number",
        //                       BAD_COMMENT_FORMAT);
    }

    _comment = _comment.substr(n_skip_chars);
    _comment_type = FP_EXPR_COMMENT_TYPE_NUMBERED;
    _number = comment_number_int.value();
}

void fp_expr_comment::accept(i_fp_visitor& visitor) const
{
    visitor.visit(*this);
}

const std::string& fp_expr_comment::get_comment() const
{
    return _comment;
}

fp_expr_comment_type fp_expr_comment::get_comment_type() const
{
    return _comment_type;
}

int fp_expr_comment::get_number() const
{
    THROW_ASSERT(_comment_type == FP_EXPR_COMMENT_TYPE_NUMBERED);
    return _number;
}

//////////////////////////////////////// fp_expr_command_solve_bw methods
fp_expr_command_solve_bw::fp_expr_command_solve_bw(int line_no, bw player,
                         minimax_outcome_enum expected_outcome)
    : i_fp_expr_command(line_no, COMMAND_TYPE_SOLVE_BW),
      _player(player),
      _expected_outcome(expected_outcome)
{
}

void fp_expr_command_solve_bw::accept(i_fp_visitor& visitor) const
{
    visitor.visit(*this);
}

bw fp_expr_command_solve_bw::get_player() const
{
    return _player;
}

minimax_outcome_enum fp_expr_command_solve_bw::get_expected_outcome() const
{
    return _expected_outcome;
}


//////////////////////////////////////// fp_expr_command_solve_n methods
fp_expr_command_solve_n::fp_expr_command_solve_n(
    int line_no, const std::optional<int>& expected_nim_value)
    : i_fp_expr_command(line_no, COMMAND_TYPE_SOLVE_N),
      _expected_nim_value(expected_nim_value)
{
}

void fp_expr_command_solve_n::accept(i_fp_visitor& visitor) const
{
    visitor.visit(*this);
}

const std::optional<int>& fp_expr_command_solve_n::
    get_expected_nim_value() const
{
    return _expected_nim_value;
}

//////////////////////////////////////////////////
// fp_expr_command_winning_moves methods

fp_expr_command_winning_moves::fp_expr_command_winning_moves(
    int line_no, ebw player,
    std::optional<std::vector<std::string>> expected_winning_moves)
    : i_fp_expr_command(line_no, COMMAND_TYPE_WINNING_MOVES),
      _player(player),
      _expected_winning_moves(expected_winning_moves)
{
}

void fp_expr_command_winning_moves::accept(i_fp_visitor& visitor) const
{
    visitor.visit(*this);
}

ebw fp_expr_command_winning_moves::get_player() const
{
    return _player;
}

const std::optional<std::vector<std::string>>& fp_expr_command_winning_moves::
    get_expected_winning_moves() const
{
    return _expected_winning_moves;
}



//////////////////////////////////////// fp_chunk methods
fp_chunk::fp_chunk()
{
}

int fp_chunk::n_content_exprs() const
{
    const size_t n_elements = _content_exprs.size();
    THROW_ASSERT(0 <= n_elements && n_elements <= std::numeric_limits<int>::max());
    return static_cast<int>(n_elements);
}

void fp_chunk::add_content_expr(i_fp_expr_content* expr)
{
    _content_exprs.emplace_back(expr);
}

const i_fp_expr_content& fp_chunk::get_content_expr(int idx) const
{
    assert(idx < n_content_exprs());
    return *(_content_exprs[idx]);
}

void fp_chunk::clear_content_exprs()
{
    _content_exprs.clear();
}

int fp_chunk::n_command_exprs() const
{
    const size_t n_elements = _command_exprs.size();
    THROW_ASSERT(0 <= n_elements && n_elements <= std::numeric_limits<int>::max());
    return static_cast<int>(n_elements);
}

void fp_chunk::add_command_expr(i_fp_expr_command* expr)
{
    _command_exprs.emplace_back(expr);
}

const i_fp_expr_command& fp_chunk::get_command_expr(int idx) const
{
    assert(idx < n_command_exprs());
    return *(_command_exprs[idx]);
}

void fp_chunk::clear_command_exprs()
{
    _command_exprs.clear();
}

void fp_chunk::set_version_string(const std::string& version_string)
{
    _version_string = version_string;
}

void fp_chunk::clear_version_string()
{
    _version_string.reset();
}

bool fp_chunk::has_version_string() const
{
    return _version_string.has_value();
}

const std::optional<std::string>& fp_chunk::get_version_string() const
{
    return _version_string;
}


//////////////////////////////////////////////////
void test_file_parser_new_stuff()
{
    fp_chunk chunk;


    chunk.add_content_expr(new fp_expr_title(1, "clobber_1xn"));
    chunk.add_content_expr(new fp_expr_game(2, "XO", false));
    chunk.add_content_expr(new fp_expr_game(3, "XO XOXX", true));
    chunk.add_content_expr(new fp_expr_comment(4, "aodisjfdbfiofj"));

    chunk.add_command_expr(
        new fp_expr_command_solve_bw(5, BLACK, MINIMAX_OUTCOME_WIN));
    chunk.add_command_expr(
        new fp_expr_command_solve_bw(5, WHITE, MINIMAX_OUTCOME_NONE));

    fp_visitor_print visitor;
    visitor.visit_chunk(chunk);

}

