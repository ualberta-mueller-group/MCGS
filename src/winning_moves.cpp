#include "winning_moves.h"

#include <cctype>
#include <sstream>
#include <vector>
#include <memory>
#include <iostream>
#include <cassert>

#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"
#include "search_utils.h"
#include "sumgame.h"
#include "throw_assert.h"
#include "file_parser.h"
#include "utilities.h"

using namespace std;


/*
// TODO more unit tests?
std::vector<::move> get_winning_moves(const game* g, bw to_play)
{
    std::vector<::move> moves;
    assert_restore_game arg(*g);

    game* g_casted = const_cast<game*>(g);

    const bw opp = opponent(to_play);
    sumgame sum(opp);

    std::unique_ptr<move_generator> gen(g_casted->create_move_generator(to_play));

    while (*gen)
    {
        assert_restore_game arg2(*g);

        ::move m = gen->gen_move();
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
        const std::vector<::move> winning_moves = get_winning_moves(g, gc.to_play);
        if (winning_moves.empty())
            std::cout << "None";
        else
        {
            for (::move m : winning_moves)
            {
                g->print_move(std::cout, m);
                std::cout << ' ';
            }
        }
        std::cout << std::endl;

        gc.cleanup_games();
    }

}
*/

//////////////////////////////////////////////////
namespace {

// True IFF printed some move
void print_winning_moves_for_player(sumgame& sum, bw player)
{
    assert(is_black_white(player));

    assert_restore_sumgame ars1(sum);
    const bw restore_player = sum.to_play();

    sum.set_to_play(player);
    const bw opp = opponent(player);

    bool found_winning_move = false;

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

        found_winning_move |= is_winning_move;

        if (is_winning_move)
        {
            const game* g = sum.subgame_const(sm.subgame_idx);

            stringstream stream;
            g->print_move(stream, sm.m);
            const std::string& str = stream.str();
            assert(!string_contains_whitespace(str));

            cout << sm.subgame_idx << ':';
            cout << str << ' ';
        }

        ++(*gen);
    }

    if (!found_winning_move)
        cout << "None";

    cout << endl;

    sum.set_to_play(restore_player);
}

} // namespace

void print_winning_moves_new(std::shared_ptr<file_parser> fp)
{
    game_case gc;
    sumgame sum(BLACK);
    assert_restore_sumgame ars1(sum);

    bool first_iteration = true;

    while (fp->parse_chunk(gc))
    {
        assert_restore_sumgame ars2(sum);
        assert(sum.num_total_games() == 0 &&    //
               is_empty_black_white(gc.to_play) //
        );

        if (!first_iteration)
            cout << '\n';
        first_iteration = false;

        sum.add(gc.games);

        THROW_ASSERT(LOGICAL_IMPLIES(gc.to_play == EMPTY, sum.all_impartial()),
                     "Tried to print winning moves for sum with {N} command, "
                     "but sum is not impartial!");


        cout << "Find winning moves for sum:\n";

        {
            const int n_games = sum.num_total_games();

            assert(n_games >= 0 &&                                       //
                   static_cast<unsigned int>(n_games) == gc.games.size() //
            );

            for (int i = 0; i < n_games; i++)
            {
                const game* g = sum.subgame_const(i);
                assert(g == gc.games[i]);
                cout << "\t" << i << ": " << *g << "\n";
            }

        }

        cout << "Player: " << player_name_bw_imp(gc.to_play) << '\n';

        bw effective_player = gc.to_play;
        if (effective_player == EMPTY)
            effective_player = BLACK;

        cout << "Winning moves: ";
        print_winning_moves_for_player(sum, effective_player);

        sum.pop(gc.games);
        gc.cleanup_games();
    }
}
