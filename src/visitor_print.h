#pragma once

#include "file_parser_new.h"

//////////////////////////////////////// class fp_visitor_print
class fp_visitor_print: i_fp_visitor
{
public:
    fp_visitor_print();

    void visit_chunk(const fp_chunk& chunk);

    void visit(const fp_expr_title& expr) override;
    void visit(const fp_expr_game& expr) override;
    void visit(const fp_expr_comment& expr) override;
    void visit(const fp_expr_command_solve_bw& expr) override;
    void visit(const fp_expr_command_solve_n& expr) override;
    void visit(const fp_expr_command_winning_moves& expr) override;

private:
};

