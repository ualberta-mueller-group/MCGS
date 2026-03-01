#include "split_test_cannibal_clobber.h"
#include "test/split_test_utils.h"
#include <string>
#include <vector>
#include "cannibal_clobber.h"

using namespace std;

namespace {

void test_no_split()
{
    vector<string> boards = {
        "",
        "XO|O.",
    };

    for (const string& board : boards)
    {
        cannibal_clobber c(board);
        assert_no_split(&c);
    }
}

void test_split()
{
    test_grid_split<cannibal_clobber>("XO|..", {
                                          "XO",
                                      });

    test_grid_split<cannibal_clobber>(".XO.", {
                                         "XO",
                                     });

    test_grid_split<cannibal_clobber>("......|.XO...|..OX..|......|.OX...|.X....",
                             {
                                 "XO.|.OX",
                                 "OX|X.",
                             });

    test_grid_split<cannibal_clobber>("O..X..|..OXX.|....XO|......|...O..|....XO",
                             {
                                 ".X..|OXX.|..XO",
                                 "XO",
                             });

    test_grid_split<cannibal_clobber>("O..OO.|.OO...|.O.O..|.X..O.|XX.X.X|O...OX",
                             {
                                 "OO",
                                 ".OO|.O.|.X.|XX.|O..",
                                 ".X|OX",
                             });

    test_grid_split<cannibal_clobber>("XXOXO.|OXXX..|......|.XX.XX|OO...O|O.X.OO",
                             {
                                 "XXOXO|OXXX.",
                                 ".XX|OO.|O..",
                                 "XX|.O|OO",
                             });

    test_grid_split<cannibal_clobber>("..O...|.OOO..|..X...|XX....|O.....|......",
                             {
                                 ".O.|OOO|.X.",
                                 "XX|O.",
                             });

    test_grid_split<cannibal_clobber>("X.O.X.O.X.O.X.O|"
                             ".O.X.O.X.O.X.O.|"
                             "X.O.X.O.X.O.X.O|"
                             ".O.X.O.X.O.X.O.|"
                             "X.O.X.O.X.O.X.O|"
                             ".O.X.O.X.O.X.O.|"
                             "X.O.X.O.X.O.X.O|"
                             ".O.X.O.X.O.X.O.|"
                             "X.O.X.O.X.O.X.O|"
                             ".O.X.O.X.O.X.O.",
                             {});

    test_grid_split<cannibal_clobber>("XOXOXO|......|XXX.OO", {
                                                         "XOXOXO",
                                                         "XXX",
                                                         "OO",
                                                     });

    test_grid_split<cannibal_clobber>("XOXOXOO|.......|X.O.X.O|.O.X.O.|X.O.X.O|.O.X.O.",
                             {
                                 "XOXOXOO",
                             });
}
} // namespace

void split_test_cannibal_clobber_all()
{
    test_no_split();
    test_split();
}
