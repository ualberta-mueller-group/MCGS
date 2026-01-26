#include "visitor_print.h"

#include <iostream>
#include <string>
#include <optional>
#include <cstddef>
#include <vector>

#include "cgt_basics.h"
#include "search_utils.h"
#include "file_parser_ast.h"
#include "test_case_enums.h"


//////////////////////////////////////// visitor_print methods
visitor_print::visitor_print()
{
}

void visitor_print::visit_chunk(const fp_chunk& chunk)
{
    std::cout << "|CHUNK|" << std::endl;

    {
        std::cout << "|CHUNK CONTENT EXPRS|" << std::endl;

        const int n_content_exprs = chunk.n_content_exprs();
        for (int i = 0; i < n_content_exprs; i++)
            chunk.get_content_expr(i).accept(*this);
    }

    {
        std::cout << "|CHUNK COMMAND EXPRS|" << std::endl;
        const int n_command_exprs = chunk.n_command_exprs();

        for (int i = 0; i < n_command_exprs; i++)
            chunk.get_command_expr(i).accept(*this);
    }
}

void visitor_print::visit(const fp_expr_title& expr)
{
    std::cout << "|TITLE L" << expr.get_line_no() << "| ``";
    std::cout << expr.get_title() << "``" << std::endl;
}

void visitor_print::visit(const fp_expr_game& expr)
{
    std::cout << "|GAME L" << expr.get_line_no() << "| ";
    std::cout << "(Bracketed: " << expr.is_bracketed() << ") ``";
    std::cout << expr.get_game_token() << "``" << std::endl;
}

void visitor_print::visit(const fp_expr_comment& expr)
{
    std::cout << "|COMMENT L" << expr.get_line_no() << "| ";
    std::cout << "(Type: " << expr.get_comment_type() << " Number: ";
    if (expr.get_comment_type() != FP_EXPR_COMMENT_TYPE_NUMBERED)
        std::cout << "?";
    else
        std::cout << expr.get_number();

    std::cout << ") ``" << expr.get_comment() << "``" << std::endl;
}

void visitor_print::visit(const fp_expr_command_solve_bw& expr)
{
    std::cout << "|SOLVE_BW L" << expr.get_line_no();
    std::cout << "| (Player: " << color_to_player_char(expr.get_player());
    std::cout << " Expected: ";

    const minimax_outcome_enum expected_outcome = expr.get_expected_outcome();

    switch (expected_outcome)
    {
        case MINIMAX_OUTCOME_NONE:
        {
            std::cout << "?";
            break;
        }

        case MINIMAX_OUTCOME_WIN:
        {
            std::cout << "Win";
            break;
        }

        case MINIMAX_OUTCOME_LOSS:
        {
            std::cout << "Loss";
            break;
        }
    }
    std::cout << ")" << std::endl;
}

void visitor_print::visit(const fp_expr_command_solve_n& expr)
{
    std::cout << "|SOLVE_N L" << expr.get_line_no() << "| ";
    std::cout << "(Expected: ";

    const std::optional<int>& expected_nim_value = expr.get_expected_nim_value();

    if (expected_nim_value.has_value())
        std::cout << expected_nim_value.value();
    else
        std::cout << "?";

    std::cout << ")" << std::endl;
}

void visitor_print::visit(const fp_expr_command_winning_moves& expr)
{
    std::cout << "|WINNING_MOVES L" << expr.get_line_no() << "| ";
    std::cout << "(Player: " << player_name_bw_imp(expr.get_player()) << " ";
    std::cout << "Expected: ";

    const std::optional<std::vector<std::string>>& expected =
        expr.get_expected_winning_moves();

    if (!expected.has_value())
        std::cout << "?";
    else
    {
        const size_t n_moves = expected->size();

        if (n_moves == 0)
            std::cout << "NONE";

        for (size_t i = 0; i < n_moves; i++)
        {
            std::cout << "``";
            std::cout << (*expected)[i];
            std::cout << "``";

            if (i + 1 < n_moves)
                std::cout << " ";
        }
    }
    std::cout << ")" << std::endl;
}

