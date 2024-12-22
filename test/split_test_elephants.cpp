#include "split_test_elephants.h"
#include "elephants.h"
#include "split_test_utils.h"


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
        "X......X.XO",
        "X..O...O...O",
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



void split_test_elephants_all()
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

