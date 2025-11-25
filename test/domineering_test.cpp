#include "domineering_test.h"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <tuple>
#include <cassert>

#include "cgt_basics.h"
#include "cgt_move_new.h"
#include "domineering.h"
#include "grid_location.h"
#include "test/test_utilities.h"

using namespace std;


namespace {



////////////////////////////////////////////////// test logic helpers
inline grid_dir get_player_dir(bw player)
{
    assert(is_black_white(player));
    return player == BLACK ? GRID_DIR_DOWN : GRID_DIR_RIGHT;
}

int_pair get_second_coord_for_player(const domineering* g,
                                     const int_pair& coord, bw player)
{
    assert(g != nullptr &&        //
           is_black_white(player) //
    );

    const grid_dir dir = get_player_dir(player);

    int_pair coord2;
    assert(grid_location::get_neighbor_coord(coord2, coord, dir, g->shape()));

    return coord2;
}

::move coord_to_move(const domineering* g, const int_pair& coord1, bw player)
{
    assert(g != nullptr &&        //
           is_black_white(player) //
    );

    const int_pair coord2 = get_second_coord_for_player(g, coord1, player);

    return cgt_move_new::move4_create_from_coords(coord1, coord2);
}

void compare_generated_moves(const domineering* g,
                             const vector<int_pair>& exp_coords, bw player)
{
    assert(g != nullptr &&        //
           is_black_white(player) //
    );

    const unordered_set<::move> gen_moves =
        get_generated_moves_for_player(g, player);

    unordered_set<::move> exp_moves;

    for (const int_pair& coord : exp_coords)
    {
        const ::move m = coord_to_move(g, coord, player);

        auto inserted = exp_moves.insert(m);
        assert(inserted.second);
    }

    assert(gen_moves == exp_moves);
}

void test_play_undo_for_player(domineering* g,
                               const vector<int_pair>& exp_coords, bw player)
{
    assert(g != nullptr &&        //
           is_black_white(player) //
    );

    for (const int_pair& coord1 : exp_coords)
    {
        assert_restore_game arg(*g);

        const ::move m = coord_to_move(g, coord1, player);

        const int_pair coord2 = get_second_coord_for_player(g, coord1, player);
        const int point1 = grid_location::coord_to_point(coord1, g->shape());
        const int point2 = grid_location::coord_to_point(coord2, g->shape());
        assert(point1 != point2);

        const int_pair shape_before = g->shape();
        const vector<int> board_before = g->board_const();

        g->play(m, player);

        const int_pair shape_after = g->shape();
        const vector<int> board_after = g->board_const();

        g->undo_move();
        assert(g->shape() == shape_before);
        assert(g->board_const() == board_before);

        assert(board_before.size() == board_after.size());
        assert(shape_before == shape_after);

        assert(board_before[point1] == EMPTY && //
               board_after[point1] == BORDER    //
        );

        assert(board_before[point2] == EMPTY && //
               board_after[point2] == BORDER    //
        );

        // Check that only point1 and point2 are different
        vector<int> board_after_reverted = board_after;
        board_after_reverted[point1] = EMPTY;
        board_after_reverted[point2] = EMPTY;
        assert(board_before == board_after_reverted);
    }
}

////////////////////////////////////////////////// main test functions
void test_moves_main()
{
    /*
       domineering board string
       list of moves for black (1st coordinate of the move)
       list of moves for white (1st coordinate of the move)

       (here 1st coordinate) is top-leftmost coordinate of the move
    */
    typedef tuple<string, vector<int_pair>, vector<int_pair>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            "",
            {
            },
            {
            },
        },

        {
            ".",
            {
            },
            {
            },
        },

        {
            "..",
            {
            },
            {
                {0, 0},
            },
        },

        {
            ".|.",
            {
                {0, 0},
            },
            {
            },
        },

        {
            "..|..",
            {
                {0, 0},
                {0, 1},
            },
            {
                {0, 0},
                {1, 0},
            },
        },

        {
            ".#.|...",
            {
                {0, 0},
                {0, 2},
            },
            {
                {1, 0},
                {1, 1},
            },
        },

        {
            "..|.#|..",
            {
                {0, 0},
                {1, 0},
            },
            {
                {0, 0},
                {2, 0},
            },
        },

