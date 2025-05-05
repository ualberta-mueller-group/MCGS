//---------------------------------------------------------------------------
// Unit tests for impartial_sumgame
// Compute nim_value and win/loss minimax result
//---------------------------------------------------------------------------
#include "impartial_sumgame_test.h"
#include "impartial_sumgame.h"
#include "clobber_1xn.h"
#include "kayles.h"
#include "cgt_nimber.h"
#include "test_utilities.h"

namespace {

void test_sum_nim_value(const sumgame& sum, int nim_value)
{
    int search_value = search_sumgame(sum);
    assert(search_value == nim_value);
}

void test_game_sum(std::vector<game*>&& games, int nim_value)
{
    sumgame sum(BLACK);
    for (game* g : games)
    {
        sum.add(g);
    }
    test_sum_nim_value(sum, nim_value);
    
    // Minimax search
    const bool result = nim_value != 0;
    assert_sum_outcomes(result, result, games);

}
} // namespace

void impartial_sumgame_test_all()
{

    test_game_sum({
                      new kayles(1), // nim value 1
                      new kayles(1), // nim value 1
                  },
                  0  // nim sum
    );
    
    test_game_sum({
                      new kayles(2), // nim value 2
                      new kayles(3), // nim value 3
                  },
                  1 // nim sum
    );
    test_game_sum({
                      new kayles(2), // nim value 2
                      new kayles(3),  // nim value 3
                      new kayles(5),  // nim value 4
                  },
                  5 // nim sum
    );
    
    test_game_sum({
                      new kayles(4),  // nim value 1
                      new nimber(7),  // nim value 7
                  },
                  6 // nim sum
    );
    
    test_game_sum({
                      new nimber(2), // nim value 2
                      new kayles(4),  // nim value 1
                      new nimber(7),  // nim value 7
                  },
                  4 // nim sum
    );
    
}
