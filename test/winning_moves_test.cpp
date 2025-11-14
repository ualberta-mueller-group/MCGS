#include "winning_moves_test.h"

#include <algorithm>
#include <tuple>
#include <cassert>
#include <vector>

#include "cgt_move.h"
#include "winning_moves.h"
#include "game.h"

#include "nogo_1xn.h"
#include "clobber_1xn.h"

namespace {

bool same_move_vecs(std::vector<move>& vec1, std::vector<move>& vec2)
{
    if (vec1.size() != vec2.size())
        return false;

    std::sort(vec1.begin(), vec1.end());
    std::sort(vec2.begin(), vec2.end());

    return vec1 == vec2;
}

} // namespace

void test_winning_moves()
{
    /*
       game (pointer)
       player to play
       expected winning moves
    */
    typedef std::tuple<game*, bw, std::vector<move>> test_case_t;

    // clang-format off
    std::vector<test_case_t> test_cases =
    {
        {
            new nogo_1xn(".........."),
            BLACK,
            {1, 3, 6, 8},
        },

        {
            new clobber_1xn("XOXOXOXOXO"),
            WHITE,
            {
                cgt_move::two_part_move(3, 2),
                cgt_move::two_part_move(7, 8),
            },
        }

    };
    // clang-format on

    for (test_case_t& test_case : test_cases)
    {

        game* g = std::get<0>(test_case);
        bw player = std::get<1>(test_case);
        std::vector<move>& expected = std::get<2>(test_case);

        std::vector<move> got = get_winning_moves(g, player);
        assert(same_move_vecs(expected, got));

        delete g;
    }
}
