#include "sumgame_change_record.h"
#include "cgt_dyadic_rational.h"
#include "cgt_integer_game.h"
#include "obj_id.h"
#include <climits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "cgt_nimber.h"
#include "cgt_up_star.h"
#include "utilities.h"

using namespace std;

//////////////////////////////////////// declarations
struct fraction
{
    inline fraction(int top, int bottom): top(top), bottom(bottom)
    {
        assert(bottom > 0);
        assert(is_power_of_2(bottom));
    }

    int top;
    int bottom;
};

ostream& operator<<(ostream& os, const fraction& f)
{
    os << f.top << "/" << f.bottom;
    return os;
}

void simplify_basic_nimber(sumgame_map_view& map_view);
void simplify_basic_up_star(sumgame_map_view& map_view);
void simplify_basic_integers_rationals(sumgame_map_view& map_view);

//////////////////////////////////////// sumgame_map_view
sumgame_map_view::sumgame_map_view(sumgame& sum, sumgame_impl::change_record& record): _sum(sum), _record(record)
{
    // build game map
    const int N = sum.num_total_games();

    for (int i = 0; i < N; i++)
    {
        game* g = sum.subgame(i);

        if (g->is_active())
        {
            obj_id_t obj_id = g->get_obj_id();
            _map[obj_id].push_back(g);
        }
    }
}

vector<game*>* sumgame_map_view::get_games_nullable(obj_id_t obj_id)
{
    auto it = _map.find(obj_id);

    if (it == _map.end())
    {
        return nullptr;
    }

    vector<game*>& map_games = it->second;
    vector<game*> active_games;

    for (game* g : map_games)
    {
        if (g->is_active())
        {
            active_games.push_back(g);
        }
    }

    swap(map_games, active_games);

    if (map_games.empty())
    {
        return nullptr;
    }

    return &map_games;
}

vector<game*>& sumgame_map_view::get_games(obj_id_t obj_id)
{
    return _map[obj_id];
}

void sumgame_map_view::deactivate_game(game* g)
{
    assert(g->is_active());
    g->set_active(false);
    _record.deactivated_games.push_back(g);
}

void sumgame_map_view::deactivate_games(vector<game*>& games)
{
    for (game* g : games)
    {
        deactivate_game(g);
    }
}

//////////////////////////////////////// fraction
static_assert(int32_t(-1) == int32_t(0xFFFFFFFF), "Not two's complement");
static_assert(numeric_limits<int>::min() < 0);

void simplify_fraction(fraction& f) // handles all error cases
{
    assert(f.bottom >= 1);
    assert(is_power_of_2(f.bottom));

    // right shift OK because operands are signed
    static_assert(is_integral_v<decltype(f.top)> && is_signed_v<decltype(f.top)>);
    static_assert(is_integral_v<decltype(f.bottom)> && is_signed_v<decltype(f.bottom)>);

    while ((f.top & 0x1) == 0 && (f.bottom & 0x1) == 0)
    {
        f.top >>= 1;
        f.bottom >>= 1;
    }
}

// TODO should have a static_assert to check that abs(INT_MIN) == INT_MAX + 1
bool raise_denominator(fraction& frac, int target_bottom)
{
    assert(target_bottom >= frac.bottom);

    assert(frac.bottom > 0);
    assert(is_power_of_2(frac.bottom));

    assert(target_bottom > 0);
    assert(is_power_of_2(target_bottom));

    if (frac.top == std::numeric_limits<int>::min())
    {
        return false;
    }

    // i.e. 11000...0 (2 bits to avoid changing sign)
    const int mask = int(0x3) << (sizeof(int) * CHAR_BIT - 2);

    cout << "BITS: ";
    print_bits(cout, mask);
    cout << endl;

    auto left_shift_safe = [](fraction& f) -> bool
    {
        if ((mask & f.top) != 0 || (mask & f.bottom) != 0)
        {
            return false;
        }

        f.top <<= 1;
        f.bottom <<= 1;

        return true;
    };

    const int frac_top_copy = frac.top;
    const int frac_bottom_copy = frac.bottom;

    bool flip_sign = false;

    if (frac.top < 0)
    {
        flip_sign = true;

        assert(abs(frac.top) == abs(-frac.top));
        frac.top = -frac.top;
    }

    while (frac.bottom < target_bottom && left_shift_safe(frac))
    { }

    assert(frac.bottom <= target_bottom);

    if (flip_sign)
    {
        assert(abs(frac.top) == abs(-frac.top));
        frac.top = -frac.top;
    }

    if (frac.bottom == target_bottom)
    {
        return true;
    }

    frac.top = frac_top_copy;
    frac.bottom = frac_bottom_copy;
    return false;
}

//////////////////////////////////////// change_record
namespace sumgame_impl {

change_record::~change_record()
{
    // owner should have called one of the undo functions
    assert(deactivated_games.empty());
    assert(added_games.empty());
}

void change_record::simplify_basic(sumgame& sum)
{
    sumgame_map_view map_view(sum, *this);
    simplify_basic_nimber(map_view);
    simplify_basic_up_star(map_view);
    simplify_basic_integers_rationals(map_view);
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

} // namespace sumgame_impl

//////////////////////////////////////// helper functions

void simplify_basic_nimber(sumgame_map_view& map_view) // handles all error cases
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

void simplify_basic_up_star(sumgame_map_view& map_view) // handles all error cases
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

bool safe_add_fraction(fraction& x, fraction& y)
{
    simplify_fraction(x);
    simplify_fraction(y);

    int target_bottom = max(x.bottom, y.bottom);

    if (!raise_denominator(x, target_bottom) || !raise_denominator(y, target_bottom))
    {
        return false;
    }

    assert(x.bottom == y.bottom);

    if (addition_will_wrap(x.top, y.top))
    {
        return false;
    }

    x.top += y.top;
    return true;
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

            const int top = g_rational->p();
            const int bottom = g_rational->q();
            fraction f(top, bottom);

            if (!safe_add_fraction(rational_sum, f))
            {
                break;
            }

            consumed_rationals.push_back(g_rational);
        }
    }
}

