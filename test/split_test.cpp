#include "split_test.h"
#include <iostream>
#include "cgt_dyadic_rational.h"
#include "cgt_integer_game.h"
#include "cgt_move.h"
#include "clobber_1xn.h"
#include "game.h"
#include "nogo_1xn.h"
#include "strip.h"
#include "sumgame.h"
#include "elephants.h"
#include "cgt_switch.h"
#include <algorithm>
#include <memory>

using std::cout, std::endl, std::string, std::unique_ptr;

//////////////////////////////////////// helper functions

void assert_strip_split_result(const strip* g, vector<string> expected, bool non_empty = false)
{
    split_result result = non_empty ? g->split_non_empty() : g->split();

    assert(result);

    vector<string> got;

    for (game* g2 : *result)
    {
        strip* gs = dynamic_cast<strip*>(g2);
        assert(gs != nullptr);

        got.push_back(gs->board_as_string());

        delete g2;
    }

    sort(expected.begin(), expected.end());
    sort(got.begin(), got.end());

    assert(expected.size() == got.size());
    for (int i = 0; i < got.size(); i++)
    {
        assert(expected[i] == got[i]);
    }
}

void assert_no_split(const game* g, bool non_empty = false)
{
    if (non_empty)
        assert(!g->split_non_empty().has_value());
    else
        assert(!g->split().has_value());
}

template <class T>
void test_strip(const string& board, const vector<string>& expected, bool no_split = false, bool non_empty = false)
{
    T pos(board);

    if (no_split)
    {
        assert_no_split(&pos, non_empty);
    } else
    {
        assert_strip_split_result(&pos, expected, non_empty);
    }

}

template <class T>
void test_strip_non_empty(const string& board, const vector<string>& expected, bool no_split = false)
{
    test_strip<T>(board, expected, no_split, true);
}


//////////////////////////////////////// elephants

void elephants_split1()
{
    test_strip<elephants>(".......X.....O..X.....O.......", {
        ".......X.....O",
        "X.....O.......",
    });
}

void elephants_split2()
{
    test_strip<elephants>("....OXO.....X......X.XOX..O...O...O.XXO.", {
        "....O",
        "XO",
        "X......X.XO",
        "X..O...O...O",
        "XXO.",
    });
}

void elephants_split3()
{
    test_strip<elephants>(".OXO...O.O..O.X.X......X...X..OO..X....X", {
        ".O",
        "XO...O.O..O",
        "X.X......X...X..OO",
        "X....X",
    });
}

void elephants_split4()
{
    test_strip<elephants>("X.......O.....XO.O.....X..OO.X.O.OOXO...", {
        "X.......O",
        "XO.O",
        "X..OO",
        "X.O.OO",
        "XO...",
    });
}

void elephants_split5()
{
    test_strip<elephants>("X...XX.O...X.....OOOO..X...O.....O.O...O", {
        "X...XX.O",
        "X.....OOOO",
        "X...O.....O.O...O",
    });
}

void elephants_split6()
{
    test_strip<elephants>("X..X........O....X....O...O.......XO.X..", {
        "X..X........O",
        "X....O...O",
        "XO",
        "X..",
    });
}

void elephants_split7()
{
    test_strip<elephants>("XX...XO.O.X..X.O..O.O.X.O.X..X..X.....O..OOO", {
        "XX...XO.O",
        "X..X.O..O.O",
        "X.O",
        "X..X..X.....O..OOO",
    });
}

void elephants_split8()
{
    elephants pos("X..X.X..X..O...OOO");
    assert_no_split(&pos);
}

void elephants_split9()
{
    elephants pos("X..X.X.X..X...O.O..O");
    assert_no_split(&pos);
}



void test_elephants_split()
{
    elephants_split1();
    elephants_split2();
    elephants_split3();
    elephants_split4();
    elephants_split5();
    elephants_split6();
    elephants_split7();
    elephants_split8();
    elephants_split9();
}

//////////////////////////////////////// clobber_1xn

void clobber_1xn_split1()
{
    test_strip<clobber_1xn>(".XO.X.X...X.......O..O.XOXOX....O.O..OOX", {
        "XO",
        "XOXOX",
        "OOX",
    });
}

void clobber_1xn_split2()
{

    test_strip<clobber_1xn>(".XOO.O.OOO...O..OX.XXX.O......X.X.....X.", {
        "XOO",
        "OX",
    });
}

void clobber_1xn_split3()
{
    test_strip<clobber_1xn>("...OXOX.XOO.OO.XOX....O.X.........XXO...", {
        "OXOX",
        "XOO",
        "XOX",
        "XXO",
    });
}

