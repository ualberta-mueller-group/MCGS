#include "game_clone_test.h"

#include <memory>

#include "game.h"
#include "impartial_game_wrapper.h"
#include "sumgame.h"
#include "all_game_headers.h"
#include "test_utilities.h"

using namespace std;

namespace {
////////////////////////////////////////////////// Helpers
// Caller releases ownership of `g`
inline game* make_impartial(game* g)
{
    assert(g != nullptr && !g->is_impartial());
    return new impartial_game_wrapper(g, true);
}

void compare_games_after_clone(const game* g1, const game* g2)
{
    assert(g1 != nullptr && g2 != nullptr);
    assert(g1 != g2);

    assert(g1->game_type() == g2->game_type());
    assert(g1->to_string() == g2->to_string());
    assert(g1->get_local_hash() == g2->get_local_hash());

    assert(get_generated_moves_for_player(g1, BLACK) ==
           get_generated_moves_for_player(g2, BLACK));

    assert(get_generated_moves_for_player(g1, WHITE) ==
           get_generated_moves_for_player(g2, WHITE));
}

void game_clone_test_compare_impl(game* g1)
{
    assert(g1 != nullptr);

    game* g2 = g1->clone();
    assert(g2 != nullptr && g1 != g2);
    assert(g2->undo_stack_size() == 0);

    compare_games_after_clone(g1, g2);

    /*
        Check that playing moves on g2 doesn't affect g1, AND that
        g2 play/undo sequence works.
    */
    const std::string g1_string_before = g1->to_string();
    const hash_t g1_hash_before = g1->get_local_hash();
    const int g1_undo_stack_size_before = g1->undo_stack_size();

    const std::string g2_string_before = g2->to_string();
    const hash_t g2_hash_before = g2->get_local_hash();
    const int g2_undo_stack_size_before = g2->undo_stack_size();

    constexpr std::array<bw, 2> COLORS = {BLACK, WHITE};
    for (const bw color : COLORS)
    {
        std::unique_ptr<move_generator> gen(g2->create_move_generator(color));

        while (*gen)
        {
            const ::move m = gen->gen_move();
            ++(*gen);

            g2->play(m, color);
            assert(g1->to_string() == g1_string_before);
            assert(g1->get_local_hash() == g1_hash_before);
            assert(g1->undo_stack_size() == g1_undo_stack_size_before);

            g2->undo_move();
            assert(g2->to_string() == g2_string_before);
            assert(g2->get_local_hash() == g2_hash_before);
            assert(g2->undo_stack_size() == g2_undo_stack_size_before);
        }
    }

    // Check that g2 - g1 == 0
    game* g1_inv = g1->inverse();

    sumgame s(BLACK);
    s.add(g1_inv);
    s.add(g2);

    s.set_to_play(BLACK);
    const bool black_wins = s.solve();
    s.set_to_play(WHITE);
    const bool white_wins = s.solve();

    assert(!black_wins);
    assert(!white_wins);

    s.pop(g2);
    s.pop(g1_inv);

    delete g2;
    delete g1_inv;
}

// Caller releases ownership of `g`
void game_clone_test_main_impl(game* g)
{
    assert(g != nullptr);

    // Test game as is
    game_clone_test_compare_impl(g);

    // Test afer playing a move
    play_nth_move(g, 0);
    g->normalize();
    const hash_t hash_after_move = g->get_local_hash();
    game_clone_test_compare_impl(g);
    assert(g->get_local_hash() == hash_after_move);

    delete g;
}

} // namespace

