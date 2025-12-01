#include "sheep_test.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <utility>
#include <cassert>
#include <cmath>
#include <tuple>

#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"
#include "sheep.h"
#include "test/test_utilities.h"
#include "utilities.h"

using namespace std;

////////////////////////////////////////////////// helpers/tests
namespace {

/*
   Generate all moves for a given player. Save the moves, and extract start/end
   location pairs from them
*/
void get_move_generator_moves_impl(sheep& g, bw to_play,
                                   unordered_set<::move>& moves,
                                   unordered_set<pair<int, int>>& pairs)
{
    assert(is_black_white(to_play));

    unique_ptr<move_generator> gen(g.create_move_generator(to_play));

    while (*gen)
    {
        ::move m = gen->gen_move();
        ++(*gen);

        int new_herd, from, to;
        cgt_move::move3_unpack(m, new_herd, from, to);

        moves.insert(m);
        pairs.insert({from, to});
    }
}

void get_move_generator_moves(sheep& g, unordered_set<::move>& moves_black,
                              unordered_set<::move>& moves_white,
                              unordered_set<pair<int, int>>& pairs)
{

    get_move_generator_moves_impl(g, BLACK, moves_black, pairs);
    get_move_generator_moves_impl(g, WHITE, moves_white, pairs);
}

/*
   Given the expected start/end location pairs:
   1. Compute all moves
   2. Play the moves and check that they did the right thing
   3. Check that the set of computed moves is the same as the set from the
      move_generator
*/
void test_moves_play(sheep& g,
                     const unordered_set<::move>& generator_moves_black,
                     const unordered_set<::move>& generator_moves_white,
                     const unordered_set<pair<int, int>>& pairs)
{
    // Save computed moves and compare them to the generated ones
    unordered_set<::move> computed_moves_black;
    unordered_set<::move> computed_moves_white;

    for (const pair<int, int>& p : pairs)
    {
        const int from_idx = p.first;
        const int to_idx = p.second;

        const int from_herd = g.at(from_idx);
        const int to_herd = g.at(to_idx);
        assert(to_herd == 0 &&    //
               abs(from_herd) > 1 //
        );

        const bw player = from_herd > 0 ? BLACK : WHITE;
        const int player_step = (player == BLACK) ? 1 : -1;

        for (int target_size = player_step; target_size != from_herd;
             target_size += player_step)
        {
            assert_restore_game arg(g);

            // Compute the move. Check that it was generated
            ::move m = cgt_move::move3_create(target_size, from_idx, to_idx);

            if (player == BLACK)
            {
                computed_moves_black.insert(m);
                assert(generator_moves_black.find(m) !=
                       generator_moves_black.end());
            }
            else
            {
                computed_moves_white.insert(m);
                assert(generator_moves_white.find(m) !=
                       generator_moves_white.end());
            }

            // Play the move and check that it correctly (and only) modified
            // start/end locations
            vector<int> pre_move_board = g.board();

            const int expected_from = from_herd - target_size;
            const int expected_to = target_size;

            assert(logical_iff(expected_from > 0, from_herd > 0));
            assert(logical_iff(expected_to > 0, player == BLACK));

            g.play(m, player);
            assert(g.at(from_idx) == expected_from && //
                   g.at(to_idx) == expected_to        //
            );

            vector<int> post_move_board = g.board();
            assert(pre_move_board.size() == post_move_board.size());

            const int N = g.size();
            for (int i = 0; i < N; i++)
            {
                if (i == from_idx)
                {
                    assert(post_move_board[i] == expected_from);
                    continue;
                }

                if (i == to_idx)
                {
                    assert(post_move_board[i] == expected_to);
                    continue;
                }

                assert(pre_move_board[i] == post_move_board[i]);
            }

            g.undo_move();
        }
    }

    assert(generator_moves_black == computed_moves_black);
    assert(generator_moves_white == computed_moves_white);
}

void test_moves_main()
{
    // Board string, and from/to point pairs for either player
    typedef tuple<string, unordered_set<pair<int, int>>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            "0 0 0 | 0 3 0 | 0 0 0",
            {
                {4, 1},
                {4, 2},
                {4, 3},
                {4, 5},
                {4, 6},
                {4, 7},
            }
        },

        {
            "0 0 0 | 0 -3 0 | 0 0 0",
            {
                {4, 1},
                {4, 2},
                {4, 3},
                {4, 5},
                {4, 6},
                {4, 7},
            }
        },

        {
            "0 0 0 | 0 0 0 | 0 0 0",
            {
            }
        },

        {
            "0 0 0 | 0 1 0 | 0 0 0",
            {
            }
        },

        {
            "0 0 0 | 0 -1 0 | 0 0 0",
            {
            }
        },

        {
            "2 0 0 | 0 0 0 | 0 0 -2",
            {
                {0, 2},
                {0, 6},
                {8, 6},
                {8, 2},
            }
        },

        {
            "0 0 0 0 0 | 0 0 0 0 0 | 0 0 3 0 0 | 0 0 0 0 0 | 0 0 0 0 0",

            {
                {12, 2},
                {12, 4},
                {12, 14},
                {12, 22},
                {12, 20},
                {12, 10},
            }
        },

