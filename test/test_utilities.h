//---------------------------------------------------------------------------
// Utility functions for unit tests
//---------------------------------------------------------------------------

#ifndef test_utilities_H
#define test_utilities_H

#include <iostream>
#include <memory>
#include "cgt_move.h"
#include "game.h"

using std::cout;
using std::endl;

inline void assert_equal(int a, int b)
{ 
    if (!(a==b))
        cout << "assert_equal FAIL: " << a << ' ' << b << endl;
    assert(a == b);
}
inline void assert_move(move_generator& mg, int mv)
{ 
    const move m = mg.gen_move();
    assert(m == mv);
}

inline void assert_num_moves(const game& g, bw to_play, int num_moves)
{ 
    std::unique_ptr<move_generator>mgp(g.create_move_generator(to_play));
    move_generator& mg(*mgp);
    for (int i = 0; i < num_moves; ++i)
    {
        assert(mg);
        ++mg;
    }
    assert(!mg);
}

inline void assert_two_part_move(move_generator& mg, int from, int to)
{ 
    move m = mg.gen_move();
    assert(from == cgt_move::from(m));
    assert(to == cgt_move::to(m));
}

inline void assert_solve(game& pos, bw to_play, bool expected_result)
{
    assert_black_white(to_play);
    alternating_move_game g(pos, to_play);
    const bool result = g.solve();
    assert(result == expected_result);
}

#endif // test_utilities_H
