/*
   TODO the top level functions have some awkwardness...

   It's hard to keep copy elision and have the visit functions be aware of
   arguments passed to the top level functions...
*/

#include "visitor_generate.h"

using namespace std;

class visitor_generate::reset_on_scope_end
{
public:
    reset_on_scope_end(visitor_generate& visitor) : _visitor(visitor) {}

    ~reset_on_scope_end() { _visitor._reset_vars(); }

private:
    visitor_generate& _visitor;
};

//////////////////////////////////////////////////
visitor_generate::visitor_generate()
    : _games_ptr(nullptr)
{
}

vector<game*> visitor_generate::get_games(const fp_chunk& chunk)
{
    assert(_all_vars_empty());
    reset_on_scope_end reset_after(*this);

    vector<game*> games;
    _games_ptr = &games;

    const int n_content_exprs = chunk.n_content_exprs();
    for (int i = 0; i < n_content_exprs; i++)
        chunk.get_content_expr(i).accept(*this);

    return games;
}

i_test_case* visitor_generate::get_test_case(const fp_chunk& chunk, int test_case_idx)
{
    assert(_all_vars_empty());
    reset_on_scope_end reset_after(*this);

    vector<game*> games;
    _games_ptr = &games;

    _test_case_idx = test_case_idx;

    const int n_content_exprs = chunk.n_content_exprs();
    for (int i = 0; i < n_content_exprs; i++)
        chunk.get_content_expr(i).accept(*this);

    chunk.get_command_expr(test_case_idx).accept(*this);

    assert(_result_test_case.has_value());
    return _result_test_case.value();
}

void visitor_generate::visit(const fp_expr_title& expr)
{
    _title.emplace(expr);
}

void visitor_generate::visit(const fp_expr_game& expr)
{

}

void visitor_generate::visit(const fp_expr_comment& expr)
{

}

void visitor_generate::visit(const fp_expr_command_solve_bw& expr)
{

}

void visitor_generate::visit(const fp_expr_command_solve_n& expr)
{

}

void visitor_generate::visit(const fp_expr_command_winning_moves& expr)
{

}

void visitor_generate::_reset_vars()
{
    _test_case_idx.reset();
    _comments.clear();
    _games_ptr = nullptr;
    _title.reset();
    _result_test_case.reset();
}

bool visitor_generate::_all_vars_empty() const
{
    return !_test_case_idx.has_value() &&  //
           _comments.empty() &&            //
           _games_ptr == nullptr &&        //
           !_title.has_value() &&          //
           !_result_test_case.has_value(); //
}
