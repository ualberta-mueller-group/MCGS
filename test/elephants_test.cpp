//------------------------------------------------------------
// Unit tests for the game Elephants and Rhinos
//------------------------------------------------------------
#include "cgt_move.h"
#include "elephants.h"

#include <iostream>
#include "test_utilities.h"
#include "test_case.h"
#include <memory>
#include <vector>
#include <algorithm>



using std::cout, std::endl, std::unique_ptr, std::vector;

void assert_same_moves(const game& g, bw to_play, vector<move>& expected)
{
    unique_ptr<move_generator> gen(g.create_move_generator(to_play));

    vector<move> moves;

    for (; *gen; ++(*gen))
    {
        moves.push_back(gen->gen_move());
    }

    // Sort both move sets
    auto compare = [](const move& m1, const move& m2) -> bool
    {
        return m1 < m2; // TODO should change this
    };

    std::sort(expected.begin(), expected.end(), compare);
    std::sort(moves.begin(), moves.end(), compare);

    assert(expected.size() == moves.size());

    for (int i = 0; i < expected.size(); i++)
    {
        assert(expected[i] == moves[i]);
    }
}



/*

    various 0s
    simple games
manually play some moves, check the board

    move generator moves

test creating board as string

test various game files
   */


void zero1()  {
    elephants pos("XO");
    assert_solve(pos, BLACK, false);
    assert_solve(pos, WHITE, false);

    assert_num_moves(pos, BLACK, 0);
    assert_num_moves(pos, WHITE, 0);
}

void zero2()  {
    elephants pos(".....XOXOXOXOXO....");
    assert_solve(pos, BLACK, false);
    assert_solve(pos, WHITE, false);

    assert_num_moves(pos, BLACK, 0);
    assert_num_moves(pos, WHITE, 0);
}


void zero3()  {
    elephants pos("O..XO...XO..XOXO..X");
    assert_solve(pos, BLACK, false);
    assert_solve(pos, WHITE, false);

    assert_num_moves(pos, BLACK, 0);
    assert_num_moves(pos, WHITE, 0);
}



void zero4()  {
    elephants pos("X.....XO.....O");
    assert_solve(pos, BLACK, false);
    assert_solve(pos, WHITE, false);

    assert_num_moves(pos, BLACK, 1);
    assert_num_moves(pos, WHITE, 1);

    unique_ptr<move_generator> mg1(pos.create_move_generator(BLACK));
    unique_ptr<move_generator> mg2(pos.create_move_generator(WHITE));

    assert(*mg1);
    assert(*mg2);

    const move m1 = mg1->gen_move();
    assert(cgt_move::from(m1) == 0);
    assert(cgt_move::to(m1) == 1);

    const move m2 = mg2->gen_move();
    assert(cgt_move::from(m2) == 13);
    assert(cgt_move::to(m2) == 12);

}

void simple1() {
    elephants pos("X.X.X.X.O.O.O");
    assert_solve(pos, BLACK, true);
    assert_solve(pos, WHITE, false);

    assert_num_moves(pos, BLACK, 4);
    assert_num_moves(pos, WHITE, 3);


    vector<move> black_moves;
    vector<move> white_moves;

    auto add_move = [](vector<move>& vec, int from, int to) -> void
    {
        vec.push_back(cgt_move::two_part_move(from, to));
    };

    add_move(black_moves, 0, 1);
    add_move(black_moves, 2, 3);
    add_move(black_moves, 4, 5);
    add_move(black_moves, 6, 7);

    add_move(white_moves, 8, 7);
    add_move(white_moves, 10, 9);
    add_move(white_moves, 12, 11);

    assert_same_moves(pos, BLACK, black_moves);
    assert_same_moves(pos, WHITE, white_moves);
    
}

void simple2() {
    elephants pos("X..X.O..O.O");
    assert_solve(pos, BLACK, false);
    assert_solve(pos, WHITE, true);

    assert_num_moves(pos, BLACK, 2);
    assert_num_moves(pos, WHITE, 3);


    vector<move> black_moves;
    vector<move> white_moves;

    auto add_move = [](vector<move>& vec, int from, int to) -> void
    {
        vec.push_back(cgt_move::two_part_move(from, to));
    };

    add_move(black_moves, 0, 1);
    add_move(black_moves, 3, 4);

    add_move(white_moves, 5, 4);
    add_move(white_moves, 8, 7);
    add_move(white_moves, 10, 9);

    assert_same_moves(pos, BLACK, black_moves);
    assert_same_moves(pos, WHITE, white_moves);
    
}

void manual1() {
    elephants pos("X..X.X.O.OO.XX.O");

    assert(pos.board_as_string() == "X..X.X.O.OO.XX.O");
    pos.play(cgt_move::two_part_move(3, 4), BLACK);
    pos.play(cgt_move::two_part_move(7, 6), WHITE);

    assert(pos.board_as_string() == "X...XXO..OO.XX.O");
}


// lifted from clobber_1xn_test.cpp
void test(const test_case &c)
{
    elephants pos(c._game);
    const bw to_play = c._black_first ? BLACK : WHITE;
    alternating_move_game g(pos, to_play);
    const bool result = g.solve();
    if (result != c._is_win)
    {
        cout << "Test failed game " << pos << ' ' 
             << c._black_first << " expected " << c._is_win << endl;
        assert(false);
    }
}

// lifted from clobber_1xn_test.cpp
void file()
{
    std::vector<test_case> cases;
    std::string game_name;
    int version;
    if (! read_test_cases("elephants.test", game_name, version, cases))
        return;
    assert(game_name == "elephants");
    assert(version == 0);
    for (const test_case& c: cases)
    {
        test(c);
    }
}




void elephants_test_all()
{
    zero1();
    zero2();
    zero3();
    zero4();
    simple1();
    simple2();
    manual1();
    file();
}
