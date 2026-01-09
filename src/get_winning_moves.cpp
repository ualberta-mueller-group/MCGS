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

#include "cgt_basics.h"
#include "game.h"
#include "sumgame.h"
#include "utilities.h"

using namespace std;

namespace {
string sum_move_string(const sumgame& sum, const sumgame_move& sm, ebw player)
{
    const game* g = sum.subgame_const(sm.subgame_idx);

    stringstream stream;

    stream << sm.subgame_idx << ':';
    g->print_move(stream, sm.m, player);

    const string str = stream.str();
    assert(!string_contains_whitespace(str));

    return str;
}

optional<vector<string>> get_winning_moves_impl(
    sumgame& sum, bw player, const timeout_token& timeout_tok)
{
    assert(is_black_white(player));
    optional<vector<string>> winning_moves = vector<string>();

    assert_restore_sumgame ars1(sum);
    const bw restore_player = sum.to_play();

    sum.set_to_play(player);
    const bw opp = opponent(player);

    bool over_time = false;

    unique_ptr<sumgame_move_generator> gen(
        sum.create_sum_move_generator(player));
    while (*gen)
    {
        over_time = timeout_tok.stop_requested();

        if (over_time)
            break;

        sumgame_move sm = gen->gen_sum_move();
        optional<solve_result> opponent_result;

        {
            assert_restore_sumgame ars2(sum);

            assert(sum.to_play() == player);
            sum.play_sum(sm, player);

            // Winning move IFF opponent loses
            assert(sum.to_play() == opp);
            opponent_result = sum.solve_with_timeout_token(timeout_tok);

            sum.undo_move();
            assert(sum.to_play() == player);
        }

        if (!opponent_result.has_value())
        {
            over_time = true;
            assert(timeout_tok.stop_requested());

            break;
        }

        const bool is_winning_move = !opponent_result.value().win;

        assert_restore_sumgame ars3(sum);

        if (is_winning_move)
        {
            const string winning_move = sum_move_string(sum, sm, player);
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
vector<string> get_winning_moves(sumgame& sum, bw player)
{
    optional<vector<string>> result =
        get_winning_moves_with_timeout(sum, player, 0);

    assert(result.has_value());
    return result.value();
}

optional<vector<string>> get_winning_moves_with_timeout(
    sumgame& sum, bw player, unsigned long long timeout_ms)
{
    timeout_source src;
    timeout_token tok = src.get_timeout_token();

    src.start_timeout(timeout_ms);
    optional<vector<string>> result =
        get_winning_moves_with_timeout_token(sum, player, tok);
    src.cancel_timeout();

    return result;
}

optional<vector<string>> get_winning_moves_with_timeout_token(
    sumgame& sum, bw player, const timeout_token& timeout_tok)
{
    return get_winning_moves_impl(sum, player, timeout_tok);
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

    const size_t n_moves = _extra_moves.size() + _missing_moves.size();

    // Reserve enough space
    {
        size_t n_move_chars = 0;

        for (const string& str : _extra_moves)
            n_move_chars += str.size();
        for (const string& str : _missing_moves)
            n_move_chars += str.size();

        size_t total_chars = 0;
        total_chars += n_moves; // diff symbols
        if (n_moves > 0)
            total_chars += n_moves - 1; // space separators
        total_chars += n_move_chars; // move contents

        diff_string.reserve(total_chars);
    }

    size_t move_idx = 0;

    for (const string& extra : _extra_moves)
    {
        if (prepend_diff_symbols)
            diff_string.push_back('+');

        diff_string += extra;

        move_idx++;
        if (move_idx < n_moves)
            diff_string.push_back(' ');
    }

    for (const string& missing : _missing_moves)
    {
        if (prepend_diff_symbols)
            diff_string.push_back('-');

        diff_string += missing;

        move_idx++;
        if (move_idx < n_moves)
            diff_string.push_back(' ');
    }

    return diff_string;
}
