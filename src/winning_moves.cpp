#include "winning_moves.h"
#include "cgt_basics.h"
#include "game.h"
#include "sumgame.h"

// TODO more unit tests?
std::vector<move> get_winning_moves(const game* g, bw to_play)
{
    std::vector<move> moves;
    assert_restore_game arg(*g);

    game* g_casted = const_cast<game*>(g);

    const bw opp = opponent(to_play);
    sumgame sum(opp);

    std::unique_ptr<move_generator> gen(g_casted->create_move_generator(to_play));

    while (*gen)
    {
        assert_restore_game arg2(*g);

        move m = gen->gen_move();
        ++(*gen);

        g_casted->play(m, to_play);

        assert(sum.is_empty() && sum.to_play() == opp);
        bool opp_win = sum.solve_with_games(g_casted);
        assert(sum.is_empty() && sum.to_play() == opp);

        if (!opp_win)
            moves.push_back(m);

        g_casted->undo_move();
    }

    return moves;
}
