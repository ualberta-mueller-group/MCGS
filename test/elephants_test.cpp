//------------------------------------------------------------
// Unit tests for the game Elephants and Rhinos
//------------------------------------------------------------

/*

    TODO: use these test cases later (include split() test)
        These should be in the elephants.hard.test file, but

    XX...XO.O.X..X.O..O.O.X.O.X..X..X.....O..OOO
    XX...XO.O + X..X.O..O.O + X.O + X..X..X.....O..OOO
    ?? --> N position from book, (first wins)
    XX...XO.O.X..X.O..O.O.X.O.X..X..X.....O..OOO B win
    XX...XO.O.X..X.O..O.O.X.O.X..X..X.....O..OOO W win

    X.......O.....XO.O.....X..OO.X.O.OOXO...
    X.......O + XO.O + X..OO + X.O.OO + XO...
    {-4* | -6*} --> R position (white wins)
    X.......O.....XO.O.....X..OO.X.O.OOXO... B loss
    X.......O.....XO.O.....X..OO.X.O.OOXO... W win

    X...XX.O...X.....OOOO..X...O.....O.O...O
    X...XX.O + X.....OOOO + X...O.....O.O...O
    {-27 | -29} --> R position (white wins)
    X...XX.O...X.....OOOO..X...O.....O.O...O B loss
    X...XX.O...X.....OOOO..X...O.....O.O...O W win

    X..X........O....X....O...O.......XO.X..
    X..X........O + X....O...O + XO + X..
    3 --> L position (black wins)
    X..X........O....X....O...O.......XO.X.. B win
    X..X........O....X....O...O.......XO.X.. W loss

    .......X.....O..X.....O.......
    .......X.....O + X.....O.......
    0 --> P position (first loses)
    .......X.....O..X.....O....... B loss
    .......X.....O..X.....O....... W loss

    .OXO...O.O..O.X.X......X...X..OO..X....X
    .O + XO...O.O..O + X.X......X...X..OO + X....X
    14 --> L position (black wins)
    .OXO...O.O..O.X.X......X...X..OO..X....X B win
    .OXO...O.O..O.X.X......X...X..OO..X....X W loss

    ....OXO.....X......X.XOX..O...O...O.XXO.
    ....O + XO + X......X.XO + X..O...O...O + XXO.
    -7 --> R position (white wins)
    ....OXO.....X......X.XOX..O...O...O.XXO. B loss
    ....OXO.....X......X.XOX..O...O...O.XXO. W win
*/



#include "elephants_test.h"
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

    for (size_t i = 0; i < expected.size(); i++)
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
    assert(pos.board_as_string() == "X...XX.O.OO.XX.O");
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
void file(const std::string& file_name)
{
    std::vector<test_case> cases;
    std::string game_name;
    int version;
    if (! read_test_cases(file_name, game_name, version, cases))
        return;
    assert(game_name == "elephants");
    assert(version == 0);
    for (const test_case& c: cases)
    {
        test(c);
    }
}

void undo1()
{
    elephants pos(".X.O.X..O.O");

    pos.play(cgt_move::two_part_move(1, 2), BLACK);
    assert(pos.board_as_string() == "..XO.X..O.O");

    pos.play(cgt_move::two_part_move(10, 9), WHITE);
    assert(pos.board_as_string() == "..XO.X..OO.");


    pos.undo_move();
    assert(pos.board_as_string() == "..XO.X..O.O");

    pos.undo_move();
    assert(pos.board_as_string() == ".X.O.X..O.O");
}


void undo2()
{
    elephants pos(".OO.X.O.XOX..O..X.X");
    pos.play(cgt_move::two_part_move(1, 0), WHITE);
    assert(pos.board_as_string() == "O.O.X.O.XOX..O..X.X");
    pos.play(cgt_move::two_part_move(2, 1), WHITE);
    assert(pos.board_as_string() == "OO..X.O.XOX..O..X.X");

    pos.undo_move();
    assert(pos.board_as_string() == "O.O.X.O.XOX..O..X.X");
    pos.undo_move();
    assert(pos.board_as_string() == ".OO.X.O.XOX..O..X.X");

}

void inverse1()
{
    elephants pos("X..O.O.XX.O");

    unique_ptr<elephants> inv(dynamic_cast<elephants*>(pos.inverse()));
    assert(inv.get() != nullptr);
    assert(inv->board_as_string() == "X.OO.X.X..O");
}

void inverse2()
{
    elephants pos("...");

    unique_ptr<elephants> inv(dynamic_cast<elephants*>(pos.inverse()));
    assert(inv.get() != nullptr);
    assert(inv->board_as_string() == "...");
}



void test_is_move()
{
    elephants pos(".XX.O.OO.X");
    unique_ptr<elephants_move_generator> gen_b(dynamic_cast<elephants_move_generator*>(pos.create_move_generator(BLACK)));
    unique_ptr<elephants_move_generator> gen_w(dynamic_cast<elephants_move_generator*>(pos.create_move_generator(WHITE)));

    assert(gen_b.get() != nullptr);
    assert(gen_w.get() != nullptr);

    assert(!gen_b->is_move(1, 2, BLACK));
    assert(gen_b->is_move(2, 3, BLACK));
    assert(!gen_b->is_move(3, 2, BLACK));
    assert(!gen_b->is_move(9, 10, BLACK));

    assert(gen_w->is_move(4, 3, WHITE));
    assert(gen_w->is_move(6, 5, WHITE));
    assert(!gen_w->is_move(7, 6, WHITE));
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
    undo1();
    undo2();
    inverse1();
    inverse2();
    test_is_move();
    file("input/elephants.test");

    // Use these later, for now they're too hard
    //file("elephants.hard.test");
}
