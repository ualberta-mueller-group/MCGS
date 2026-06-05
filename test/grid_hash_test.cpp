#include "grid_hash_test.h"

#include <type_traits>
#include <vector>
#include <cassert>
#include <tuple>
#include <string>

#include "cgt_basics.h"
#include "elephants.h"
#include "game.h"
#include "gen_king_dirt.h"
#include "grid.h"
#include "grid_hash.h"
#include "grid_hash_orientation.h"
#include "grid_location.h"
#include "hashing.h"
#include "grid_hash_test_utilities.h"
#include "impartial_game_wrapper.h"
#include "nogo_1xn.h"
#include "toppling_dominoes.h"
#include "utilities.h"
#include "all_game_headers.h"
#include "test_utilities.h"

using namespace std;

namespace {

// These tests make this assumption
static_assert(in_interval((int) GRID_HASH_ORIENTATION_0, 0, 7));
static_assert(in_interval((int) GRID_HASH_ORIENTATION_0T, 0, 7));
static_assert(in_interval((int) GRID_HASH_ORIENTATION_90, 0, 7));
static_assert(in_interval((int) GRID_HASH_ORIENTATION_90T, 0, 7));
static_assert(in_interval((int) GRID_HASH_ORIENTATION_180, 0, 7));
static_assert(in_interval((int) GRID_HASH_ORIENTATION_180T, 0, 7));
static_assert(in_interval((int) GRID_HASH_ORIENTATION_270, 0, 7));
static_assert(in_interval((int) GRID_HASH_ORIENTATION_270T, 0, 7));

////////////////////////////////////////////////// Helpers
inline impartial_game_wrapper* make_impartial(game* g)
{
    assert(g != nullptr && !g->is_impartial());
    return new impartial_game_wrapper(g, true);
}

set<::move> make_db_encoded_moves_for(const game& g, bw player)
{
    assert(is_black_white(player));
    set<::move> moves_enc;

    unique_ptr<move_generator> gen(g.create_move_generator(player));
    while (*gen)
    {
        const ::move m = gen->gen_move();
        ++(*gen);

        const ::move m_enc = g.encode_grid_move_to_db(m);
        assert(m == g.decode_grid_move_from_db(m_enc));

        moves_enc.insert(m_enc);
    }

    return moves_enc;
}

template <class Grid_T>
vector<game*> create_grid_games_all_orientations(const string& board_as_string, grid_type_enum grid_type)
{
    static_assert(std::is_base_of_v<grid, Grid_T>);

    const vector<grid_pair_t> grids = string_to_all_grid_orientations(board_as_string, grid_type);

    vector<game*> games;
    games.reserve(2 * grids.size());

    for (const grid_pair_t& gp : grids)
    {
        const vector<int>& board = gp.first;
        const int_pair& shape = gp.second;

        game* g = new Grid_T(board, shape);
        games.emplace_back(g);

        if (!g->is_impartial())
        {
            game* g_imp = make_impartial(new Grid_T(board, shape));
            games.emplace_back(g_imp);
        }
    }

    return games;
}

template <class Grid_T>
vector<game*> create_grid_games_all_orientations(const vector<int>& params, const string& board_as_string, grid_type_enum grid_type)
{
    static_assert(std::is_base_of_v<grid, Grid_T>);

    const vector<grid_pair_t> grids = string_to_all_grid_orientations(board_as_string, grid_type);

    vector<game*> games;
    games.reserve(2 * grids.size());

    for (const grid_pair_t& gp : grids)
    {
        const vector<int>& board = gp.first;
        const int_pair& shape = gp.second;

        game* g = new Grid_T(params, board, shape);
        games.emplace_back(g);

        if (!g->is_impartial())
        {
            game* g_imp = make_impartial(new Grid_T(params, board, shape));
            games.emplace_back(g_imp);
        }
    }

    return games;
}

/*
   Given a grid and grid_hash mask, generate the 7 other grids (using rotation
   and transpose operations). Then check that the original grid and generated
   grids marked by the mask are all assigned the same hash by the grid_hash
   class
*/
void test_grid_hash_impl(unsigned int mask, const grid_pair_t& grid0)
{
    // 0 and 0t
    grid_pair_t grid0t = transpose_grid_pair(grid0);

    // 90 and 90t
    grid_pair_t grid90 = rotate_grid_pair(grid0);
    grid_pair_t grid90t = transpose_grid_pair(grid90);

    // 180 and 180t
    grid_pair_t grid180 = rotate_grid_pair(grid90);
    grid_pair_t grid180t = transpose_grid_pair(grid180);

    // 270 and 270t
    grid_pair_t grid270 = rotate_grid_pair(grid180);
    grid_pair_t grid270t = transpose_grid_pair(grid270);

    grid_hash gh(mask);

    const hash_t hash0 = get_grid_pair_hash(grid0, gh);
    const hash_t hash90 = get_grid_pair_hash(grid90, gh);
    const hash_t hash180 = get_grid_pair_hash(grid180, gh);
    const hash_t hash270 = get_grid_pair_hash(grid270, gh);

    const hash_t hash0t = get_grid_pair_hash(grid0t, gh);
    const hash_t hash90t = get_grid_pair_hash(grid90t, gh);
    const hash_t hash180t = get_grid_pair_hash(grid180t, gh);
    const hash_t hash270t = get_grid_pair_hash(grid270t, gh);

    vector<hash_t> hash_vec(8, 0);

    auto add_to_hash_vec = [&](hash_t hash, int idx) -> void
    {
        assert(hash_vec[idx] == 0);
        hash_vec[idx] = hash;
    };

    add_to_hash_vec(hash0, GRID_HASH_ORIENTATION_0);
    add_to_hash_vec(hash90, GRID_HASH_ORIENTATION_90);
    add_to_hash_vec(hash180, GRID_HASH_ORIENTATION_180);
    add_to_hash_vec(hash270, GRID_HASH_ORIENTATION_270);
    add_to_hash_vec(hash0t, GRID_HASH_ORIENTATION_0T);
    add_to_hash_vec(hash90t, GRID_HASH_ORIENTATION_90T);
    add_to_hash_vec(hash180t, GRID_HASH_ORIENTATION_180T);
    add_to_hash_vec(hash270t, GRID_HASH_ORIENTATION_270T);

    for (int bit_idx = 0; bit_idx < 8; bit_idx++)
    {
        const hash_t hash = hash_vec[bit_idx];

        if (mask & (1 << bit_idx))
        {
            assert(hash == hash0);
        }
    }
}

void test_grid_hash_relative_coord_transforms_impl(const std::string& board_as_string, unsigned int symmetry_mask)
{
    const vector<grid_pair_t> grids = string_to_all_grid_orientations(board_as_string, GRID_TYPE_COLOR);
    assert(grids.size() == GRID_HASH_ORIENTATIONS.size());

    unordered_map<hash_t, int_pair> hash_to_encoded_coord;

    for (const grid_pair_t& gp : grids)
    {
        const int_pair& shape = gp.second;

        grid_hash gh(symmetry_mask);
        const hash_t hash = get_grid_pair_hash(gp, gh);
        const grid_hash_orientation ori = gh.get_orientation();

        const int_pair coord = find_color_in_grid_pair(gp, WHITE);
        const int_pair coord_enc = grid_hash::get_transformed_coords(shape, coord, ori);
        assert(coord == grid_hash::get_inverse_transformed_coords(shape, coord_enc, ori));

        auto element_iterator = hash_to_encoded_coord.try_emplace(hash, coord_enc);

        // All orientations having the same hash should have the same encoded
        // coordinate
        const bool did_insert = element_iterator.second;
        const int_pair& element_coord = element_iterator.first->second;
        assert(LOGICAL_IMPLIES(!did_insert, element_coord == coord_enc));
    }
}

// Caller releases ownership of games
void test_db_encode_decode_grid_move_impl(vector<game*>& games)
{
    unordered_map<hash_t, set<::move>> hash_to_encoded_black_moves;
    unordered_map<hash_t, set<::move>> hash_to_encoded_white_moves;

    for (game* g : games)
    {
        const hash_t hash = g->get_local_hash();

        insert_or_assert_equal(hash_to_encoded_black_moves, hash, make_db_encoded_moves_for(*g, BLACK));
        insert_or_assert_equal(hash_to_encoded_white_moves, hash, make_db_encoded_moves_for(*g, WHITE));

        delete g;
    }
}

void test_db_encode_decode_grid_move_impl(vector<game*>&& games)
{
    test_db_encode_decode_grid_move_impl(games);
}

////////////////////////////////////////////////// Main test functions
void test_grid_hash()
{
    typedef tuple<string> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        // These are all the same clobber grid, but with different orientations
        "XO.|.X.",
        ".X|XO|..",
        ".X.|.OX",
        "..|OX|X.",
        "X.|OX|..",
        ".X.|XO.",
        "..|XO|.X",
        ".OX|.X.",
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const string& board_string = get<0>(test_case);

        const grid_pair_t grid_pair = string_to_grid(board_string);

        test_grid_hash_impl(GRID_HASH_ACTIVE_MASK_IDENTITY, grid_pair);

        test_grid_hash_impl(GRID_HASH_ACTIVE_MASK_ROTATION90, grid_pair);
        test_grid_hash_impl(GRID_HASH_ACTIVE_MASK_ROTATION180, grid_pair);

        test_grid_hash_impl(GRID_HASH_ACTIVE_MASK_MIRROR_VERT, grid_pair);
        test_grid_hash_impl(GRID_HASH_ACTIVE_MASK_MIRROR_HORIZ, grid_pair);
        test_grid_hash_impl(GRID_HASH_ACTIVE_MASK_MIRRORS, grid_pair);

        test_grid_hash_impl(GRID_HASH_ACTIVE_MASK_ALL, grid_pair);
    }
}

