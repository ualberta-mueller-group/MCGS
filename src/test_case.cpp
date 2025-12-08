#include "test_case.h"

using namespace std;

////////////////////////////////////////////////// i_test_case methods
i_test_case::i_test_case(test_case_type_enum test_case_type,
                         std::vector<game*> games)
    : _games(games),
      _test_case_type(test_case_type),
      _did_run(false)
{
}

i_test_case::~i_test_case()
{
    for (game* g : _games)
        delete g;
}

test_case_type_enum i_test_case::get_test_case_type() const
{
    return _test_case_type;
}

bool i_test_case::did_run() const
{
    return _did_run;
}

void i_test_case::run()
{
    assert(!_did_run);
    _run_impl();
    _did_run = true;
}

csv_row& i_test_case::get_csv_row()
{
    return _csv_row;
}

////////////////////////////////////////////////// test_case_bw_solve methods
test_case_bw_solve::test_case_bw_solve(fp_expr_command_solve_bw expr,
                                       std::vector<game*> games)
    : i_test_case(TEST_CASE_TYPE_BW_SOLVE, games),
    _expr(expr)
{
}

void test_case_bw_solve::_run_impl()
{
}

////////////////////////////////////////////////// test_case_n_solve methods

test_case_n_solve::test_case_n_solve(fp_expr_command_solve_n expr,
                                     std::vector<game*> games)
    : i_test_case(TEST_CASE_TYPE_N_SOLVE, games),
      _expr(expr)
{
}

void test_case_n_solve::_run_impl()
{
}