void clobber_1xn_split4()
{
    test_strip<clobber_1xn>("X..OO.OO.X.XOOO.X....X.X....O....OX.X...", {
        "XOOO",
        "OX",
    });
}

void clobber_1xn_split5()
{
    test_strip<clobber_1xn>("X.XO...X..O..O..O.OO......XXO.X..XO.O.X.", {
        "XO",
        "XXO",
        "XO",
    });
}

void clobber_1xn_split6()
{
    test_strip<clobber_1xn>("..O....X..O.O.X..XXO..X...OXO.X..X.OO.O.", {
        "XXO",
        "OXO",
    });
}

void clobber_1xn_split7()
{
    test_strip<clobber_1xn>("..X.", {
    });


}

void clobber_1xn_split8()
{
    test_strip<clobber_1xn>("...", {
    });

}

void clobber_1xn_split9()
{
    clobber_1xn pos("XOXOXO");
    assert_no_split(&pos);
}

void clobber_1xn_split10()
{
    clobber_1xn pos("OOXXOXOXOX");
    assert_no_split(&pos);
}


void test_clobber_1xn_split()
{
    clobber_1xn_split1();
    clobber_1xn_split2();
    clobber_1xn_split3();
    clobber_1xn_split4();
    clobber_1xn_split5();
    clobber_1xn_split6();
    clobber_1xn_split7();
    clobber_1xn_split8();
    clobber_1xn_split9();
    clobber_1xn_split10();
}

//////////////////////////////////////// dyadic_rational

void dyadic_rational_split1()
{
    dyadic_rational pos(35, 8);

    assert(!pos.split().has_value());

    int to_play = BLACK;
    {
        unique_ptr<move_generator> mg1 = unique_ptr<move_generator>(pos.create_move_generator(to_play));
        assert(*mg1);
        move m1 = mg1->gen_move();
        pos.play(m1, to_play);
    }
    // 34/8 == 17/4
    assert(!pos.split().has_value());
    {
        unique_ptr<move_generator> mg2 = unique_ptr<move_generator>(pos.create_move_generator(to_play));
        assert(*mg2);
        move m2 = mg2->gen_move();
        pos.play(m2, to_play);
    }
    // 16/4 == 4
    split_result sr = pos.split();

    assert(sr.has_value() && sr->size() == 1);

    integer_game *ig = dynamic_cast<integer_game*>(sr->back());
    assert(ig != nullptr);

    assert(ig->value() == 4);

    for (game* g : *sr)
    {
        delete g;
    }


}

void dyadic_rational_split2()
{
    dyadic_rational pos(37, 16);

    assert(!pos.split().has_value());

    int to_play = WHITE;

    {
        unique_ptr<move_generator> mg1 = unique_ptr<move_generator>(pos.create_move_generator(to_play));
        assert(*mg1);
        move m1 = mg1->gen_move();
        pos.play(m1, to_play);
    }

    // 38/16 == 19 / 8
    assert(!pos.split().has_value());
    {
        unique_ptr<move_generator> mg2 = unique_ptr<move_generator>(pos.create_move_generator(to_play));
        assert(*mg2);
        move m2 = mg2->gen_move();
        pos.play(m2, to_play);
    }
    // 20 / 8 == 10 / 4 == 5 / 2
    assert(!pos.split().has_value());
    {
        unique_ptr<move_generator> mg3 = unique_ptr<move_generator>(pos.create_move_generator(to_play));
        assert(*mg3);
        move m3 = mg3->gen_move();
        pos.play(m3, to_play);
    }

    // 6/2 == 3
    split_result sr = pos.split();

    assert(sr.has_value() && sr->size() == 1);

    integer_game *ig = dynamic_cast<integer_game*>(sr->back());
    assert(ig != nullptr);

    assert(ig->value() == 3);

    for (game* g : *sr)
    {
        delete g;
    }
}

void dyadic_rational_split3()
{
    dyadic_rational pos(16, 2);

    split_result sr = pos.split();

    assert(sr.has_value());
    assert(sr->size() == 1);
    integer_game* ig = dynamic_cast<integer_game*>(sr->back());
    assert(ig != nullptr);

    assert(ig->value() == 8);
}

void test_dyadic_rational_split()
{
    dyadic_rational_split1();
    dyadic_rational_split2();
    dyadic_rational_split3();

}


//////////////////////////////////////// integer_game

void integer_game1()
{
    integer_game pos(21);
    assert_no_split(&pos);
}

void integer_game2()
{
    integer_game pos(-5);
    assert_no_split(&pos);
}

