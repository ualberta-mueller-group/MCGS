#include "fission_test.h"
#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"
#include "fission.h"
#include "grid.h"
#include "grid_location.h"
#include "test/test_utilities.h"
#include <iostream>
#include <set>

using namespace std;

namespace {

////////////////////////////////////////////////// utilities
// TODO these are probably general enough to move into test utilities

inline bool location_compatible(const grid_location& loc, const grid* g)
{
    return loc.valid() &&                 //
           loc.get_shape() == g->shape(); //
}

bool relative_grid_location_is_value(const grid* g, const grid_location& loc,
                                     grid_dir dir, int query_value)
{
    assert(location_compatible(loc, g));

    int point_relative;
    if (!loc.get_neighbor_point(point_relative, dir))
        return false;

    return g->at(point_relative) == query_value;
}

////////////////////////////////////////////////// fission utilities
/*
    Move coordinates and player able to make the move. EMPTY means both can play
   it
*/
typedef tuple<int, int, bw> move_tuple_t;

// target is the X being "split"
inline ::move encode_fission_move(const int_pair& target_coord)
{
    return cgt_move::two_part_move(target_coord.first, target_coord.second);
}

inline int_pair decode_fission_move(const ::move& m)
{
    return int_pair(cgt_move::first(m), cgt_move::second(m));
}

// directions where new Xs should be placed
void get_player_dirs(bw player, grid_dir& dir1, grid_dir& dir2)
{
    assert(is_black_white(player));
    dir1 = player == BLACK ? GRID_DIR_UP : GRID_DIR_LEFT;
    dir2 = player == BLACK ? GRID_DIR_DOWN : GRID_DIR_RIGHT;
}

////////////////////////////////////////////////// test logic helpers
optional<::move> get_move_at(const fission* g, const grid_location& loc, bw player)
{
    assert(location_compatible(loc, g) && is_black_white(player));

    const int point = loc.get_point();
    const int tile = g->at(point);
    if (tile != BLACK)
        return {};

    grid_dir dir1;
    grid_dir dir2;
    get_player_dirs(player, dir1, dir2);

    if (relative_grid_location_is_value(g, loc, dir1, EMPTY) && //
        relative_grid_location_is_value(g, loc, dir2, EMPTY)  //
    )
        return encode_fission_move(loc.get_coord());

    return {};
}

void get_expected_moves(const fission* g, unordered_set<::move>& exp_b_moves,
                        unordered_set<::move>& exp_w_moves,
                        set<move_tuple_t>& found_coords)
{
    assert(g != nullptr &&        //
           exp_b_moves.empty() && //
           exp_w_moves.empty() && //
           found_coords.empty()   //
    );

    for (grid_location loc(g->shape()); loc.valid(); loc.increment_position())
    {
        optional<::move> black_move = get_move_at(g, loc, BLACK);
        optional<::move> white_move = get_move_at(g, loc, WHITE);

        const bool b_val = black_move.has_value();
        const bool w_val = white_move.has_value();
        if (b_val || w_val)
        {
            bw player = EMPTY;
            if (!(b_val && w_val))
                player = b_val ? BLACK : WHITE;

            const int_pair& coord = loc.get_coord();
            move_tuple_t mt(coord.first, coord.second, player);

            auto inserted = found_coords.insert(mt);
            assert(inserted.second);
        }

        if (black_move.has_value())
        {
            auto inserted = exp_b_moves.insert(black_move.value());
            assert(inserted.second);
        }

        if (white_move.has_value())
        {
            auto inserted = exp_w_moves.insert(white_move.value());
            assert(inserted.second);
        }
    }
}

void test_play_move_for(fission* g, const ::move& m, bw player)
{
    assert(is_black_white(player));
    assert_restore_game arg(*g);

    const int_pair pre_shape = g->shape();
    const vector<int> pre_board = g->board_const();

    g->play(m, player);

    const int_pair post_shape = g->shape();
    const vector<int> post_board = g->board_const();

    g->undo_move();

    assert(pre_shape == post_shape);
    assert(g->board_const() == pre_board);

    const int_pair target_coord = decode_fission_move(m);
    const grid_location loc_target(g->shape(), target_coord);
    const int point_target = loc_target.get_point();

    grid_dir dir1;
    grid_dir dir2;
    get_player_dirs(player, dir1, dir2);

    int point1;
    int point2;
    assert(loc_target.get_neighbor_point(point1, dir1) && //
           loc_target.get_neighbor_point(point2, dir2)    //
    );

    for (grid_location loc(pre_shape); loc.valid(); loc.increment_position())
    {
        const int point = loc.get_point();

        const int pre_tile = pre_board[point];
        const int post_tile = post_board[point];

        if (point == point_target)
        {
            assert(pre_tile == BLACK && //
                   post_tile == EMPTY   //
            );
            continue;
        }

        if (point == point1 || point == point2)
        {
            assert(pre_tile == EMPTY && //
                   post_tile == BLACK   //
            );
            continue;
        }

        assert(pre_tile == post_tile);
    }

}

void test_play_moves(fission* g, const unordered_set<::move>& b_moves,
                     const unordered_set<::move>& w_moves)
{
    assert_restore_game arg(*g);

    for (const ::move& m : b_moves)
        test_play_move_for(g, m, BLACK);

    for (const ::move& m : w_moves)
        test_play_move_for(g, m, WHITE);
}

////////////////////////////////////////////////// main test functions
void test_moves_main()
{
    typedef tuple<string, set<move_tuple_t>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            "...|...|...",
            {
            },
        },