        {
            "0 0 0 0 0 | 0 0 0 0 0 | 0 0 -3 0 0 | 0 0 0 0 0 | 0 0 0 0 0",

            {
                {12, 2},
                {12, 4},
                {12, 14},
                {12, 22},
                {12, 20},
                {12, 10},
            }
        },

        {
            "1 1 0 1 0 | 1 1 0 0 1 | 0 0 -3 0 0 | 1 0 0 1 1 | 0 1 0 1 1",

            {
                {12, 2},
                {12, 4},
                {12, 14},
                {12, 22},
                {12, 20},
                {12, 10},
            }
        },

        {
            "0 0 1 0 1 | 0 0 0 0 0 | 1 0 3 0 1 | 0 0 0 0 0 | 1 0 1 0 0",

            {
                {12, 7},
                {12, 8},
                {12, 13},
                {12, 17},
                {12, 16},
                {12, 11},
            }
        },

        {
            "0 0 -3 0 0 0 0 | 0 0 0 0 0 1 0 | 0 0 0 1 1 0 0 | 0 -7 0 3 1 0 0 |"
            "0 0 1 1 0 0 0 | 0 1 0 0 1 2 0 | 0 0 0 1 9 3 0",

            {
                {2, 0},
                {2, 14},
                {2, 23},
                {2, 6},
                {22, 28},
                {22, 21},
                {22, 1},
                {22, 4},
                {22, 23},
                {22, 29},
                {24, 23},
                {47, 48},
                {47, 41},
                {40, 41},
                {40, 34},
                {40, 19},
            }
        },

    };
    // clang-format on

    for (test_case_t& test_case : test_cases)
    {
        const string& board_string = get<0>(test_case);
        unordered_set<pair<int, int>>& expected_pairs = get<1>(test_case);

        sheep g(board_string);

        // Get move_generator moves and extract location pairs from them
        unordered_set<::move> generator_moves_black;
        unordered_set<::move> generator_moves_white;
        unordered_set<pair<int, int>> generator_pairs;

        get_move_generator_moves(g, generator_moves_black,
                                 generator_moves_white, generator_pairs);

        // Correct start/end pairs?
        assert(expected_pairs == generator_pairs);

        // Given pairs, check if correct moves were generated. Also check that
        // playing the moves does the correct thing
        test_moves_play(g, generator_moves_black, generator_moves_white,
                        generator_pairs);
    }
}

void test_constructors()
{
    // board string, shape, board contents
    typedef tuple<string, int_pair, vector<int>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            "0,0 | -5,-2",
            {2, 2},
            {0, 0, -5, -2},
        },

        {
            "5",
            {1, 1},
            {5},
        },

        {
            "-1,-5",
            {1, 2},
            {-1,-5},
        },

        {
            "-1 -5,0|-4,2 4 ",
            {2, 3},
            {-1, -5, 0, -4, 2, 4},
        },

    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& board_string = get<0>(test_case);
        const int_pair& exp_shape = get<1>(test_case);
        const vector<int>& exp_board = get<2>(test_case);

        sheep g(board_string);
        assert(g.shape() == exp_shape);
        assert(g.board_const() == exp_board);

        sheep g2(exp_board, exp_shape);

        assert(g2.board_const() == exp_board && //
               g2.shape() == exp_shape          //
        );
    }

    // Check 0 size games
    sheep g1("");
    sheep g2(" ");
    sheep g3(vector<int>(), {0, 0});

    assert(g1.size() == 0 &&  //
           g2.size() == 0 &&  //
           g3.size() == 0     //
    );

    assert_num_moves(g1, BLACK, 0);
    assert_num_moves(g1, WHITE, 0);

    assert_num_moves(g2, BLACK, 0);
    assert_num_moves(g2, WHITE, 0);

    assert_num_moves(g3, BLACK, 0);
    assert_num_moves(g3, WHITE, 0);

    // Check string format errors
    ASSERT_DID_THROW(sheep g4("5|"));
    ASSERT_DID_THROW(sheep g4("5,3|2"));
    ASSERT_DID_THROW(sheep g4("5,x|2,5"));

    // Out of bounds numbers
    const std::string HERD_OOB_POS = to_string(sheep::MAX_HERD + 1);
    const std::string HERD_OOB_NEG = to_string(-sheep::MAX_HERD - 1);
    ASSERT_DID_THROW(sheep g4(HERD_OOB_POS));
    ASSERT_DID_THROW(sheep g4(HERD_OOB_NEG));

    const std::string HERD_MAX_POS = to_string(sheep::MAX_HERD);
    const std::string HERD_MAX_NEG = to_string(-sheep::MAX_HERD);
    {
        sheep g1(HERD_MAX_POS);
        sheep g2(HERD_MAX_NEG);

        assert(g1.size() == 1 && g1.at(0) == sheep::MAX_HERD);
        assert(g2.size() == 1 && g2.at(0) == -sheep::MAX_HERD);
    }
}

} // namespace

//////////////////////////////////////////////////
void sheep_test_all()
{
    test_moves_main();
    test_constructors();
}
