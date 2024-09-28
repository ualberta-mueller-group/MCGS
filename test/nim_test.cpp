//---------------------------------------------------------------------------
// Unit tests for the game of nim
//---------------------------------------------------------------------------
#include "nim.h"

#include <cassert>
#include <iostream>
#include "test_case.h"
#include "test_utilities.h"

using std::cout;
using std::endl;

void assert_solve(nim& pos)
{
    alternating_move_game g(pos, BLACK);
    const bool result = g.solve();
    assert(result == pos.static_solve());
}

namespace {
void nim_test_zero()
{
    nim g("");
    assert_solve(g);
}

void nim_test_zero2()
{
    nim g("0");
    assert_solve(g);
}

void nim_test_1()
{
    nim g("1");
    assert_solve(g);
}

void nim_test_2()
{
    nim g("2");
    assert_solve(g);
}

void nim_test_3()
{
    nim g("10");
    assert_solve(g);
}

void nim_test_sum_1()
{
    nim g("1 0");
    assert_solve(g);
}

void nim_test_sum_2()
{
    nim g("1 1");
    assert_solve(g);
}

void nim_test_sum_3()
{
    nim g("4 5");
    assert_solve(g);
}

void nim_test_sum_4()
{
    nim g("1 2 3");
    assert_solve(g);
}

void nim_test_sum_5()
{
    nim g("3 4 5 6");
    assert_solve(g);
}

void nim_test_sum_6()
{
    nim g("3 4 5 2");
    assert_solve(g);
}


void nim_move_generator_test_1()
{
    nim g("2 0 1");
    assert(g.heaps().size() == 2); // removed 0 heap so heaps = [2,1]
    
    std::unique_ptr<move_generator>mgp(g.create_move_generator(BLACK));
    move_generator& mg(*mgp);
    assert(mg);
    assert_move(mg, 0, 1);
    ++mg;
    assert(mg);
    assert_move(mg, 0, 2);
    ++mg;
    assert(mg);
    assert_move(mg, 1, 1);
    ++mg;
    assert(!mg);
}

void test(const test_case &c)
{
    nim pos(c._game);
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

void nim_test_file()
{
    std::vector<test_case> cases;
    std::string game_name;
    int version;
    if (! read_test_cases("nim.test", game_name, version, cases))
        return;
    assert(game_name == "nim");
    assert(version == 0);
    for (const test_case& c: cases)
    {
        test(c);
    }
}

} // namespace

void nim_test_all()
{
    nim_test_zero();
    nim_test_zero2();
    nim_move_generator_test_1();
    nim_test_1();
    nim_test_2();
    nim_test_3();
    nim_test_sum_1();
    nim_test_sum_2();
    nim_test_sum_3();
    nim_test_sum_4();
    nim_test_sum_5();
    nim_test_sum_6();
    nim_test_file();
}