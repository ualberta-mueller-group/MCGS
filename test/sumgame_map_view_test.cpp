#include "sumgame_map_view_test.h"

#include "sumgame.h"
#include <vector>
#include <memory>
#include <cassert>
#include "all_game_headers.h"
#include "sumgame_change_record.h"
#include "sumgame_map_view.h"
#include "game_compare_utils.h"

using namespace std;
using namespace compare_games_by_print;

namespace {
void test_get_games()
{
    vector<shared_ptr<game>> sumgame_games {
        make_shared<integer_game>(0),       //
        make_shared<integer_game>(1),       //
        make_shared<integer_game>(2),       //
        make_shared<dyadic_rational>(3, 4), //
        make_shared<integer_game>(3),       //
    };

    vector<shared_ptr<integer_game>> expected_integers {
        make_shared<integer_game>(0), //
        make_shared<integer_game>(1), //
        make_shared<integer_game>(2), //
        make_shared<integer_game>(3), //
    };

    vector<shared_ptr<dyadic_rational>> expected_rationals {
        make_shared<dyadic_rational>(3, 4),
    };

    sumgame sum(BLACK);
    for (const shared_ptr<game>& ptr : sumgame_games)
        sum.add(ptr.get());

    sumgame_impl::change_record cr;
    sumgame_map_view map_view(sum, cr);

    {
        vector<game*>* integers_ptr =
            map_view.get_games_nullable(game_type<integer_game>());
        assert(integers_ptr != nullptr);
        assert(same_games(*integers_ptr, expected_integers));

        vector<game*>& integers_ref =
            map_view.get_games(game_type<integer_game>());
        assert(same_games(integers_ref, expected_integers));
    }

    {
        vector<game*>* rationals_ptr =
            map_view.get_games_nullable(game_type<dyadic_rational>());
        assert(rationals_ptr != nullptr);
        assert(same_games(*rationals_ptr, expected_rationals));

        vector<game*>& rationals_ref =
            map_view.get_games(game_type<dyadic_rational>());
        assert(same_games(rationals_ref, expected_rationals));
    }

    vector<game*>* clobber_ptr =
        map_view.get_games_nullable(game_type<clobber_1xn>());
    assert(clobber_ptr == nullptr);
}

void test_deactivate_games()
{
    vector<shared_ptr<game>> sumgame_games {
        make_shared<integer_game>(3),        //
        make_shared<clobber_1xn>("XO"),      //
        make_shared<integer_game>(5),        //
        make_shared<nogo_1xn>("X..O"),       //
        make_shared<clobber_1xn>("XO.OOOX"), //
        make_shared<integer_game>(7),        //
    };

    vector<shared_ptr<game>> expected_games {
        make_shared<nogo_1xn>("X..O"),       //
        make_shared<clobber_1xn>("XO"),      //
        make_shared<clobber_1xn>("XO.OOOX"), //
        // make_shared<integer_game>(3), //
        make_shared<integer_game>(5), //
        // make_shared<integer_game>(7), //
    };

    sumgame sum(BLACK);
    for (shared_ptr<game>& ptr : sumgame_games)
        sum.add(ptr.get());

    sumgame_impl::change_record cr;
    sumgame_map_view map_view(sum, cr);

    // deactivate some integer games
    {
        vector<game*>* integer_games =
            map_view.get_games_nullable(game_type<integer_game>());
        assert(integer_games != nullptr);
        assert(integer_games->size() == 3);

        vector<game*> to_deactivate {
            (*integer_games)[0],
            (*integer_games)[2],
        };

        map_view.deactivate_games(to_deactivate);

        integer_games = map_view.get_games_nullable(game_type<integer_game>());
        assert(integer_games != nullptr);
        assert(integer_games->size() == 1);

        assert(sumgame_same_games(sum, expected_games));

        // Disable last integer_game
        map_view.deactivate_game(integer_games->back());
        expected_games.pop_back();
        assert(sumgame_same_games(sum, expected_games));

        integer_games = map_view.get_games_nullable(game_type<integer_game>());
        assert(integer_games == nullptr);
    }

    // Normally would be cleared by an undo function from within sumgame minimax
    // search
    cr.deactivated_games.clear();
}

void test_add_game()
{
    vector<shared_ptr<game>> sumgame_games {
        make_shared<integer_game>(1), //
        make_shared<integer_game>(3), //
        make_shared<integer_game>(5), //
    };

    vector<shared_ptr<game>> expected_games {
        make_shared<integer_game>(1),   //
        make_shared<integer_game>(3),   //
        make_shared<integer_game>(5),   //
                                        //
        make_shared<clobber_1xn>("XO"), //
        make_shared<integer_game>(7),   //
    };

    sumgame sum(BLACK);
    for (shared_ptr<game>& ptr : sumgame_games)
        sum.add(ptr.get());

    sumgame_impl::change_record cr;
    sumgame_map_view map_view(sum, cr);

    vector<shared_ptr<game>> add_games;

    // Add some games
    {
        // add_game() with specifically typed game pointer
        add_games.emplace_back(make_shared<integer_game>(7));
        integer_game* int_game =
            cast_game<integer_game*>(add_games.back().get());
        map_view.add_game(int_game);

        // add_game() with "game" type game pointer
        add_games.emplace_back(make_shared<clobber_1xn>("XO"));
        game* clobber_game = add_games.back().get();
        map_view.add_game(clobber_game);
    }

    // Normally would be cleared by an undo function from within sumgame minimax
    // search
    cr.added_games.clear();
}
} // namespace

void sumgame_map_view_test_all()
{
    test_get_games();
    test_deactivate_games();
    test_add_game();
}