void test_grid_hash_static_coord_transforms()
{
    /*
       - shape
       - coords
       - vector of 8 shape/coord pairs. 1 for each grid_hash_orientation.
    */
    typedef tuple<int_pair, int_pair, vector<tuple<int_pair, int_pair>>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            {2, 3}, {0, 0},
            {
                {{2, 3}, {0, 0}},
                {{3, 2}, {0, 0}},
                {{3, 2}, {0, 1}},
                {{2, 3}, {1, 0}},
                {{2, 3}, {1, 2}},
                {{3, 2}, {2, 1}},
                {{3, 2}, {2, 0}},
                {{2, 3}, {0, 2}},
            },
        },

        {
            {3, 3}, {1, 1},
            {
                {{3, 3}, {1, 1}},
                {{3, 3}, {1, 1}},
                {{3, 3}, {1, 1}},
                {{3, 3}, {1, 1}},
                {{3, 3}, {1, 1}},
                {{3, 3}, {1, 1}},
                {{3, 3}, {1, 1}},
                {{3, 3}, {1, 1}},
            },
        },

        {
            {1, 1}, {0, 0},
            {
                {{1, 1}, {0, 0}},
                {{1, 1}, {0, 0}},
                {{1, 1}, {0, 0}},
                {{1, 1}, {0, 0}},
                {{1, 1}, {0, 0}},
                {{1, 1}, {0, 0}},
                {{1, 1}, {0, 0}},
                {{1, 1}, {0, 0}},
            },
        },

        {
            {3, 5}, {1, 3},
            {
                {{3, 5}, {1, 3}},
                {{5, 3}, {3, 1}},
                {{5, 3}, {3, 1}},
                {{3, 5}, {1, 3}},
                {{3, 5}, {1, 1}},
                {{5, 3}, {1, 1}},
                {{5, 3}, {1, 1}},
                {{3, 5}, {1, 1}},
            },
        },
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const int_pair& shape = get<0>(test_case);
        const int_pair& coords = get<1>(test_case);
        const vector<tuple<int_pair, int_pair>>& exp_transformed = get<2>(test_case);

        assert(exp_transformed.size() == GRID_HASH_ORIENTATIONS.size());

        const size_t N_ORIENTATIONS = GRID_HASH_ORIENTATIONS.size();
        for (size_t ori_idx = 0; ori_idx < N_ORIENTATIONS; ori_idx++)
        {
            const int_pair& exp_trans_shape = get<0>(exp_transformed[ori_idx]);
            const int_pair& exp_trans_coords = get<1>(exp_transformed[ori_idx]);

            const grid_hash_orientation ori = GRID_HASH_ORIENTATIONS[ori_idx];

            const int_pair trans_shape = grid_hash::get_transformed_shape(shape, ori);
            const int_pair trans_coords = grid_hash::get_transformed_coords(shape, coords, ori);

            assert(trans_shape == exp_trans_shape);
            assert(trans_coords == exp_trans_coords);

            const int_pair inv_trans_shape = grid_hash::get_inverse_transformed_shape(trans_shape, ori);
            const int_pair inv_trans_coords = grid_hash::get_inverse_transformed_coords(shape, trans_coords, ori);

            assert(inv_trans_shape == shape);
            assert(inv_trans_coords == coords);
        }
    }
}

