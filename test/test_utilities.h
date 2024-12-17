//---------------------------------------------------------------------------
// Utility functions for unit tests
//---------------------------------------------------------------------------

#pragma once
// simpler include guard

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


//////////////////////////////////////////////////////////// evil recursive function template

// game specification for testing function
template <class T>
struct game_spec
{
    game_spec<T>(const std::string& board) : board(board)
    {}

    const std::string& board;

};

// general case
template <class T, class ...Ts>
void _add_to(sumgame& sum, vector<game*>& game_vec, const game_spec<T>& g1, Ts... gs)
{
    T* pos = new T(g1.board);

    game_vec.push_back(pos);
    sum.add(pos);

    _add_to(sum, game_vec, gs...);
}

// base case
template <class T>
void _add_to(sumgame& sum, vector<game*>& game_vec, const game_spec<T>& g1)
{
    T* pos = new T(g1.board);

    game_vec.push_back(pos);
    sum.add(pos);
}

template <class ...Ts>
void assert_sum_outcome_for(int player, bool expected_outcome, Ts... game_specs)
{
    assert_black_white(player);
    sumgame sum(player);

    vector<game*> games; // clean up later...

    _add_to(sum, games, game_specs...);

    bool outcome = sum.solve();

    assert(outcome == expected_outcome);

    for (game* g : games)
    {
        delete g;
    }
}

template <class ...Ts>
void assert_sum_outcome(bool black_outcome, bool white_outcome, Ts... gs)
{
    assert_sum_outcome_for(BLACK, black_outcome, gs...);
    assert_sum_outcome_for(WHITE, white_outcome, gs...);
}

//////////////////////////////////////////////////////////// other templates

template <class T>
void assert_correct_inverse(const std::string& game_as_string)
{
    {
        T* pos = new T(game_as_string);
        game* pos_negative = pos->inverse();

        sumgame sum(BLACK);
        sum.add(pos);
        sum.add(pos_negative);

        bool outcome = sum.solve();

        assert(outcome == false);

        delete pos;
        delete pos_negative;
    }

    {
        T* pos = new T(game_as_string);
        game* pos_negative = pos->inverse();

        sumgame sum(WHITE);
        sum.add(pos);
        sum.add(pos_negative);

        bool outcome = sum.solve();

        assert(outcome == false);

        delete pos;
        delete pos_negative;
    }

}


