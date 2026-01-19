#include "get_winning_moves.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <vector>
#include <memory>
#include <iostream>
#include <cassert>
#include <string>
#include <set>
#include <cstdint>

#include "cgt_basics.h"
#include "game.h"
#include "impartial_sumgame.h"
#include "solver_stats.h"
#include "sumgame.h"
#include "timeout_token.h"
#include "throw_assert.h"
#include "utilities.h"
#include "print_moves.h"

using namespace std;

namespace {
optional<bool> is_winning_move(sumgame& sum, const sumgame_move& sm, bw player,
                               bool use_impartial_search,
                               const timeout_token& timeout_tok, uint64_t depth)
{
    assert_restore_sumgame ars(sum);
    assert(is_black_white(player));

    const uint64_t next_depth = depth + 1;
    optional<bool> is_winning;

    assert(sum.to_play() == player);
    sum.play_sum(sm, player);

    if (!use_impartial_search)
    {
        const optional<solve_result> result =
            sum.solve_with_timeout_token(timeout_tok, next_depth);

        if (result.has_value())
            is_winning = !result->win;
    }
    else
    {
        const optional<int> nim_value_opt =
            search_impartial_sumgame_with_timeout_token(sum, timeout_tok, next_depth);

        if (nim_value_opt.has_value())
        {
            const int nim_value = nim_value_opt.value();
            assert(nim_value >= 0);

            is_winning = (nim_value == 0);
        }
    }

    sum.undo_move();

    return is_winning;
}


optional<vector<string>> get_winning_moves_impl(
    sumgame& sum, ebw player, const timeout_token& timeout_tok, uint64_t depth)
{
    THROW_ASSERT(                       //
        LOGICAL_IMPLIES(                //
            player == EMPTY,            //
            sum.all_impartial()         //
            ),                          //
        "Sum contains partisan games"); //

    assert(is_empty_black_white(player));

    stats::report_search_node(sum, player, depth);
    // No "next_depth = depth + 1" -- no move is played here

    optional<vector<string>> winning_moves = vector<string>();

    const bool use_impartial = (player == EMPTY);
    const bw use_player = use_impartial ? BLACK : player;
    const bool with_subgame_idx = sum.num_active_games() > 1;

    assert_restore_sumgame ars1(sum);
    const bw restore_player = sum.to_play();

    sum.set_to_play(use_player);

    bool over_time = false;

    unique_ptr<sumgame_move_generator> gen(
        sum.create_sum_move_generator(use_player));

    while (*gen)
    {
        over_time = timeout_tok.stop_requested();

        if (over_time)
            break;

        sumgame_move sm = gen->gen_sum_move();
        optional<bool> is_winning_opt =
            is_winning_move(sum, sm, use_player, use_impartial, timeout_tok, depth);

        if (!is_winning_opt.has_value())
        {
            over_time = true;
            assert(timeout_tok.stop_requested());
            break;
        }

        const bool is_winning = is_winning_opt.value();

        if (is_winning)
        {
            assert_restore_sumgame ars3(sum);
            const string winning_move =
                sumgame_move_to_string(sum, sm, use_player, with_subgame_idx);

            winning_moves.value().emplace_back(winning_move);
        }

        ++(*gen);
    }

    sum.set_to_play(restore_player);

    if (over_time)
        return {};

    return winning_moves;
}

} // namespace

//////////////////////////////////////////////////
vector<string> get_winning_moves(sumgame& sum, ebw player)
{
    optional<vector<string>> result =
        get_winning_moves_with_timeout(sum, player, 0);

    assert(result.has_value());
    return result.value();
}

optional<vector<string>> get_winning_moves_with_timeout(
    sumgame& sum, ebw player, unsigned long long timeout_ms)
{
    timeout_source src;
    timeout_token tok = src.get_timeout_token();

    src.start_timeout(timeout_ms);
    optional<vector<string>> result =
        get_winning_moves_with_timeout_token(sum, player, tok, INITIAL_SEARCH_DEPTH);
    src.cancel_timeout();

    return result;
}

optional<vector<string>> get_winning_moves_with_timeout_token(
    sumgame& sum, ebw player, const timeout_token& timeout_tok, uint64_t depth)
{
    return get_winning_moves_impl(sum, player, timeout_tok, depth);
}

////////////////////////////////////////////////// winning_moves_diff_t methods
winning_moves_diff_t::winning_moves_diff_t(const vector<string>& computed_moves,
                                           const vector<string>& expected_moves)

{
    const set<string> computed_set(computed_moves.begin(), computed_moves.end());
    const set<string> expected_set(expected_moves.begin(), expected_moves.end());

    // extra = computed - expected
    set_difference(computed_set.begin(), computed_set.end(), //
                   expected_set.begin(), expected_set.end(), //
                   back_inserter(_extra_moves)               //
    );

    // missing = expected - computed
    set_difference(expected_set.begin(), expected_set.end(), //
                   computed_set.begin(), computed_set.end(), //
                   back_inserter(_missing_moves)             //
    );

    sort_winning_moves(_extra_moves);
    sort_winning_moves(_missing_moves);
}

string winning_moves_diff_t::get_diff_string(bool prepend_diff_symbols) const
{
    string diff_string;

    if (!_missing_moves.empty())
        diff_string += "Missing: " + string_join(_missing_moves, " ");

    if (!_extra_moves.empty())
    {
        if (!diff_string.empty())
            diff_string.push_back(' ');

        diff_string += "Extra: " + string_join(_extra_moves, " ");
    }
    
    return diff_string;
}
