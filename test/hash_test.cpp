#include "hash_test.h"
#include <functional>
#include <iostream>
#include <memory>
#include <type_traits>
#include <unordered_set>

#include "clobber_1xn.h"
#include "game.h"
#include "sumgame.h"
#include "hashing.h"
#include "all_game_headers.h"
#include "sstream"
#include "test/test_utilities.h"

using namespace std;

namespace {

/*
       Recursively plays moves on a game.

    Checks that hash after a move is unique among hashes for moves by the
    same player at the current node, and is different from the initial hash.

    Also checks that the hash after undoing a move is the initial hash.
*/
void test_game_recursive(game& g, int remaining_depth)
{
    if (remaining_depth == 0)
        return;

    const hash_t initial_hash = g.compute_hash().get_value();

    unique_ptr<move_generator> mgp1(g.create_move_generator(BLACK));
    unique_ptr<move_generator> mgp2(g.create_move_generator(WHITE));

    for (move_generator* mg: {mgp1.get(), mgp2.get()})
    {
        unordered_set<hash_t> hash_set;
        hash_set.insert(initial_hash);

        while (*mg)
        {
            const ::move m = mg->gen_move();
            ++(*mg);

            g.play(m, mg->to_play());

            const hash_t new_hash = g.compute_hash().get_value();
            assert(new_hash != initial_hash);

            auto it = hash_set.insert(new_hash);
            assert(it.second);

            test_game_recursive(g, remaining_depth - 1);

            g.undo_move();
            const hash_t undo_hash = g.compute_hash().get_value();
            assert(undo_hash == initial_hash);
        }
    }
}

/*
        Recursively plays moves on a sum.

    Checks that hash after a move is different from the initial hash. Also
    checks that hash after undoing a move is the initial hash.
*/
void test_sum_recursive(sumgame& sum, int remaining_depth)
{
    if (remaining_depth == 0)
        return;

    const bw initial_to_play = sum.to_play();
    const hash_t initial_hash = sum.get_global_hash_value();

    unique_ptr<sumgame_move_generator> mgp1(sum.create_sum_move_generator(BLACK));
    unique_ptr<sumgame_move_generator> mgp2(sum.create_sum_move_generator(WHITE));

    for (sumgame_move_generator* mg: {mgp1.get(), mgp2.get()})
    {
        sum.set_to_play(mg->to_play());

        while (*mg)
        {
            const sumgame_move m = mg->gen_sum_move();
            ++(*mg);

            assert(sum.to_play() == mg->to_play());
            sum.play_sum(m, mg->to_play());

            const hash_t new_hash = sum.get_global_hash_value();
            assert(new_hash != initial_hash);

            test_sum_recursive(sum, remaining_depth - 1);

            sum.undo_move();

            const hash_t undo_hash = sum.get_global_hash_value();
            assert(undo_hash == initial_hash);
        }
    }

    sum.set_to_play(initial_to_play);
}

// Call test_game_recursive() and test_sum_recursive()
void test_recursive_all()
{
    {
        vector<shared_ptr<game>> games
        {
            shared_ptr<game>(new clobber_1xn("XOXOXOXO")),
            shared_ptr<game>(new nogo_1xn("X....O")),
            shared_ptr<game>(new elephants("...O.X..O..X...X.X..O")),
            shared_ptr<game>(new integer_game(-20)),
            shared_ptr<game>(new nimber(3)),
            shared_ptr<game>(new up_star(6, true)),
            shared_ptr<game>(new dyadic_rational(fraction(9, 8))),
            shared_ptr<game>(new switch_game(fraction(21, 16), fraction(-13, 4))),
        };

        for (shared_ptr<game>& gp : games)
        {
            test_game_recursive(*gp, 4);
        }
    }

    {
        vector<shared_ptr<game>> games
        {
            shared_ptr<game>(new clobber_1xn("XOXOXO")),
            shared_ptr<game>(new nogo_1xn("....")),
            shared_ptr<game>(new elephants("X...O")),
            shared_ptr<game>(new integer_game(-3)),
            shared_ptr<game>(new nimber(2)),
            shared_ptr<game>(new up_star(2, true)),
            shared_ptr<game>(new dyadic_rational(fraction(-3, 8))),
            shared_ptr<game>(new switch_game(fraction(3, 16), fraction(-7, 4))),

        };

        sumgame sum(BLACK);

        for (shared_ptr<game>& gp : games)
        {
            sum.add(gp.get());
        }

        test_sum_recursive(sum, 4);
    }
}


/*
    Play moves on a strip non-recursively, and check that the hash after a move
    is different from the initial hash, and equal to computing the hash for a
    copy of the game obtained after the move.
*/
template <class T>
void test_strip(T& g)
{
    static_assert(is_base_of_v<strip, T>);
    static_assert(!is_abstract_v<T>);

    const hash_t initial_hash = g.compute_hash().get_value();

    shared_ptr<move_generator> mg_black(g.create_move_generator(BLACK));
    shared_ptr<move_generator> mg_white(g.create_move_generator(WHITE));

    for (move_generator* mg : {mg_black.get(), mg_white.get()})
    {
        for (; *mg; ++(*mg))
        {
            const ::move m = mg->gen_move();
            g.play(m, mg->to_play());

            const hash_t new_hash = g.compute_hash().get_value();
            assert(new_hash != initial_hash);

            T game_copy(g.board_as_string());
            const hash_t expected_hash = game_copy.compute_hash().get_value();

            assert(new_hash == expected_hash);

            g.undo_move();
            const hash_t undo_hash = g.compute_hash().get_value();

            assert(undo_hash == initial_hash);
        }
    }
}

template <class T>
void test_strip(T&& g)
{
    test_strip(g);
}

// check that all permutations consisting of all games give the same global hash
hash_t test_sum_order_impl(sumgame& sum, vector<game*>& games, vector<bool>& used)
{
    assert(games.size() == used.size());

    bool first = true;
    hash_t hash = 0;

    const size_t N = games.size();
    for (size_t i = 0; i < N; i++)
    {
        if (used[i])
            continue;

        used[i] = true;
        game* g = games[i];

        sum.add(g);
        const hash_t got_hash = test_sum_order_impl(sum, games, used);

        sum.pop(g);
        used[i] = false;

        if (first)
        {
            first = false;
            hash = got_hash;
            continue;
        }

        assert(got_hash == hash);
    }

    if (!first)
        return hash;

    assert(sum.num_active_games() == games.size());
    hash = sum.get_global_hash_value();
    return hash;
}

void test_sum_order()
{
    vector<shared_ptr<game>> games_managed
    {
        shared_ptr<game>(new clobber_1xn("XOXOXOXO")),
        shared_ptr<game>(new nogo_1xn("X....O")),
        shared_ptr<game>(new elephants("...O.X..O..X...X.X..O")),
        shared_ptr<game>(new integer_game(-20)),
        shared_ptr<game>(new nimber(3)),
        shared_ptr<game>(new up_star(6, true)),
        shared_ptr<game>(new dyadic_rational(fraction(9, 8))),
        shared_ptr<game>(new switch_game(fraction(21, 16), fraction(-13, 4))),
    };

    vector<game*> games;
    for (shared_ptr<game>& sp : games_managed)
    {
        games.push_back(sp.get());
    }

    vector<bool> used(games.size());

    sumgame sum(BLACK);

    sum.set_to_play(BLACK);
    test_sum_order_impl(sum, games, used);

    sum.set_to_play(WHITE);
    test_sum_order_impl(sum, games, used);
}

void test_sum_mutate()
{
    assert(false); // TODO
}

} // namespace

void hash_test_all()
{
    test_recursive_all();

    test_strip(clobber_1xn("XOXOXOXOXOXOXOXO"));
    test_strip(nogo_1xn("........."));
    test_strip(elephants("...O..X..X...O..X.O.X.O"));

    test_sum_order();
    test_sum_mutate();
}
