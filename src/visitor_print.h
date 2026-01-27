/*
    Debugging utility. Visitor for printing contents of an fp_chunk
*/
#pragma once

#include "file_parser_ast.h"

//////////////////////////////////////// class visitor_print
class visitor_print: i_fp_visitor
{
public:
    visitor_print();

    void visit_chunk(const fp_chunk& chunk);

    void visit(const fp_expr_title& expr) override;
    void visit(const fp_expr_game& expr) override;
    void visit(const fp_expr_comment& expr) override;
    void visit(const fp_expr_command_solve_bw& expr) override;
    void visit(const fp_expr_command_solve_n& expr) override;
    void visit(const fp_expr_command_winning_moves& expr) override;

private:
};

