/*
   TODO make a better name for this?
*/
#pragma once

#include "file_parser_new.h"
#include "game.h"
#include "test_case.h"

class visitor_generate: public i_fp_visitor
{
public:
    visitor_generate();

    std::vector<game*> get_games(const fp_chunk& chunk);
    i_test_case* get_test_case(const fp_chunk& chunk, int test_case_idx);

    void visit(const fp_expr_title& expr) override;
    void visit(const fp_expr_game& expr) override;
    void visit(const fp_expr_comment& expr) override;

    void visit(const fp_expr_command_solve_bw& expr) override;
    void visit(const fp_expr_command_solve_n& expr) override;
    void visit(const fp_expr_command_winning_moves& expr) override;

protected:
    void _reset_vars();
    bool _all_vars_empty() const;

    std::optional<int> _test_case_idx;

    std::vector<std::string> _comments;
    std::vector<game*>* _games_ptr;
    std::optional<fp_expr_title> _title;

    std::optional<i_test_case*> _result_test_case;

    class reset_on_scope_end;
};

