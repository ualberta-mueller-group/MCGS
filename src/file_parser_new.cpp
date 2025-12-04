#include "file_parser_new.h"



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

    const int n_commands = chunk.n_command_exprs();
    for (int i = 0; i < n_commands; i++)
        visitor.visit(chunk, i);
}

