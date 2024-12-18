#include "split_test_nogo_1xn.h"
#include "nogo_1xn.h"
#include "split_test_utils.h"

/*
    Tests have commented out results for non block-simplified split() results
*/


void nogo_1xn_split1()
{
    test_strip<nogo_1xn>(".X..OX..", {
        ".X..O",
        "X..",
    });

}

void nogo_1xn_split2()
{
    test_strip<nogo_1xn>("..OO.X...OOO...O.XO..OO.....OX.XO.O.", {
//        "..OO.X...OOO...O.X",
//        "O..OO.....O",
//        "O.O.",

        "..O.X...O...O.X",
        "O..O.....O",
        "O.O.",
    });
}
void nogo_1xn_split3()
{
    test_strip<nogo_1xn>("O.....OOX....OO..XXO.XO.O..OOXX....O.O", {
//        "O.....OO",
//        "X....OO..XX",
//        "O.O..OO",
//        "XX....O.O",

        "O.....O",
        "X....O..X",
        "O.O..O",
        "X....O.O",
    });
}
void nogo_1xn_split4()
{
    test_strip<nogo_1xn>(".X.OO.XO.O.X.X....XO.OX.O.O.OX..O...OO.", {
//        ".X.OO.X",
//        "O.O.X.X....X",
//        "X.O.O.O",
//        "X..O...OO.",

        ".X.O.X",
        "O.O.X.X....X",
        "X.O.O.O",
        "X..O...O.",
    });
}
void nogo_1xn_split5()
{
    test_strip<nogo_1xn>(".OO..O..X..O......X...OX.....X......X.X......X..OO", {
//        ".OO..O..X..O......X...O",
//        "X.....X......X.X......X..OO",

        ".O..O..X..O......X...O",
        "X.....X......X.X......X..O",
    });
}
void nogo_1xn_split6()
{
    test_strip<nogo_1xn>(".....", {
    }, true);
}
void nogo_1xn_split7()
{
    test_strip<nogo_1xn>(".X...", {
    }, true);
}
void nogo_1xn_split8()
{
    test_strip<nogo_1xn>("..XO..", {
        "..X",
        "O..",
    });
}

void split_test_nogo_1xn_all()
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

