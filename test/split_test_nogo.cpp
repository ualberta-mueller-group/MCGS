#include "split_test_nogo.h"
#include "nogo.h"
#include "split_test_utils.h"


void split_test_nogo_all()
{
    test_grid_split<nogo>("OX.O.|.OO.X|..XX.|.XO.X",
                            {
                                "O.|#X",
                                "O##|.O#|..X|.XO",
                                "#O#|O.X|#X#",
                                "#X|X.|#X"
                            });
}