        {
            "#...|.#..|..#.|...#",
            {
                {0, 2},
                {0, 3},
                {1, 0},
                {1, 3},
                {2, 0},
                {2, 1},
            },
            {
                {0, 1},
                {0, 2},
                {1, 2},
                {2, 0},
                {3, 0},
                {3, 1},
            },
        },

        {
            "#...#|..#..|.###.|..#..|.#.#.",
            {
                {0, 1},
                {0, 3},
                {1, 0},
                {1, 4},
                {2, 0},
                {2, 4},
                {3, 0},
                {3, 4},
            },
            {
                {0, 1},
                {0, 2},
                {1, 0},
                {1, 3},
                {3, 0},
                {3, 3},
            },
        },

        {
            "...|.#.|...",
            {
                {0, 0},
                {0, 2},
                {1, 0},
                {1, 2},
            },
            {
                {0, 0},
                {0, 1},
                {2, 0},
                {2, 1},
            },
        },
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& board = get<0>(test_case);
        const vector<int_pair>& exp_b_coords = get<1>(test_case);
        const vector<int_pair>& exp_w_coords = get<2>(test_case);

        domineering g(board);

        // Check that correct moves were generated
        compare_generated_moves(&g, exp_b_coords, BLACK);
        compare_generated_moves(&g, exp_w_coords, WHITE);

        test_play_undo_for_player(&g, exp_b_coords, BLACK);
        test_play_undo_for_player(&g, exp_w_coords, WHITE);
    }
}

void test_constructors()
{
    /*
       domineering board string
       grid dimensions
       expected board
    */
    typedef tuple<string, int_pair, vector<int>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            "",
            {1, 0},
            {},
        },

        {
            ".",
            {1, 1},
            {
                EMPTY,
            },
        },

        {
            "..",
            {1, 2},
            {
                EMPTY, EMPTY,
            },
        },

        {
            ".|.",
            {2, 1},
            {
                EMPTY,
                EMPTY,
            },
        },

        {
            "..|..",
            {2, 2},
            {
                EMPTY, EMPTY,
                EMPTY, EMPTY,
            },
        },

        {
            ".#.|...",
            {2, 3},
            {
                EMPTY, BORDER, EMPTY,
                EMPTY, EMPTY, EMPTY,
            },
        },

        {
            "..|.#|..",
            {3, 2},
            {
                EMPTY, EMPTY,
                EMPTY, BORDER,
                EMPTY, EMPTY,
            },
        },

        {
            "#...|.#..|..#.|...#",
            {4, 4},
            {
                BORDER, EMPTY, EMPTY, EMPTY,
                EMPTY, BORDER, EMPTY, EMPTY,
                EMPTY, EMPTY, BORDER, EMPTY,
                EMPTY, EMPTY, EMPTY, BORDER,
            },
        },

        {
            "#...#|..#..|.###.|..#..|.#.#.",
            {5, 5},
            {
                BORDER, EMPTY, EMPTY, EMPTY, BORDER,
                EMPTY, EMPTY, BORDER, EMPTY, EMPTY,
                EMPTY, BORDER, BORDER, BORDER, EMPTY,
                EMPTY, EMPTY, BORDER, EMPTY, EMPTY,
                EMPTY, BORDER, EMPTY, BORDER, EMPTY,
            },
        },

        {
            "...|.#.|...",
            {3, 3},
            {
                EMPTY, EMPTY, EMPTY,
                EMPTY, BORDER, EMPTY,
                EMPTY, EMPTY, EMPTY,
            },
        },
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& input_board = get<0>(test_case);
        const int_pair& exp_shape = get<1>(test_case);
        const vector<int>& exp_board = get<2>(test_case);

        domineering g(input_board);
        assert(g.shape() == exp_shape);
        assert(g.board_const() == exp_board);
    }

    // test 0 size games
    {
        domineering g("");
        assert_num_moves(g, BLACK, 0);
        assert_num_moves(g, WHITE, 0);
    }

    // check invalid strings
    ASSERT_DID_THROW(domineering g("X"));
    ASSERT_DID_THROW(domineering g("O"));
    ASSERT_DID_THROW(domineering g(".X"));
    ASSERT_DID_THROW(domineering g(".O"));
}

} // namespace

void domineering_test_all()
{
    cout << "TODO: " << __FILE__ << endl;

    test_constructors();
    test_moves_main();
}
