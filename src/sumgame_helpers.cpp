#include "sumgame_helpers.h"

outcome_class get_sum_outcome(sumgame& sum)
{
    const bw restore_player = sum.to_play();

    sum.set_to_play(BLACK);
    const bool black_wins = sum.solve();

    sum.set_to_play(WHITE);
    const bool white_wins = sum.solve();

    sum.set_to_play(restore_player);

    return bools_to_outcome_class(black_wins, white_wins);

}

