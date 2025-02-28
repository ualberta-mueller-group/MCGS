#include "sumgame_change_record.h"
#include "cgt_dyadic_rational.h"
#include "cgt_integer_game.h"
#include "obj_id.h"
#include <climits>
#include <type_traits>
#include <vector>
#include "sumgame_map_view.h"
#include "fraction.h"

#include "cgt_nimber.h"
#include "cgt_up_star.h"
#include "utilities.h"

using namespace std;

//////////////////////////////////////// declarations
void simplify_basic_nimber(sumgame_map_view& map_view);
void simplify_basic_up_star(sumgame_map_view& map_view);
void simplify_basic_integers_rationals(sumgame_map_view& map_view);

//////////////////////////////////////// change_record
namespace sumgame_impl {

change_record::~change_record()
{
    // owner should have called one of the undo functions
    assert(deactivated_games.empty());
    assert(added_games.empty());
}

change_record::change_record(change_record&& other) noexcept
{
    _move_impl(std::forward<change_record>(other));
}

change_record& change_record::operator=(change_record&& other) noexcept
{
    _move_impl(std::forward<change_record>(other));
    return *this;
}

void change_record::simplify_basic(sumgame& sum)
{
    sumgame_map_view map_view(sum, *this);
    simplify_basic_nimber(map_view);
    simplify_basic_up_star(map_view);
    simplify_basic_integers_rationals(map_view);
    return;
}

void change_record::undo_simplify_basic(sumgame& sum)
{
    // IMPORTANT: reactivate before deleting to avoid use after free
    for (auto it = deactivated_games.rbegin(); it != deactivated_games.rend(); it++)
    {
        game* g = *it;

        assert(!g->is_active());
        g->set_active(true);
    }

    for (auto it = added_games.rbegin(); it != added_games.rend(); it++)
    {
        game* g = *it;

        assert(g->is_active());
        sum.pop(g);
        delete g;
    }

    _clear();
}

void change_record::_clear()
{
    deactivated_games.clear();
    added_games.clear();
}


void change_record::_move_impl(change_record&& other) noexcept
{
    deactivated_games = std::move(other.deactivated_games);
    added_games = std::move(other.added_games);

    other._clear();
}


} // namespace sumgame_impl

//////////////////////////////////////// helper functions

void simplify_basic_nimber(sumgame_map_view& map_view)
{
    vector<game*>* nimbers = map_view.get_games_nullable(get_obj_id<nimber>());

    if (nimbers == nullptr)
    {
        return;
    }
        
    vector<int> heap_vec;

    for (game* g : *nimbers)
    {
        nimber* g_nimber = cast_game<nimber*>(g);

        int val = g_nimber->value();
        heap_vec.push_back(val);
    }

    if (heap_vec.size() >= 2 || (heap_vec.size() == 1 && heap_vec.back() < 2))
    {
        map_view.deactivate_games(*nimbers);

        int sum = nimber::nim_sum(heap_vec);
        assert(sum >= 0);
    
        // 0: add nothing

        if (sum == 1) // 1: star
        {
            up_star* new_game = new up_star(0, true);
            map_view.add_game(new_game);
        } 
        else if (sum >= 2) // >= 2: nimber
        {
            nimber* new_game = new nimber(sum);
            map_view.add_game(new_game);
        }
    }

}

void simplify_basic_up_star(sumgame_map_view& map_view)
{
    vector<game*>* up_stars = map_view.get_games_nullable(get_obj_id<up_star>());

    if (up_stars == nullptr || up_stars->size() < 2)
    {
        return;
    }

    int ups = 0;
    bool star = false;

    vector<game*> consumed_games; // if an addition will overflow, not all up_stars will be consumed

    for (game* g : *up_stars)
    {
        up_star* g_up_star = cast_game<up_star*>(g);

        if (!safe_add(ups, g_up_star->num_ups()))
        {
            break;
        }

        star ^= g_up_star->has_star();
        consumed_games.push_back(g_up_star);
    }

    if (consumed_games.size() < 2)
    {
        return;
    }

    map_view.deactivate_games(consumed_games);

    if (ups != 0 || star != false)
    {
        up_star* new_game = new up_star(ups, star);
        map_view.add_game(new_game);
    }
}

void simplify_basic_integers_rationals(sumgame_map_view& map_view)
{
    vector<game*>* integers = map_view.get_games_nullable(get_obj_id<integer_game>());
    vector<game*>* rationals = map_view.get_games_nullable(get_obj_id<dyadic_rational>());

    size_t game_count = 0;
    game_count += (integers == nullptr) ? 0 : integers->size();
    game_count += (rationals == nullptr) ? 0 : rationals->size();

    if (game_count <= 1)
    {
        return;
    }

    vector<game*> consumed_integers;
    vector<game*> consumed_rationals;

    // add up integers
    int int_sum = 0;
    if (integers != nullptr)
    {
        for (game* g : *integers)
        {
            integer_game* g_integer = cast_game<integer_game*>(g);

            if (!safe_add(int_sum, g_integer->value()))
            {
                break;
            }

            consumed_integers.push_back(g_integer);
        }
    }

    // add up rationals
    fraction rational_sum(0, 1);
    if (rationals != nullptr)
    {
        for (game* g : *rationals)
        {
            dyadic_rational* g_rational = cast_game<dyadic_rational*>(g);
            fraction f(*g_rational);

            if (!safe_add_fraction(rational_sum, f))
            {
                break;
            }

            consumed_rationals.push_back(g_rational);
        }
    }

    size_t n_consumed_games = consumed_integers.size() + consumed_rationals.size();

    if (n_consumed_games < 2)
    {
        return;
    }

    fraction final_sum(int_sum, 1);
    bool final_sum_valid = safe_add_fraction(final_sum, rational_sum);

    auto insert_game = [&](int top, int bottom) -> void
    {
        assert(bottom > 0);

        if (top == 0)
        {
            return;
        }

        if (bottom == 1)
        {
            integer_game* new_game = new integer_game(top);
            map_view.add_game(new_game);
            return;
        }

        dyadic_rational* new_game = new dyadic_rational(top, bottom);
        map_view.add_game(new_game);
    };


    // now commit only useful cases
    assert(n_consumed_games >= 2);

    if (final_sum_valid)
    {
        map_view.deactivate_games(consumed_integers);
        map_view.deactivate_games(consumed_rationals);
        insert_game(final_sum.top, final_sum.bottom);
        return;
    }

    if (consumed_integers.size() > 1)
    {
        map_view.deactivate_games(consumed_integers);
        insert_game(int_sum, 1);
    }

    if (consumed_rationals.size() > 1)
    {
        map_view.deactivate_games(consumed_rationals);
        insert_game(rational_sum.top, rational_sum.bottom);
    }
}

