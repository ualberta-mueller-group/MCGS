/*
    Abstract class for test cases (i_test_case) and non-abstract
    implementations:
        test_case_solve_bw
        test_case_solve_n
        test_case_winning_moves

    See development notes (File Parser and Internals) for more information
*/
#pragma once

#include <vector>
#include <optional>
#include <string>

#include "game.h"
#include "file_parser_ast.h"
#include "csv_row.h"
#include "test_case_enums.h"

////////////////////////////////////////////////// interface i_test_case
class i_test_case
{
public:
    i_test_case(command_type_enum command_type, std::vector<game*> games);
    virtual ~i_test_case();

    command_type_enum get_command_type() const;
    bool did_run() const;

    void run(unsigned long long timeout);

    const std::vector<game*>& get_games() const;
    csv_row& get_csv_row();

protected:
    virtual void _run_impl(unsigned long long timeout) = 0;

    std::vector<game*> _games;
    csv_row _csv_row;

private:
    const command_type_enum _command_type;
    bool _did_run;
};

////////////////////////////////////////////////// class test_case_bw_solve
class test_case_solve_bw: public i_test_case
{
public:
    test_case_solve_bw(fp_expr_command_solve_bw expr, std::vector<game*> games);

protected:
    void _run_impl(unsigned long long timeout) override;

    const fp_expr_command_solve_bw _expr;
};

////////////////////////////////////////////////// class test_case_n_solve
class test_case_solve_n: public i_test_case
{
public:
    test_case_solve_n(fp_expr_command_solve_n expr, std::vector<game*> games);

protected:
    void _run_impl(unsigned long long timeout) override;

    const fp_expr_command_solve_n _expr;
};

////////////////////////////////////////////////// class test_case_winning_moves
class test_case_winning_moves: public i_test_case
{
public:
    test_case_winning_moves(fp_expr_command_winning_moves expr,
                            std::vector<game*> games);

    static std::optional<std::string> winning_moves_string(
        const std::optional<std::vector<std::string>>& winning_moves);

protected:
    void _run_impl(unsigned long long timeout) override;

    const fp_expr_command_winning_moves _expr;
};