////////////////////////////////////////////////// Exported functions
void game_clone_test_all()
{
    // clobber_1xn
    game_clone_test_main_impl(new clobber_1xn("XOXOX"));
    game_clone_test_main_impl(make_impartial(new clobber_1xn("XXOOX")));

    // nogo_1xn
    game_clone_test_main_impl(new nogo_1xn(".X.O."));
    game_clone_test_main_impl(make_impartial(new nogo_1xn("..X....")));

    // elephants
    game_clone_test_main_impl(new elephants("..X.O.X."));
    game_clone_test_main_impl(make_impartial(new elephants("..O.O.X...O.")));

    // toppling_dominoes
    {
        /*
            Toppling Dominoes sometimes internally mirrors its board to
            implement normalization
        */

        toppling_dominoes* g1 = new toppling_dominoes("XXO#OX");
        toppling_dominoes* g1_n = new toppling_dominoes("XXO#OX");
        g1_n->normalize();
        game* g1_imp = make_impartial(new toppling_dominoes("XXO#OX"));

        toppling_dominoes* g2 = new toppling_dominoes("OOXXOX");
        toppling_dominoes* g2_n = new toppling_dominoes("OOXXOX");
        g2_n->normalize();
        game* g2_imp = make_impartial(new toppling_dominoes("OOXXOX"));

        assert(g1->current_dominoes() == g1_n->current_dominoes());
        assert(g2->current_dominoes() != g2_n->current_dominoes());

        game_clone_test_main_impl(g1);
        game_clone_test_main_impl(g1_n);
        game_clone_test_main_impl(g1_imp);
        game_clone_test_main_impl(g2);
        game_clone_test_main_impl(g2_n);
        game_clone_test_main_impl(g2_imp);
    }

    // gen_toads
    {
        game* g1 = new gen_toads({1, 1, 0, 0}, "X...X..O..O");
        game* g1_imp = make_impartial(new gen_toads({1, 1, 0, 0}, "X...X..O..O"));

        game* g2 = new gen_toads({1, 2, 1, 0}, "X..XO..O");
        game* g2_imp = make_impartial(new gen_toads({1, 2, 1, 0}, "X..XO..O"));

        game* g3 = new gen_toads({2, 3, 1, 1}, "X..XO..O");
        game* g3_imp = make_impartial(new gen_toads({2, 3, 1, 1}, "X..XO..O"));

        game_clone_test_main_impl(g1);
        game_clone_test_main_impl(g1_imp);
        game_clone_test_main_impl(g2);
        game_clone_test_main_impl(g2_imp);
        game_clone_test_main_impl(g3);
        game_clone_test_main_impl(g3_imp);
    }

    // amazons
    game_clone_test_main_impl(new amazons(".X|.O"));
    game_clone_test_main_impl(make_impartial(new amazons(".X|.O")));

    // nogo
    game_clone_test_main_impl(new nogo("..|.."));
    game_clone_test_main_impl(make_impartial(new nogo("..|..")));
    game_clone_test_main_impl(
        new nogo({BLACK, EMPTY}, {BORDER, EMPTY}, int_pair(1, 2)));
    game_clone_test_main_impl(make_impartial(
        new nogo({BLACK, EMPTY}, {BORDER, EMPTY}, int_pair(1, 2))));

    // clobber
    game_clone_test_main_impl(new clobber("XO|OX"));
    game_clone_test_main_impl(make_impartial(new clobber("XO|OX")));

    // cannibal_clobber
    game_clone_test_main_impl(new cannibal_clobber("XO|OX"));
    game_clone_test_main_impl(make_impartial(new cannibal_clobber("XO|OX")));

    // domineering
    game_clone_test_main_impl(new domineering("..|.."));
    game_clone_test_main_impl(make_impartial(new domineering("..|..")));

    // fission
    game_clone_test_main_impl(new fission("...|.X.|..."));
    game_clone_test_main_impl(make_impartial(new fission("...|.X.|...")));

    // sheep
    game_clone_test_main_impl(new sheep("3 0 0 | 0 0 -3"));
    game_clone_test_main_impl(make_impartial(new sheep("3 0 0 | 0 0 -3")));

    // gen_king_dirt
    game_clone_test_main_impl(new gen_king_dirt({1, 1, 0}, "..|.."));
    game_clone_test_main_impl(make_impartial(new gen_king_dirt({1, 1, 0}, "..|..")));

    game_clone_test_main_impl(new gen_king_dirt({1, 1, 1}, "..|.."));
    game_clone_test_main_impl(make_impartial(new gen_king_dirt({1, 1, 1}, "..|..")));

    game_clone_test_main_impl(new gen_king_dirt({2, 1, 1}, "..|.."));
    game_clone_test_main_impl(make_impartial(new gen_king_dirt({2, 1, 1}, "..|..")));

    // dyadic_rational
    game_clone_test_main_impl(new dyadic_rational(3, 8));
    game_clone_test_main_impl(make_impartial(new dyadic_rational(3, 8)));

    game_clone_test_main_impl(new dyadic_rational(-5, 8));
    game_clone_test_main_impl(make_impartial(new dyadic_rational(-5, 8)));

    // integer_game
    game_clone_test_main_impl(new integer_game(4));
    game_clone_test_main_impl(make_impartial(new integer_game(4)));

    game_clone_test_main_impl(new integer_game(-4));
    game_clone_test_main_impl(make_impartial(new integer_game(-4)));

    // nimber
    game_clone_test_main_impl(new nimber(2));
    game_clone_test_main_impl(new nimber(4));

    // up_star
    game_clone_test_main_impl(new up_star(4, true));
    game_clone_test_main_impl(make_impartial(new up_star(4, true)));

    game_clone_test_main_impl(new up_star(-5, false));
    game_clone_test_main_impl(make_impartial(new up_star(-5, false)));


    // switch_game
    {
        game_clone_test_main_impl(new switch_game(fraction(1, 4), fraction(-2, 4)));
        game_clone_test_main_impl(
            make_impartial(new switch_game(fraction(1, 4), fraction(-2, 4))));

        game_clone_test_main_impl(
            new switch_game(fraction(-3, 4), fraction(-2, 4)));
        game_clone_test_main_impl(
            make_impartial(new switch_game(fraction(-3, 4), fraction(-2, 4))));

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

        game_clone_test_main_impl(g1_inv);
        game_clone_test_main_impl(g1_clone);

        game_clone_test_main_impl(g2_inv);
        game_clone_test_main_impl(g2_clone);
    }

    // kayles
    {
        game_clone_test_main_impl(new kayles(4));

        /*
            Kayles requires splitting after calling `play`. Check that
            cloning works after calling `play` and before calling `split`.
        */
        kayles* g1 = new kayles(5);
        play_nth_move(g1, 1);

        game* g2 = g1->clone();

        split_result sr1 = g1->split();
        assert(sr1.has_value() && sr1->size() == 2);
        delete g1;

        kayles* g1_1 = dynamic_cast<kayles*>((*sr1)[0]);
        kayles* g1_2 = dynamic_cast<kayles*>((*sr1)[1]);
        assert(g1_1 != nullptr && g1_2 != nullptr);
        assert(g1_1->value() == 3 && g1_2->value() == 1);

        split_result sr2 = g2->split();
        assert(sr2.has_value() && sr2->size() == 2);
        delete g2;

        kayles* g2_1 = dynamic_cast<kayles*>((*sr2)[0]);
        kayles* g2_2 = dynamic_cast<kayles*>((*sr2)[1]);
        assert(g2_1 != nullptr && g2_2 != nullptr);
        assert(g2_1->value() == 3 && g2_2->value() == 1);

        compare_games_after_clone(g1_1, g2_1);
        compare_games_after_clone(g1_2, g2_2);

        game_clone_test_main_impl(g1_1);
        game_clone_test_main_impl(g1_2);

        game_clone_test_main_impl(g2_1);
        game_clone_test_main_impl(g2_2);
    }

}
