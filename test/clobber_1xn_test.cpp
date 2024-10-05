//---------------------------------------------------------------------------
// Unit tests for the game of clobber
//---------------------------------------------------------------------------
#include "clobber_1xn.h"

#include <cassert>
#include <iostream>
#include "cgt_move.h"
#include "test_case.h"
#include "test_utilities.h"

using std::cout;
using std::endl;

namespace {
void assert_solve(clobber_1xn& pos, bw to_play, bool expected_result)
{
    alternating_move_game g(pos, BLACK);
    const bool result = g.solve();
    assert(result == expected_result);
}

void clobber_1xn_test_zero()
{
    clobber_1xn g("");
    assert_solve(g, BLACK, false);
}

void clobber_1xn_test_zero2()
{
    clobber_1xn g("X");
    assert_solve(g, BLACK, false);
}

void clobber_1xn_test_1()
{
    clobber_1xn g("XO");
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, true);
}

void clobber_1xn_test_2()
{
    clobber_1xn g("XOX");
    assert_solve(g, BLACK, true);
}

void clobber_1xn_test_4()
{
    clobber_1xn g("XOXOXO");
    assert_solve(g, BLACK, false);
}

void clobber_1xn_test_5()
{
    clobber_1xn g("XXOXOXOOX");
    assert_solve(g, BLACK, true);
}

void clobber_1xn_test_3()
{
    clobber_1xn g("XOXO");
    assert_solve(g, BLACK, true);
}

void clobber_1xn_test_sum_1()
{
    clobber_1xn g("XO.X");
    assert_solve(g, BLACK, true);
}

void clobber_1xn_test_sum_2()
{
    clobber_1xn g("XO.OX");
    assert_solve(g, BLACK, false);
}

void clobber_1xn_test_sum_3()
{
    clobber_1xn g("XO.XO.XO");
    assert_solve(g, BLACK, true);
}

void clobber_1xn_test_sum_4()
{
    clobber_1xn g("XOX.OXO");
    assert_solve(g, BLACK, false);
}

void clobber_1xn_test_sum_5()
{
    clobber_1xn g("XO.XOXOXO");
    assert_solve(g, BLACK, true);
}

void clobber_1xn_test_sum_6()
{
    clobber_1xn g("XOXOXO.XOXOXO");
    assert_solve(g, BLACK, false);
}

void clobber_1xn_move_generator_test_1()
{
    clobber_1xn g("XXX.OOO");
    
    std::unique_ptr<move_generator>mgp(g.create_move_generator(BLACK));
    move_generator& mg(*mgp);
    
    assert(!mg);
}

void clobber_1xn_move_generator_test_2()
{
    clobber_1xn g("XO");
    
    std::unique_ptr<move_generator>mgp(g.create_move_generator(BLACK));
    move_generator& mg(*mgp);
    assert(mg);
    assert_two_part_move(mg, 0, 1);
    ++mg;
    assert(!mg);
}

void clobber_1xn_move_generator_test_3()
{
    clobber_1xn g("OXOX");
    
    std::unique_ptr<move_generator>mgp(g.create_move_generator(BLACK));
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

void clobber_1xn_move_generator_test_4()
{
    clobber_1xn g("OXOX");
    
    std::unique_ptr<move_generator>mgp(g.create_move_generator(WHITE));
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

void clobber_1xn_test_string_1()
{
    std::string board("OXOX");
    clobber_1xn g(board);
    std::string output = g.board_as_string();
    assert(board == output);
}

void clobber_1xn_test_string_2()
{
    std::string board("OXOX.XXX.OOO");
    clobber_1xn g(board);
    std::string output = g.board_as_string();
    assert(board == output);
}

void clobber_1xn_test_string_3()
{
    std::string board("");
    clobber_1xn g(board);
    std::string output = g.board_as_string();
    assert(board == output);
}

void test(const test_case &c)
{
    clobber_1xn pos(c._game);
    const bw to_play = c._black_first ? BLACK : WHITE;
    alternating_move_game g(pos, to_play);
    const bool result = g.solve();
    if (result != c._is_win)
    {
        cout << "Test failed game " << pos << ' ' 
             << c._black_first << " expected " << c._is_win << endl;
        assert(false);
    }
    // TODO write_bw(to_play), not c._black_first
}

void clobber_test_file()
{
    std::vector<test_case> cases;
    std::string game_name;
    int version;
    if (! read_test_cases("clobber_1xn.test", game_name, version, cases))
        return;
    assert(game_name == "clobber_1xn");
    assert(version == 0);
    for (const test_case& c: cases)
    {
        test(c);
    }
}

} // namespace

void clobber_1xn_test_all()
{
    clobber_1xn_test_zero();
    clobber_1xn_test_zero2();
    clobber_1xn_move_generator_test_1();
    clobber_1xn_move_generator_test_2();
    clobber_1xn_move_generator_test_3();
    clobber_1xn_move_generator_test_4();
    clobber_1xn_test_1();
    clobber_1xn_test_2();
    clobber_1xn_test_3();
    clobber_1xn_test_4();
    clobber_1xn_test_5();
    clobber_1xn_test_sum_1();
    clobber_1xn_test_sum_2();
    clobber_1xn_test_sum_3();
    clobber_1xn_test_sum_4();
    clobber_1xn_test_sum_5();
    clobber_1xn_test_sum_6();
    clobber_1xn_test_string_1();
    clobber_1xn_test_string_2();
    clobber_1xn_test_string_3();
    clobber_test_file();
}