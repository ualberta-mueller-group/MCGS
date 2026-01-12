/*
   TODO make a better name for this?
*/
#pragma once

#include "file_parser_ast.h"
#include "game.h"
#include "simple_text_hash.h"
#include "test_case.h"

class visitor_generate: public i_fp_visitor
{
public:
    visitor_generate();
    ~visitor_generate();

    std::vector<game*> get_games(const fp_chunk& chunk);
    i_test_case* get_test_case(const fp_chunk& chunk, int test_case_idx);

    void visit(const fp_expr_title& expr) override;
    void visit(const fp_expr_game& expr) override;
    void visit(const fp_expr_comment& expr) override;

    void visit(const fp_expr_command_solve_bw& expr) override;
    void visit(const fp_expr_command_solve_n& expr) override;
    void visit(const fp_expr_command_winning_moves& expr) override;

protected:
    struct visitor_context
    {
        visitor_context(const fp_chunk& chunk, std::vector<game*>& games)
            : chunk(chunk), games(games)
        {
            const std::optional<fp_expr_title>& implicit_title =
                chunk.get_implicit_title();

            if (implicit_title.has_value())
                title.emplace(implicit_title.value());
        }

        const fp_chunk& chunk;
        std::optional<int> test_case_idx;

        std::vector<game*>& games;
        std::optional<fp_expr_title> title;
        std::vector<std::string> comments;
        simple_text_hash input_hash;

        std::optional<i_test_case*> result_test_case;
    };

    std::optional<visitor_context> _ctx;
};

