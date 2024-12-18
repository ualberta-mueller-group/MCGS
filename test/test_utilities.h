//---------------------------------------------------------------------------
// Utility functions for unit tests
//---------------------------------------------------------------------------

#pragma once
// simpler include guard

#include <memory>
#include <tuple>
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

// Interface of instantiated game factory
struct game_factory
{
    virtual game* new_game() const = 0;
};


// Messy hidden internals of game factory
template <class T, class ...Ts>
struct _game_factory_impl : public game_factory
{
    // T is type of game
    // ...Ts is types of arguments to game constructor

    _game_factory_impl(Ts... args) : data(args...)
    { }

    game* new_game() const override
    {
        return std::apply(_make_new, data);
    }

private:

    // this is just here so we can call std::apply to unpack the tuple
    static game* _make_new(Ts... args)
    {
        return new T(args...);
    }


    std::tuple<Ts...> data;
};

/*  i.e.
    make_factory<clobber_1xn>("XO.OX");
    make_factory<switch_game>(5, 3);

*/
template <class T, class ...Ts>
_game_factory_impl<T, Ts...> make_factory(Ts... data)
{
    return _game_factory_impl<T, Ts...>(data...);
}


// general case
template <class T, class ...Ts>
void _add_games(sumgame& sum, vector<game*>& game_vec, const T& factory, Ts... factories)
{
    const game_factory& _factory = factory;

    game* pos = _factory.new_game();

    game_vec.push_back(pos);
    sum.add(pos);

    _add_games(sum, game_vec, factories...);
}

// base case
template<class T>
void _add_games(sumgame& sum, vector<game*>& game_vec, const T& factory)
{
    const game_factory& _factory = factory;

    game* pos = _factory.new_game();

    game_vec.push_back(pos);
    sum.add(pos);
}

template <class ...Ts>
void assert_player_sum_outcome(int player, bool expected_outcome, Ts... factories)
{
    assert_black_white(player);
    sumgame sum(player);

    vector<game*> games; // clean up later...

    _add_games(sum, games, factories...);

    bool outcome = sum.solve();

    assert(outcome == expected_outcome);

    for (game* g : games)
    {
        delete g;
    }
}

template <class ...Ts>
void assert_sum_outcomes(bool black_outcome, bool white_outcome, Ts... factories)
{
    assert_player_sum_outcome(BLACK, black_outcome, factories...);
    assert_player_sum_outcome(WHITE, white_outcome, factories...);
}

//////////////////////////////////////////////////////////// other templates

inline void assert_inverse_sum_zero(const game_factory& factory)
{

    for (int i = 0; i <= 1; i++)
    {
        int to_play = i == 0 ? BLACK : WHITE;

        game* pos = factory.new_game();
        game* pos_negative = pos->inverse();

        sumgame sum(to_play);
        sum.add(pos);
        sum.add(pos_negative);

        bool outcome = sum.solve();

        assert(outcome == false);

        delete pos;
        delete pos_negative;
    }

}






