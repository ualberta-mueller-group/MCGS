#include "file_parser_new.h"



//////////////////////////////////////////////////
void test_file_parser_new_stuff()
{
    fp_chunk chunk;

    chunk.add_expr(new fp_expr_title(0, "clobber"));
    chunk.add_expr(new fp_expr_game(1, "XOXO", false));
    chunk.add_expr(new fp_expr_game(2, "XOXOXO XOXOX", true));

    fp_expr_curly_block* block = new fp_expr_curly_block(3);
    block->add_command(new fp_expr_command_solve_bw(3, BLACK, MINIMAX_OUTCOME_NONE));
    block->add_command(new fp_expr_command_solve_bw(4, WHITE, MINIMAX_OUTCOME_WIN));
    chunk.add_expr(block);

    fp_visitor_print visitor;
    visitor.visit(chunk);

}

