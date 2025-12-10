#include "get_winning_moves.h"

#include <sstream>
#include <vector>
#include <memory>
#include <iostream>
#include <cassert>
#include <string>

#include "cgt_basics.h"
#include "game.h"
#include "sumgame.h"
#include "utilities.h"

using namespace std;

vector<string> get_winning_moves_for_player(sumgame& sum, bw player)
{
    assert(is_black_white(player));
    vector<string> winning_moves;

    assert_restore_sumgame ars1(sum);
    const bw restore_player = sum.to_play();

    sum.set_to_play(player);
    const bw opp = opponent(player);

    unique_ptr<sumgame_move_generator> gen(sum.create_sum_move_generator(player));
    while (*gen)
    {
        assert_restore_sumgame ars2(sum);
        sumgame_move sm = gen->gen_sum_move();

        assert(sum.to_play() == player);
        sum.play_sum(sm, player);

        // Winning move IFF opponent loses
        assert(sum.to_play() == opp);
        const bool is_winning_move = !sum.solve();

        sum.undo_move();
        assert(sum.to_play() == player);

        if (is_winning_move)
        {
            const game* g = sum.subgame_const(sm.subgame_idx);

            stringstream stream;
            stream << sm.subgame_idx << ':';
            g->print_move(stream, sm.m);

            const std::string& str = stream.str();
            assert(!string_contains_whitespace(str));
            winning_moves.emplace_back(str);
        }

        ++(*gen);
    }

    sum.set_to_play(restore_player);

    return winning_moves;
}

