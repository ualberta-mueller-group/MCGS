#include "visitor_print.h"

#include <iostream>
#include <string>
#include <optional>
#include <cstddef>
#include <vector>

#include "cgt_basics.h"
#include "ThGraph.h"
#include "file_parser_ast.h"
#include "test_case_enums.h"
#include "thermograph_helpers.h"

using namespace std;

//////////////////////////////////////// visitor_print methods
visitor_print::visitor_print()
{
}

void visitor_print::visit_chunk(const fp_chunk& chunk)
{
    cout << "|CHUNK|" << endl;

    {
        cout << "|CHUNK CONTENT EXPRS|" << endl;

        const int n_content_exprs = chunk.n_content_exprs();
        for (int i = 0; i < n_content_exprs; i++)
            chunk.get_content_expr(i).accept(*this);
    }

    {
        cout << "|CHUNK COMMAND EXPRS|" << endl;
        const int n_command_exprs = chunk.n_command_exprs();

        for (int i = 0; i < n_command_exprs; i++)
            chunk.get_command_expr(i).accept(*this);
    }
}

void visitor_print::visit(const fp_expr_title& expr)
{
    cout << "|TITLE L" << expr.get_line_no() << "| ``";
    cout << expr.get_title() << "``" << endl;
}

void visitor_print::visit(const fp_expr_game& expr)
{
    cout << "|GAME L" << expr.get_line_no() << "| ";
    cout << "(Bracketed: " << expr.is_bracketed() << ") ``";
    cout << expr.get_game_token() << "``" << endl;
}

void visitor_print::visit(const fp_expr_comment& expr)
{
    cout << "|COMMENT L" << expr.get_line_no() << "| ";
    cout << "(Type: " << expr.get_comment_type() << " Number: ";
    if (expr.get_comment_type() != FP_EXPR_COMMENT_TYPE_NUMBERED)
        cout << "?";
    else
        cout << expr.get_number();

    cout << ") ``" << expr.get_comment() << "``" << endl;
}

void visitor_print::visit(const fp_expr_command_solve_bw& expr)
{
    cout << "|SOLVE_BW L" << expr.get_line_no();
    cout << "| (Player: " << color_to_player_char(expr.get_player());
    cout << " Expected: ";

    const minimax_outcome_enum expected_outcome = expr.get_expected_outcome();

    switch (expected_outcome)
    {
        case MINIMAX_OUTCOME_NONE:
        {
            cout << "?";
            break;
        }

        case MINIMAX_OUTCOME_WIN:
        {
            cout << "Win";
            break;
        }

        case MINIMAX_OUTCOME_LOSS:
        {
            cout << "Loss";
            break;
        }
    }
    cout << ")" << endl;
}

void visitor_print::visit(const fp_expr_command_solve_n& expr)
{
    cout << "|SOLVE_N L" << expr.get_line_no() << "| ";
    cout << "(Expected: ";

    const optional<int>& expected_nim_value = expr.get_expected_nim_value();

    if (expected_nim_value.has_value())
        cout << expected_nim_value.value();
    else
        cout << "?";

    cout << ")" << endl;
}

void visitor_print::visit(const fp_expr_command_winning_moves& expr)
{
    cout << "|WINNING_MOVES L" << expr.get_line_no() << "| ";
    cout << "(Player: " << player_name_bw_imp(expr.get_player()) << " ";
    cout << "Expected: ";

    const optional<vector<string>>& expected =
        expr.get_expected_winning_moves();

    if (!expected.has_value())
        cout << "?";
    else
    {
        const size_t n_moves = expected->size();

        if (n_moves == 0)
            cout << "NONE";

        for (size_t i = 0; i < n_moves; i++)
        {
            cout << "``";
            cout << (*expected)[i];
            cout << "``";

            if (i + 1 < n_moves)
                cout << " ";
        }
    }
    cout << ")" << endl;
}

void visitor_print::visit(const fp_expr_command_thermograph& expr)
{
    cout << "|THERMOGRAPH L" << expr.get_line_no() << "| ";
    cout << "Expected: ";

    const optional<ThGraph>& graph = expr.get_exp_graph();

    if (graph.has_value())
        print_thermograph(cout, *graph);
    else
        cout << "?";

    cout << endl;
}