void test_grid_hash_relative_coord_transforms()
{
    typedef tuple<string> test_case_t;

    vector<test_case_t> test_cases =
    {
        {
            "XO.|.X.",
            "O....|..#..|.....",
            ".....|...O.|.....",
        },
    };

    for (const test_case_t& test_case : test_cases)
    {
        const string& board_string = get<0>(test_case);

        test_grid_hash_relative_coord_transforms_impl(
            board_string, GRID_HASH_ACTIVE_MASK_IDENTITY);

        test_grid_hash_relative_coord_transforms_impl(
            board_string, GRID_HASH_ACTIVE_MASK_ROTATION90);

        test_grid_hash_relative_coord_transforms_impl(
            board_string, GRID_HASH_ACTIVE_MASK_ROTATION180);

        test_grid_hash_relative_coord_transforms_impl(
            board_string, GRID_HASH_ACTIVE_MASK_MIRROR_VERT);

        test_grid_hash_relative_coord_transforms_impl(
            board_string, GRID_HASH_ACTIVE_MASK_MIRROR_HORIZ);

        test_grid_hash_relative_coord_transforms_impl(
            board_string, GRID_HASH_ACTIVE_MASK_MIRRORS);

        test_grid_hash_relative_coord_transforms_impl(
            board_string, GRID_HASH_ACTIVE_MASK_ALL);
    }
}