void integer_game3()
{
    integer_game pos(0);
    assert_no_split(&pos);

}

void integer_game4()
{
    integer_game pos(6);
    assert_no_split(&pos);

    unique_ptr<move_generator> mg = unique_ptr<move_generator>(pos.create_move_generator(BLACK));

    assert(*mg);
    move m = mg->gen_move();
    pos.play(m, BLACK);

    assert(pos.value() == 5);
    assert_no_split(&pos);
}



void test_integer_game_split()
{
    integer_game1();
    integer_game2();
    integer_game3();
    integer_game4();

}

//////////////////////////////////////// switch

void test_switch_generic(int l, int r)
{
    switch_game pos(l, r);

    // Shouldn't split until move is played
    assert(!pos.split().has_value());

    // Play black move, check split
    {
        unique_ptr<move_generator> mgb(pos.create_move_generator(BLACK));
        assert(mgb);
        move mb = mgb->gen_move();
        pos.play(mb, BLACK);
        assert(pos.is_integer());
        assert(pos.value() == l);

        split_result srb = pos.split();

        assert(srb);
        assert(srb->size() == 1);

        integer_game* igb = dynamic_cast<integer_game*>(srb->back());
        assert(igb != nullptr);
        assert(igb->value() == l);

        for (game* g : *srb)
        {
            delete g;
        }
    }

    // undo move
    pos.undo_move();
    assert(!pos.split().has_value());

    // Play white move, check split
    {
        unique_ptr<move_generator> mgw(pos.create_move_generator(WHITE));
        assert(mgw);
        move mw = mgw->gen_move();
        pos.play(mw, WHITE);
        assert(pos.is_integer());
        assert(pos.value() == r);

        split_result srw = pos.split();

        assert(srw);
        assert(srw->size() == 1);

        integer_game* igw = dynamic_cast<integer_game*>(srw->back());
        assert(igw != nullptr);
        assert(igw->value() == r);

        for (game* g : *srw)
        {
            delete g;
        }
    }

}



void switch_game1()
{
    test_switch_generic(7, 5);

}

void switch_game2()
{
    test_switch_generic(400, -300);

}

void switch_game3()
{
    test_switch_generic(-21, -90);
}



void test_switch_game_split()
{
    switch_game1();
    switch_game2();
    switch_game3();
}



void nogo_1xn_split1()
{
    test_strip_non_empty<nogo_1xn>(".X..OX..", {
        ".X..O",
        "X..",
    });

}

void nogo_1xn_split2()
{
    test_strip_non_empty<nogo_1xn>("..OO.X...OOO...O.XO..OO.....OX.XO.O.", {
        "..OO.X...OOO...O.X",
        "O..OO.....O",
        "O.O.",
    });
}
void nogo_1xn_split3()
{
    test_strip_non_empty<nogo_1xn>("O.....OOX....OO..XXO.XO.O..OOXX....O.O", {
        "O.....OO",
        "X....OO..XX",
        "O.O..OO",
        "XX....O.O",
    });
}
void nogo_1xn_split4()
{
    test_strip_non_empty<nogo_1xn>(".X.OO.XO.O.X.X....XO.OX.O.O.OX..O...OO.", {
        ".X.OO.X",
        "O.O.X.X....X",
        "X.O.O.O",
        "X..O...OO.",
    });
}
void nogo_1xn_split5()
{
    test_strip_non_empty<nogo_1xn>(".OO..O..X..O......X...OX.....X......X.X......X..OO", {
        ".OO..O..X..O......X...O",
        "X.....X......X.X......X..OO",
    });
}
void nogo_1xn_split6()
{
    test_strip_non_empty<nogo_1xn>(".....", {
    }, true);
}
void nogo_1xn_split7()
{
    test_strip_non_empty<nogo_1xn>(".X...", {
    }, true);
}
void nogo_1xn_split8()
{
    test_strip_non_empty<nogo_1xn>("..XO..", {
        "..X",
        "O..",
    });
}




//////////////////////////////////////// nogo_1xn

void test_nogo_1xn_split()
{
    nogo_1xn_split1();
    nogo_1xn_split2();
    nogo_1xn_split3();
    nogo_1xn_split4();
    nogo_1xn_split5();
    nogo_1xn_split6();
    nogo_1xn_split7();
    nogo_1xn_split8();
}



//////////////////////////////////////// run all split tests

void split_test_all()
{
    // TODO do we need to test nim and other games that don't split?
    test_elephants_split();
    test_clobber_1xn_split();
    test_dyadic_rational_split();
    test_integer_game_split();
    test_switch_game_split();
    test_nogo_1xn_split();

}
