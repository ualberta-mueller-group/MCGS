#include "sumgame_helpers.h"

#include "bounds.h"
#include "sumgame.h"
#include "cgt_basics.h"
#include "utilities.h"

////////////////////////////////////////////////// Helpers
namespace {
inline bool player_wins(sumgame& sum, bw player)
{
    sum.set_to_play(player);
    return sum.solve();
}


} // namespace

////////////////////////////////////////////////// Exported functions
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

bool sum_rel_zero(sumgame& sum, relation rel)
{
    assert_restore_sumgame ars(sum);
    restore_sumgame_player restore(sum);

    switch (rel)
    {
        case REL_EQUAL:
            return !player_wins(sum, BLACK) && !player_wins(sum, WHITE);
        case REL_FUZZY:
            return player_wins(sum, BLACK) && player_wins(sum, WHITE);
        case REL_LESS_OR_EQUAL:
            return !player_wins(sum, BLACK);
        case REL_LESS:
            return !player_wins(sum, BLACK) && player_wins(sum, WHITE);
        case REL_GREATER_OR_EQUAL:
            return !player_wins(sum, WHITE);
        case REL_GREATER:
            return player_wins(sum, BLACK) && !player_wins(sum, WHITE);
        case REL_UNKNOWN:
        default:
            assert(false);
    }
}

bool sum_rel_scale_game(sumgame& sum, relation rel, bound_scale scale,
                        bound_t scale_idx)
{
    assert_restore_sumgame ars(sum);

    game* g_inv = get_inverse_scale_game(scale_idx, scale);
    sum.add(g_inv);

    const bool has_relation = sum_rel_zero(sum, rel);

    sum.pop(g_inv);
    delete g_inv;

    return has_relation;
}

bool sum_rel_game(sumgame& sum, relation rel, game& g)
{
    assert_restore_sumgame ars(sum);

    game* g_inv = g.inverse();
    sum.add(g_inv);

    const bool has_relation = sum_rel_zero(sum, rel);

    sum.pop(g_inv);
    delete g_inv;

    return has_relation;
}

bool sum_rel_sum(sumgame& sum1, relation rel, sumgame& sum2)
{
    assert_restore_sumgame ars1(sum1);
    assert_restore_sumgame ars2(sum2);

    const std::vector<game*>& sum2_games = sum2.subgames();

    std::vector<game*> sum2_inv_games;
    sum2_inv_games.reserve(sum2_games.size());

    for (const game* g : sum2_games)
    {
        if (g->is_active())
            sum2_inv_games.push_back(g->inverse());
    }

    sum1.add(sum2_inv_games);

    const bool has_relation = sum_rel_zero(sum1, rel);

    sum1.pop(sum2_inv_games);

    for (game* g : sum2_inv_games)
        delete g;

    return has_relation;
}
