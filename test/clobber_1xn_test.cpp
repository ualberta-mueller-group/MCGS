#include "clobber_1xn.h"

#include <cassert>
#include <iostream>
#include "cgt_move.h"

using std::cout;
using std::endl;

void clobber_1xn_test_zero()
{
    clobber_1xn g("");
    bool result = g.solve();
    assert(result == false);
}

void clobber_1xn_test_zero2()
{
    clobber_1xn g("X");
    bool result = g.solve();
    assert(result == false);
}

void clobber_1xn_test_1()
{
    clobber_1xn g("XO");
    bool result = g.solve();
    assert(result == true);
    g.set_to_play(WHITE);
    result = g.solve();
    assert(result == true);
}

void clobber_1xn_test_2()
{
    clobber_1xn g("XOX");
    bool result = g.solve();
    assert(result == true);
}

void clobber_1xn_test_4()
{
    clobber_1xn g("XOXOXO");
    bool result = g.solve();
    assert(result == false);
}

void clobber_1xn_test_5()
{
    clobber_1xn g("XXOXOXOOX");
    bool result = g.solve();
    assert(result == true);
}

void clobber_1xn_test_3()
{
    clobber_1xn g("XOXO");
    bool result = g.solve();
    assert(result == true);
}

void clobber_1xn_test_sum_1()
{
    clobber_1xn g("XO.X");
    bool result = g.solve();
    assert(result == true);
}

void clobber_1xn_test_sum_2()
{
    clobber_1xn g("XO.OX");
    bool result = g.solve();
    assert(result == false);
}

void clobber_1xn_test_sum_3()
{
    clobber_1xn g("XO.XO.XO");
    bool result = g.solve();
    assert(result == true);
}

void clobber_1xn_test_sum_4()
{
    clobber_1xn g("XOX.OXO");
    bool result = g.solve();
    assert(result == false);
}

void clobber_1xn_test_sum_5()
{
    clobber_1xn g("XO.XOXOXO");
    bool result = g.solve();
    assert(result == true);
}

void clobber_1xn_test_sum_6()
{
    clobber_1xn g("XOXOXO.XOXOXO");
    bool result = g.solve();
    assert(result == false);
}

void assert_move(move_generator& mg, int from, int to)
{ 
    move m = mg.gen_move();
    assert(from == cgt_move::from(m));
    assert(to == cgt_move::to(m));
}

void clobber_1xn_move_generator_test_1()
{
    clobber_1xn g("XXX.OOO");
    
    move_generator* mgp(g.create_mg());
    move_generator& mg(*mgp);
    
    assert(!mg);
}

void clobber_1xn_move_generator_test_2()
{
    clobber_1xn g("XO");
    
    move_generator* mgp(g.create_mg());
    move_generator& mg(*mgp);
    assert(mg);
    assert_move(mg, 0, 1);
    ++mg;
    assert(!mg);
}

void clobber_1xn_move_generator_test_3()
{
    clobber_1xn g("OXOX");
    
    move_generator* mgp(g.create_mg());
    move_generator& mg(*mgp);
    assert(mg);
    assert_move(mg, 1, 2);
    ++mg;
    assert(mg);
    assert_move(mg, 1, 0);
    ++mg;
    assert(mg);
    assert_move(mg, 3, 2);
    ++mg;
    assert(!mg);
}

void clobber_1xn_test_all()
{
    clobber_1xn_test_zero();
    clobber_1xn_test_zero2();
    clobber_1xn_move_generator_test_1();
    clobber_1xn_move_generator_test_2();
    clobber_1xn_move_generator_test_3();
    clobber_1xn_test_1();
    clobber_1xn_test_2();
    clobber_1xn_test_3();
    clobber_1xn_test_sum_1();
    clobber_1xn_test_sum_2();
    clobber_1xn_test_sum_3();
    clobber_1xn_test_sum_4();
    clobber_1xn_test_sum_5();
    clobber_1xn_test_sum_6();
}