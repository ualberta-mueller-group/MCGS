#include "split_test_clobber_1xn.h"
#include "clobber_1xn.h"
#include "split_test_utils.h"

namespace {
void clobber_1xn_split1()
{
    test_strip<clobber_1xn>(".XO.X.X...X.......O..O.XOXOX....O.O..OOX",
                            {
                                "XO",
                                "XOXOX",
                                "OOX",
                            });
}

void clobber_1xn_split2()
{

    test_strip<clobber_1xn>(".XOO.O.OOO...O..OX.XXX.O......X.X.....X.",
                            {
                                "XOO",
                                "OX",
                            });
}

void clobber_1xn_split3()
{
    test_strip<clobber_1xn>("...OXOX.XOO.OO.XOX....O.X.........XXO...",
                            {
                                "OXOX",
                                "XOO",
                                "XOX",
                                "XXO",
                            });
}

void clobber_1xn_split4()
{
    test_strip<clobber_1xn>("X..OO.OO.X.XOOO.X....X.X....O....OX.X...",
                            {
                                "XOOO",
                                "OX",
                            });
}

void clobber_1xn_split5()
{
    test_strip<clobber_1xn>("X.XO...X..O..O..O.OO......XXO.X..XO.O.X.",
                            {
                                "XO",
                                "XXO",
                                "XO",
                            });
}

void clobber_1xn_split6()
{
    test_strip<clobber_1xn>("..O....X..O.O.X..XXO..X...OXO.X..X.OO.O.",
                            {
                                "XXO",
                                "OXO",
                            });
}

void clobber_1xn_split7()
{
    test_strip<clobber_1xn>("..X.", {});
}

void clobber_1xn_split8()
{
    test_strip<clobber_1xn>("...", {});
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
} // namespace

void split_test_clobber_1xn_all()
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
