//---------------------------------------------------------------------------
// Unit tests for the game of clobber
//---------------------------------------------------------------------------
#include "clobber_1xn_test.h"
#include "clobber_1xn.h"

#include <cassert>
#include <iostream>
#include "cgt_move.h"
#include "test_utilities.h"

using std::cout;
using std::endl;

namespace {
namespace clobber_1xn_test {

void zero()
{
    clobber_1xn g("");
    assert_solve(g, BLACK, false);
}

void zero2()
{
    clobber_1xn g("X");
    assert_solve(g, BLACK, false);
}

void game_xo()
{
    clobber_1xn g("XO");
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, true);
}

void game_xox()
{
    clobber_1xn g("XOX");
    assert_solve(g, BLACK, true);
}

void game_xoxo()
{
    clobber_1xn g("XOXO");
    assert_solve(g, BLACK, true);
}

void game_xoxoxo()
{
    clobber_1xn g("XOXOXO");
    assert_solve(g, BLACK, false);
}

void game_9()
{
    clobber_1xn g("XXOXOXOOX");
    assert_solve(g, BLACK, true);
}

void sum_1()
{
    clobber_1xn g("XO.X");
    assert_solve(g, BLACK, true);
}

void sum_2()
{
    clobber_1xn g("XO.OX");
    assert_solve(g, BLACK, false);
}

void sum_3()
{
    clobber_1xn g("XO.XO.XO");
    assert_solve(g, BLACK, true);
}

void sum_4()
{
    clobber_1xn g("XOX.OXO");
    assert_solve(g, BLACK, false);
}

void sum_5()
{
    clobber_1xn g("XO.XOXOXO");
    assert_solve(g, BLACK, true);
}

void sum_6()
{
    clobber_1xn g("XOXOXO.XOXOXO");
    assert_solve(g, BLACK, false);
}

void play_two_white_moves()
{
    clobber_1xn g("XXO");
    g.play(cgt_move::two_part_move(2, 1), WHITE);
    assert(g.board_as_string() == "XO.");
    g.play(cgt_move::two_part_move(1, 0), WHITE);
    assert(g.board_as_string() == "O..");
    g.undo_move();
    assert(g.board_as_string() == "XO.");
    g.undo_move();
    assert(g.board_as_string() == "XXO");
}

} // namespace clobber_1xn_test

namespace clobber_1xn_move_generator_test {

void test_1()
{
    clobber_1xn g("XXX.OOO");

    std::unique_ptr<move_generator> mgp(g.create_move_generator(BLACK));
    move_generator& mg(*mgp);

    assert(!mg);
}

void test_2()
{
    clobber_1xn g("XO");

    std::unique_ptr<move_generator> mgp(g.create_move_generator(BLACK));
    move_generator& mg(*mgp);
    assert(mg);
    assert_two_part_move(mg, 0, 1);
    ++mg;
    assert(!mg);
}

void test_3()
{
    clobber_1xn g("OXOX");

    std::unique_ptr<move_generator> mgp(g.create_move_generator(BLACK));
    move_generator& mg(*mgp);
    assert(mg);
    assert_two_part_move(mg, 1, 2);
    ++mg;
    assert(mg);
    assert_two_part_move(mg, 1, 0);
    ++mg;
    assert(mg);
    assert_two_part_move(mg, 3, 2);
    ++mg;
    assert(!mg);
}

void test_4()
{
    clobber_1xn g("OXOX");

    std::unique_ptr<move_generator> mgp(g.create_move_generator(WHITE));
    move_generator& mg(*mgp);
    assert(mg);
    assert_two_part_move(mg, 0, 1);
    ++mg;
    assert(mg);
    assert_two_part_move(mg, 2, 3);
    ++mg;
    assert(mg);
    assert_two_part_move(mg, 2, 1);
    ++mg;
    assert(!mg);
}
} // namespace clobber_1xn_move_generator_test

namespace clobber_1xn_string_test {

void oxox()
{
    std::string board("OXOX");
    clobber_1xn g(board);
    std::string output = g.board_as_string();
    assert(board == output);
}

void string_12()
{
    std::string board("OXOX.XXX.OOO");
    clobber_1xn g(board);
    std::string output = g.board_as_string();
    assert(board == output);
}

void empty()
{
    std::string board("");
    clobber_1xn g(board);
    std::string output = g.board_as_string();
    assert(board == output);
}

void file()
{
    assert_solve_test_file(UNIT_TEST_INPUT_DIR + "/clobber_1xn.test", 14);
}

} // namespace clobber_1xn_string_test
} // namespace

void clobber_1xn_test_all()
{
    clobber_1xn_test::zero();
    clobber_1xn_test::zero2();
    clobber_1xn_test::game_xo();
    clobber_1xn_test::game_xox();
    clobber_1xn_test::game_xoxo();
    clobber_1xn_test::game_xoxoxo();
    clobber_1xn_test::game_9();
    clobber_1xn_test::sum_1();
    clobber_1xn_test::sum_2();
    clobber_1xn_test::sum_3();
    clobber_1xn_test::sum_4();
    clobber_1xn_test::sum_5();
    clobber_1xn_test::sum_6();
    clobber_1xn_test::play_two_white_moves();
    clobber_1xn_move_generator_test::test_1();
    clobber_1xn_move_generator_test::test_2();
    clobber_1xn_move_generator_test::test_3();
    clobber_1xn_move_generator_test::test_4();
    clobber_1xn_string_test::oxox();
    clobber_1xn_string_test::string_12();
    clobber_1xn_string_test::empty();
    clobber_1xn_string_test::file();
}
