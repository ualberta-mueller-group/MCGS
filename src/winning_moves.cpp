#include "winning_moves.h"

#include <vector>
#include <memory>
#include <iostream>
#include <cassert>

#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"
#include "sumgame.h"
#include "sheep.h"
#include "throw_assert.h"
#include "file_parser.h"

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

void print_winning_moves_impl(std::shared_ptr<file_parser> fp)
{
    game_case gc;

    while (fp->parse_chunk(gc))
    {
        THROW_ASSERT(gc.games.size() == 1 && is_black_white(gc.to_play));

        game* g = gc.games.back();
        THROW_ASSERT(g != nullptr);

        std::cout << *g << '\n' << "Winning moves: ";
        const std::vector<move> winning_moves = get_winning_moves(g, gc.to_play);
        if (winning_moves.empty())
            std::cout << "None";
        else
        {
            for (move m : winning_moves)
            {
                g->print_move(std::cout, m);
                std::cout << ' ';
            }
        }
        std::cout << std::endl;

        gc.cleanup_games();
    }

}