        {
            "...|.X.|...",
            {
                {1, 1, EMPTY},
            },
        },

        {
            "...|.#.|...",
            {
            },
        },

        {
            "X.X|.X.|X.X",
            {
                {1, 1, EMPTY},
            },
        },

        {
            ".X..|....|..X.",
            {
                {0, 1, WHITE},
                {2, 2, WHITE},
            },
        },

        {
            ".X.X.|X.X.X|.X.X.|X.X.X|.X.X.",
            {
                {0, 1, WHITE},
                {0, 3, WHITE},
                {1, 0, BLACK},
                {1, 2, EMPTY},
                {1, 4, BLACK},
                {2, 1, EMPTY},
                {2, 3, EMPTY},
                {3, 0, BLACK},
                {3, 2, EMPTY},
                {3, 4, BLACK},
                {4, 1, WHITE},
                {4, 3, WHITE},
            },
        },

        {
            "X.X.X|.X.X.|X.X.X|.X.X.|X.X.X",
            {
                {0, 2, WHITE},
                {1, 1, EMPTY},
                {1, 3, EMPTY},
                {2, 0, BLACK},
                {2, 2, EMPTY},
                {2, 4, BLACK},
                {3, 1, EMPTY},
                {3, 3, EMPTY},
                {4, 2, WHITE},
            },
        },

        {
            ".......|..X.X..|.X#.#X.|...X...|.X#.#X.|..X.X..|.......",
            {
                {1, 2, WHITE},
                {1, 4, WHITE},
                {2, 1, BLACK},
                {2, 5, BLACK},
                {3, 3, EMPTY},
                {4, 1, BLACK},
                {4, 5, BLACK},
                {5, 2, WHITE},
                {5, 4, WHITE},
            },
        },

        {
            "#.#X#.#|.X.#.X.|#.#.#.#|##.X.#.|#.#.#.#|.X.#.X.|#.#X#.#",
            {
                {1, 1, EMPTY},
                {1, 5, EMPTY},
                {3, 3, EMPTY},
                {5, 1, EMPTY},
                {5, 5, EMPTY},
            },
        },

    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& board = get<0>(test_case);
        const set<move_tuple_t>& exp_coords = get<1>(test_case);

        fission g(board);

        // Get generator moves
        unordered_set<::move> b_moves =
            get_generated_moves_for_player(&g, BLACK);

        unordered_set<::move> w_moves =
            get_generated_moves_for_player(&g, WHITE);

        // Get expected moves, compare
        {
            unordered_set<::move> exp_b_moves;
            unordered_set<::move> exp_w_moves;
            set<move_tuple_t> found_coords;
            get_expected_moves(&g, exp_b_moves, exp_w_moves, found_coords);

            assert(b_moves == exp_b_moves);
            assert(w_moves == exp_w_moves);
            assert(found_coords == exp_coords);
        }

        // Test playing moves
        test_play_moves(&g, b_moves, w_moves);
    }
}

void test_constructors()
{
    typedef tuple<string, int_pair, vector<int>> test_case_t;

    // test string constructors

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            "..|X#",
            {2, 2},
            {
                EMPTY, EMPTY, 
                BLACK, BORDER,
            },
        },

        {
            "X|.",
            {2, 1},
            {
                BLACK,
                EMPTY,
            },
        },

        {
            "X.#|.#.|##.",
            {3, 3},
            {
                BLACK, EMPTY, BORDER,
                EMPTY, BORDER, EMPTY,
                BORDER, BORDER, EMPTY,
            },
        },

    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& board_string = get<0>(test_case);
        const int_pair& exp_shape = get<1>(test_case);
        const vector<int>& exp_board = get<2>(test_case);

        fission g(board_string);
        assert(g.shape() == exp_shape);
        assert(g.board_const() == exp_board);
    }

    // test 0 size games
    {
        fission g1("");
        assert(grid_location::shape_is_empty(g1.shape()));
        assert_num_moves(g1, BLACK, 0);
        assert_num_moves(g1, WHITE, 0);
    }

    // check invalid strings
    ASSERT_DID_THROW(fission g("..|O."));
    ASSERT_DID_THROW(fission g("O"));
}

} // namespace

void fission_test_all()
{
    test_constructors();
    test_moves_main();
}
