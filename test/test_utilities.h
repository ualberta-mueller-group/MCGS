//---------------------------------------------------------------------------
// Utility functions for unit tests
//---------------------------------------------------------------------------

#ifndef test_utilities_H
#define test_utilities_H

#include <memory>
#include "alternating_move_game.h"
#include "cgt_move.h"
#include "game.h"
#include "sumgame.h"

inline void assert_move(move_generator& mg, int mv)
{ 
    const move m = mg.gen_move();
    assert(m == mv);
}

inline void assert_num_moves(const game& g, bw to_play, int num_moves)
{ 
    std::unique_ptr<move_generator>mgp(g.create_move_generator(to_play));
    move_generator& mg(*mgp);
    for (int i = 0; i < num_moves; ++i)
    {
        assert(mg);
        ++mg;
    }
    assert(!mg);
}

inline void assert_two_part_move(move_generator& mg, int from, int to)
{ 
    move m = mg.gen_move();
    assert(from == cgt_move::from(m));
    assert(to == cgt_move::to(m));
}

void assert_solve(game& pos, bw to_play,
                  const bool expected_result);
void assert_solve_sum(sumgame& g, bw to_play, 
                      const bool expected_result);
void test_sum(sumgame& sum, bool resB, bool resW);
void test_one_game(game& g, bool resB, bool resW);
void test_two_games(game& g1, game& g2, bool resB, bool resW);
void test_three_games(game& g1, game& g2, game& g3, bool resB, bool resW);

inline void test_zero_1(game& g)
{
    test_one_game(g, false, false);
}

inline void test_zero_2(game& g1, game& g2)
{
    test_two_games(g1, g2, false, false);
}
inline void test_zero_3(game& g1, game& g2, game& g3)
{
    test_three_games(g1, g2, g3, false, false);
}

inline void test_inverse(game& g1, game& g2) // g1+g2 == 0
{
    test_zero_2(g1, g2);
}
//void test_equal(game& g1, game& g2); // needs game::inverse()
#endif // test_utilities_H
