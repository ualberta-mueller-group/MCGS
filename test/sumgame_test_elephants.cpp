#include "sumgame_test_elephants.h"
#include "cgt_basics.h"
#include "elephants.h"
#include "sumgame.h"

#include <iostream>
using std::cout, std::endl;

using std::string;


//////////////////////////////////////////////////////////// evil recursive function template

// game specification for testing function
template <class T>
struct game_spec
{
    game_spec<T>(const string& board) : board(board)
    {}

    const string& board;

};

// general case
template <class T, class ...Ts>
void add_to(sumgame& sum, vector<game*>& game_vec, const game_spec<T>& g1, Ts... gs)
{
    T* pos = new T(g1.board);

    game_vec.push_back(pos);
    sum.add(pos);

    add_to(sum, game_vec, gs...);
}

// base case
template <class T>
void add_to(sumgame& sum, vector<game*>& game_vec, const game_spec<T>& g1)
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

    add_to(sum, games, game_specs...);

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
void assert_correct_inverse(const string& game_as_string)
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

//////////////////////////////////////////////////////////// tests

void elephants1()
{
    // 1* (this game is positive)
    assert_sum_outcome(true, false, 
        game_spec<elephants>("X.O.X.X")
    );
}

void elephants2()
{
    // {* | -1*} (this game is negative, not 0)
    assert_sum_outcome(false, true, 
        game_spec<elephants>("X.O.X.X"),
        game_spec<elephants>("O.X.O.O")
    );

}

void elephants3()
{
    // 0
    assert_sum_outcome(false, false, 
        game_spec<elephants>("X.O.X.X"),
        game_spec<elephants>("O.O.X.O")
    );

}

void elephants4()
{
    assert_correct_inverse<elephants>("X.O.X.X");
}

void elephants5()
{
    assert_correct_inverse<elephants>("O....X");
    assert_correct_inverse<elephants>("X....O");
    assert_correct_inverse<elephants>("X.X.O.O");
}


void sumgame_test_elephants_all()
{
    elephants1();
    elephants2();
    elephants3();
    elephants4();
    elephants5();
}
