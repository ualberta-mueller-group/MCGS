#pragma once

#include "bounds.h"
#include "sumgame.h"
#include "game.h"

// TODO this function is probably duplicated in several places...
outcome_class get_sum_outcome(sumgame& sum);
bool sum_rel_zero(sumgame& sum, relation rel);

bool sum_rel_scale_game(sumgame& sum, relation rel, bound_scale scale,
                        bound_t scale_idx);
bool sum_rel_game(sumgame& sum, relation rel, game& g);
bool sum_rel_sum(sumgame& sum1, relation rel, sumgame& sum2);

class restore_sumgame_player
{
public:
    inline restore_sumgame_player(sumgame& sum)
        : _sum(sum), _restore_player(sum.to_play())
    {
    }

    inline ~restore_sumgame_player() { _sum.set_to_play(_restore_player); }

private:
    sumgame& _sum;
    const bw _restore_player;
};

