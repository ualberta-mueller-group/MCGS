#include "test_case.h"
#include "test_case_enums.h"

using namespace std;

////////////////////////////////////////////////// i_test_case methods
i_test_case::i_test_case(command_type_enum command_type,
                         std::vector<game*> games)
    : _games(games),
      _command_type(command_type),
      _did_run(false)
{
}

i_test_case::~i_test_case()
{
    for (game* g : _games)
        delete g;
}

command_type_enum i_test_case::get_command_type() const
{
    return _command_type;
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

////////////////////////////////////////////////// test_case_solve_bw methods
test_case_solve_bw::test_case_solve_bw(fp_expr_command_solve_bw expr,
                                       std::vector<game*> games)
    : i_test_case(COMMAND_TYPE_SOLVE_BW, games),
    _expr(expr)
{
}

void test_case_solve_bw::_run_impl()
{
}

////////////////////////////////////////////////// test_case_solve_n methods
test_case_solve_n::test_case_solve_n(fp_expr_command_solve_n expr,
                                     std::vector<game*> games)
    : i_test_case(COMMAND_TYPE_SOLVE_N, games),
      _expr(expr)
{
}

void test_case_solve_n::_run_impl()
{
}
