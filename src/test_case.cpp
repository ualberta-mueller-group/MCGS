#include "test_case.h"
#include "cgt_basics.h"
#include "file_parser_new.h"
#include "get_winning_moves.h"
#include "impartial_sumgame.h"
#include "solver_stats.h"
#include "stopwatch.h"
#include "test_case_enums.h"
#include "utilities.h"
#include <string>

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

void i_test_case::run(unsigned long long timeout)
{
    assert(!_did_run);

    assert(_csv_row.has_visitor_fields() &&  //
           _csv_row.has_pre_test_fields() && //
           !_csv_row.has_post_test_fields()   //
    );

    stats::reset_stats();
    _run_impl(timeout);

    assert(_csv_row.has_visitor_fields() &&  //
           _csv_row.has_pre_test_fields() && //
           _csv_row.has_post_test_fields()   //
    );

    _did_run = true;
}

const std::vector<game*>& i_test_case::get_games() const
{
    return _games;
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
    optional<string> expected_result_string;
    switch (_expr.get_expected_outcome())
    {
        case MINIMAX_OUTCOME_NONE:
            break;
        case MINIMAX_OUTCOME_WIN:
        {
            expected_result_string = "Win";
            break;
        }
        case MINIMAX_OUTCOME_LOSS:
        {
            expected_result_string = "Loss";
            break;
        }
    }

    _csv_row.fill_pre_test_fields(_games, _expr.get_player(), expected_result_string);
}

void test_case_solve_bw::_run_impl(unsigned long long timeout)
{
    stopwatch sw;

    sumgame s(_expr.get_player());

    sw.start();
    s.add(_games);
    optional<solve_result> result = s.solve_with_timeout(timeout);
    s.pop(_games);
    sw.stop();

    optional<string> result_string;
    if (result.has_value())
        result_string = result->win ? "Win" : "Loss";

    _csv_row.fill_post_test_fields(result_string, sw.get_duration_ms());
}

////////////////////////////////////////////////// test_case_solve_n methods
test_case_solve_n::test_case_solve_n(fp_expr_command_solve_n expr,
                                     std::vector<game*> games)
    : i_test_case(COMMAND_TYPE_SOLVE_N, games),
      _expr(expr)
{
    optional<string> expected_result_string;

    const optional<int>& expected_nim_value = _expr.get_expected_nim_value();

    if (expected_nim_value.has_value())
        expected_result_string = std::to_string(expected_nim_value.value());

    _csv_row.fill_pre_test_fields(_games, EMPTY, expected_result_string);
}

void test_case_solve_n::_run_impl(unsigned long long timeout)
{
    stopwatch sw;

    sumgame sum(BLACK);

    for (game* g : _games)
        if (!g->is_impartial())
            throw std::logic_error("Sum contains partisan games");

    sw.start();
    sum.add(_games);
    const optional<int> nim_value =
        search_impartial_sumgame_with_timeout(sum, timeout);
    sum.pop(_games);
    sw.stop();

    optional<string> result_string;

    if (nim_value.has_value())
        result_string = std::to_string(nim_value.value());

    _csv_row.fill_post_test_fields(result_string, sw.get_duration_ms());
}

//////////////////////////////////////////////////
// test_case_winning_moves methods

namespace {

} // namespace

test_case_winning_moves::test_case_winning_moves(
    fp_expr_command_winning_moves expr, std::vector<game*> games)
    : i_test_case(COMMAND_TYPE_WINNING_MOVES, games),
      _expr(expr)
{
    const optional<string> expected_result_string =
        winning_moves_string(expr.get_expected_winning_moves());

    _csv_row.fill_pre_test_fields(_games, expr.get_player(),
                                  expected_result_string);
}

optional<string> test_case_winning_moves::winning_moves_string(
    const optional<vector<string>>& winning_moves)
{
    if (!winning_moves.has_value())
        return {};

    const vector<string>& winning_moves_vec = winning_moves.value();
    if (winning_moves_vec.empty())
        return "None";

    return string_join(winning_moves_vec);
}

void test_case_winning_moves::_run_impl(unsigned long long timeout)
{
    THROW_ASSERT(is_black_white(_expr.get_player()));

    stopwatch sw;

    sumgame sum(_expr.get_player());

    sw.start();
    sum.add(_games);
    vector<string> winning_moves =
        get_winning_moves_for_player(sum, _expr.get_player());
    sum.pop(_games);
    sw.stop();

    const optional<string> result_string = winning_moves_string(winning_moves);
    _csv_row.fill_post_test_fields(result_string, sw.get_duration_ms());
}