void test_db_encode_decode_grid_move()
{
    // clobber_1xn
    test_db_encode_decode_grid_move_impl({
        new clobber_1xn("XOOXO"),
        new clobber_1xn("OOXOX"),
        make_impartial(new clobber_1xn("XOOXO")),
        make_impartial(new clobber_1xn("OOXOX")),
    });

    // nogo_1xn
    test_db_encode_decode_grid_move_impl({
        new nogo_1xn("...."),
        new nogo_1xn("..X.."),
        make_impartial(new nogo_1xn("....")),
        make_impartial(new nogo_1xn("..X..")),
    });

    // elephants
    test_db_encode_decode_grid_move_impl({
        new elephants("X...O...X..O"),
        new elephants("...X..O.."),
        make_impartial(new elephants("X...O...X..O")),
        make_impartial(new elephants("...X..O..")),
    });

    // toppling_dominoes
    test_db_encode_decode_grid_move_impl({
        new toppling_dominoes("X#O#X"),
        new toppling_dominoes("XOOOX"),
        make_impartial(new toppling_dominoes("X#O#X")),
        make_impartial(new toppling_dominoes("XOOOX")),
    });

    // gen_toads
    test_db_encode_decode_grid_move_impl({
        new gen_toads({1, 1, 0, 0}, "X...X..O..O"),
        make_impartial(new gen_toads({1, 1, 0, 0}, "X...X..O..O")),
        new gen_toads({1, 2, 1, 0}, "X..XO..O"),
        make_impartial(new gen_toads({1, 2, 1, 0}, "X..XO..O")),
        new gen_toads({2, 3, 1, 1}, "X..XO..O"),
        make_impartial(new gen_toads({2, 3, 1, 1}, "X..XO..O")),
    });

    // amazons
    test_db_encode_decode_grid_move_impl({
        create_grid_games_all_orientations<amazons>("X..|...|O..", GRID_TYPE_COLOR),
    });

    test_db_encode_decode_grid_move_impl({
        create_grid_games_all_orientations<amazons>("O.|.X", GRID_TYPE_COLOR),
    });

    // nogo
    test_db_encode_decode_grid_move_impl({
        create_grid_games_all_orientations<nogo>("...|...", GRID_TYPE_COLOR),
    });

    test_db_encode_decode_grid_move_impl({
        create_grid_games_all_orientations<nogo>("..|X.", GRID_TYPE_COLOR),
    });

    // clobber
    test_db_encode_decode_grid_move_impl({
        create_grid_games_all_orientations<clobber>("XO|OX", GRID_TYPE_COLOR),
    });

    test_db_encode_decode_grid_move_impl({
        create_grid_games_all_orientations<clobber>("XOX|O.O", GRID_TYPE_COLOR),
    });


    // cannibal_clobber
    test_db_encode_decode_grid_move_impl({
        create_grid_games_all_orientations<cannibal_clobber>("XO|OX", GRID_TYPE_COLOR),
    });

    // domineering
    test_db_encode_decode_grid_move_impl({
        create_grid_games_all_orientations<domineering>("...|.#.", GRID_TYPE_COLOR),
    });

    // fission
    test_db_encode_decode_grid_move_impl({
        create_grid_games_all_orientations<fission>(".X...|.....|..X..|...#.|.....", GRID_TYPE_COLOR),
    });

    // sheep
    test_db_encode_decode_grid_move_impl({
        create_grid_games_all_orientations<sheep>("3 0 0 0 | 0 2 -2 0 | 0 0 0 0 | 0 0 0 -3", GRID_TYPE_NUMBER),
    });

    // gen_king_dirt
    test_db_encode_decode_grid_move_impl({
        create_grid_games_all_orientations<gen_king_dirt>({1, 1, 0}, ".X|..", GRID_TYPE_COLOR),
    });

    test_db_encode_decode_grid_move_impl({
        create_grid_games_all_orientations<gen_king_dirt>({1, 1, 1}, "..|O.", GRID_TYPE_COLOR),
    });

    test_db_encode_decode_grid_move_impl({
        create_grid_games_all_orientations<gen_king_dirt>({2, 1, 1}, "..|X.", GRID_TYPE_COLOR),
    });
    

    // dyadic_rational
    test_db_encode_decode_grid_move_impl({
        new dyadic_rational(3, 8),
        make_impartial(new dyadic_rational(3, 8)),
        new dyadic_rational(-5, 8),
        make_impartial(new dyadic_rational(-5, 8)),
    });


    // integer_game
    test_db_encode_decode_grid_move_impl({
        new integer_game(4),
        make_impartial(new integer_game(4)),
        new integer_game(-4),
        make_impartial(new integer_game(-4)),
    });

    // nimber
    test_db_encode_decode_grid_move_impl({
        new nimber(2),
        new nimber(4),
    });

    // up_star
    test_db_encode_decode_grid_move_impl({
        new up_star(4, true),
        make_impartial(new up_star(4, true)),
        new up_star(-5, false),
        make_impartial(new up_star(-5, false)),
    });


    // switch_game
    {
        game* g1 = new switch_game(fraction(1, 4), fraction(-2, 4));
        game* g2 = new switch_game(fraction(-3, 4), fraction(-2, 4));
        play_nth_move(g1, 0);
        play_nth_move(g2, 0);

        game* g1_inv = g1->inverse();
        game* g1_clone = g1->clone();
        delete g1;

        game* g2_inv = g2->inverse();
        game* g2_clone = g2->clone();
        delete g2;

        vector<game*> games
        {
            new switch_game(fraction(1, 4), fraction(-2, 4)),
            make_impartial(new switch_game(fraction(1, 4), fraction(-2, 4))),
            new switch_game(fraction(-3, 4), fraction(-2, 4)),
            make_impartial(new switch_game(fraction(-3, 4), fraction(-2, 4))),
            g1_inv,
            g1_clone,
            g2_inv,
            g2_clone,
        };

        test_db_encode_decode_grid_move_impl(games);
    }

    // kayles
    {
        kayles* g1 = new kayles(5);
        play_nth_move(g1, 1);

        game* g2 = g1->clone();

        split_result sr1 = g1->split();
        assert(sr1.has_value() && sr1->size() == 2);
        delete g1;

        kayles* g1_1 = dynamic_cast<kayles*>((*sr1)[0]);
        kayles* g1_2 = dynamic_cast<kayles*>((*sr1)[1]);
        assert(g1_1 != nullptr && g1_2 != nullptr);

        split_result sr2 = g2->split();
        assert(sr2.has_value() && sr2->size() == 2);
        delete g2;

        kayles* g2_1 = dynamic_cast<kayles*>((*sr2)[0]);
        kayles* g2_2 = dynamic_cast<kayles*>((*sr2)[1]);
        assert(g2_1 != nullptr && g2_2 != nullptr);

        vector<game*> games
        {
            new kayles(5),
            new kayles(4),
            g1_1,
            g1_2,
            g2_1,
            g2_2,
        };

        test_db_encode_decode_grid_move_impl(games);
    }

}

} // namespace


//////////////////////////////////////////////////
void grid_hash_test_all()
{
    test_grid_hash();
    test_grid_hash_static_coord_transforms();
    test_grid_hash_relative_coord_transforms();
    test_db_encode_decode_grid_move();
}

