/*
   TODO the top level functions have some awkwardness...

   It's hard to keep copy elision and have the visit functions be aware of
   arguments passed to the top level functions...
*/

#include "visitor_generate.h"
#include "cgt_basics.h"
#include "file_parser.h"
#include "file_parser_ast.h"
#include "search_utils.h"
#include "test_case.h"
#include "test_case_enums.h"
#include "utilities.h"
#include <cmath>
#include <string>

using namespace std;


//////////////////////////////////////////////////
visitor_generate::visitor_generate()
{
}

visitor_generate::~visitor_generate()
{
    assert(!_ctx.has_value());
}

vector<game*> visitor_generate::get_games(const fp_chunk& chunk)
{
    vector<game*> games;
    _ctx.emplace(chunk, games);

    const int n_content_exprs = chunk.n_content_exprs();
    for (int i = 0; i < n_content_exprs; i++)
        chunk.get_content_expr(i).accept(*this);

    _ctx.reset();
    return games;
}

i_test_case* visitor_generate::get_test_case(const fp_chunk& chunk, int test_case_idx)
{
    vector<game*> games;
    _ctx.emplace(chunk, games);
    _ctx->test_case_idx = test_case_idx;

    const int n_content_exprs = chunk.n_content_exprs();
    for (int i = 0; i < n_content_exprs; i++)
        chunk.get_content_expr(i).accept(*this);

    chunk.get_command_expr(test_case_idx).accept(*this);

    assert(_ctx->result_test_case.has_value());
    i_test_case* result_test_case = _ctx->result_test_case.value();

    csv_row& row = result_test_case->get_csv_row();
    row.fill_visitor_fields(_ctx->comments,
                            result_test_case->get_command_type(),
                            _ctx->input_hash.get_string());

    _ctx.reset();
    return result_test_case;
}

void visitor_generate::visit(const fp_expr_title& expr)
{
    assert(_ctx.has_value());
    _ctx->title.emplace(expr);
}

void visitor_generate::visit(const fp_expr_game& expr)
{
    assert(_ctx.has_value());

    // TODO proper parser exception
    THROW_ASSERT(_ctx->title.has_value());

    const string& title_token = _ctx->title->get_title();
    const string& game_token = expr.get_game_token();

    _ctx->input_hash.update(title_token + game_token);

    game* g = file_parser::construct_game(title_token, expr.get_line_no(),
                                           game_token);

    _ctx->games.emplace_back(g);
}

void visitor_generate::visit(const fp_expr_comment& expr)
{
    assert(_ctx.has_value());

    if (!_ctx->test_case_idx.has_value())
        return;

    const int test_case_idx = _ctx->test_case_idx.value();

    switch (expr.get_comment_type())
    {
        case FP_EXPR_COMMENT_TYPE_NUMBERED:
        {
            if (expr.get_number() == test_case_idx)
                _ctx->comments.emplace_back(expr.get_comment());
            break;
        }

        case FP_EXPR_COMMENT_TYPE_SHARED:
        {
            _ctx->comments.emplace_back(expr.get_comment());
            break;
        }

        case FP_EXPR_COMMENT_TYPE_SILENT:
            return;
    }
}

void visitor_generate::visit(const fp_expr_command_solve_bw& expr)
{
    assert(_ctx.has_value() && !_ctx->result_test_case.has_value());

    {
        simple_text_hash& input_hash = _ctx->input_hash;
        input_hash.update(command_type_to_string(expr.get_command_type()));

        input_hash.update(string(1, color_to_player_char(expr.get_player())));
        input_hash.update(minimax_outcome_to_string(expr.get_expected_outcome()));
    }

    _ctx->result_test_case =
        new test_case_solve_bw(expr, std::move(_ctx->games));
}

void visitor_generate::visit(const fp_expr_command_solve_n& expr)
{
    assert(_ctx.has_value() && !_ctx->result_test_case.has_value());

    {
        simple_text_hash& input_hash = _ctx->input_hash;
        input_hash.update(command_type_to_string(expr.get_command_type()));

        const optional<int>& exp_value_opt = expr.get_expected_nim_value();
        const int exp_value =
            exp_value_opt.has_value() ? exp_value_opt.value() : -1;
        input_hash.update(to_string(exp_value));
    }

    _ctx->result_test_case =
        new test_case_solve_n(expr, std::move(_ctx->games));
}

void visitor_generate::visit(const fp_expr_command_winning_moves& expr)
{
    assert(_ctx.has_value() && !_ctx->result_test_case.has_value());

    {
        simple_text_hash& input_hash = _ctx->input_hash;
        input_hash.update(command_type_to_string(expr.get_command_type()));
        input_hash.update(player_name_bw_imp(expr.get_player()));

        const optional<vector<string>>& exp_moves_opt =
            expr.get_expected_winning_moves();

        if (exp_moves_opt.has_value())
        {
            const vector<string>& exp_moves = exp_moves_opt.value();
            input_hash.update(exp_moves.empty() ? "NO MOVES"
                                                : string_join(exp_moves, " "));
        }
        else
            input_hash.update("???");
    }

    _ctx->result_test_case =
        new test_case_winning_moves(expr, std::move(_ctx->games));
}

