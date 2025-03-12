#include "cgt_game_simplification.h"

#include "fraction.h"
#include "safe_arithmetic.h"
#include "game_type.h"

#include "cgt_nimber.h"
#include "cgt_up_star.h"
#include "cgt_switch.h"

using namespace std;

//////////////////////////////////////// helper functions
namespace {
bool convert_number_switch(fraction x, fraction y, sumgame_map_view& map_view)
{
    if (!fraction::make_compatible(x, y))
        return false;
    assert(x.bottom() == y.bottom());
    assert(x.top() <= y.top());

    // 0 case
    if (x.top() < 0 && y.top() > 0)
    {
        return true;
    }

    // equal case
    if (x.top() == y.top())
    {
        map_view.add_game(new dyadic_rational(x));
        map_view.add_game(new up_star(0, true));
        return true;
    }

    // now (x < y <= 0) OR (0 <= x < y), and we want case 2
    bool swapped = false;
    if (x.top() < 0)
    {
        swapped = true;
        x.negate();
        y.negate();

        swap(x, y);
    }
    assert(0 <= x.top() && x.top() < y.top());

    if ((x.top() + 1 == y.top()) && ( // this addition is safe because x.top() < y.top()
        !x.raise_denominator_by_pow2(1) ||
        !y.raise_denominator_by_pow2(1)
    ))
        return false;

    /*
        find u = i/2^j such that x < u < y, iteratively increasing j

        Here we work with numerators and assume that x, y, and u all have the same
            denominator D. Then, x = xn/D, y = yn/D, u = un/D

        j = 0 --> un = D     --> u = 1
        j = 1 --> un = D/2   --> u = 1/2
        j = 2 --> un = D/4   --> u = 1/4
        j = 3 --> un = D/8   --> u = 1/8
        j = 4 --> un = D/16  --> u = 1/16
        ...

        Note that we must find the unique "simplest" u:
            lowest j, or if j == 0, the u with lowest absolute value
    */
    const int denominator = x.bottom();
    const int width = y.top() - x.top();
    assert(width > 1);
    int un = denominator;

    for (int j = 0; ; j++)
    {
        assert(is_power_of_2(un));

        // Check if u exists at this j by testing if un occurs between (xn % un) and ((xn % un) + width)
        int xn2 = x.top();
        {
            bool success = safe_pow2_mod(xn2, un);
            assert(success);
        }
        int yn2 = xn2 + width;

        if (xn2 < un && un < yn2) // found j
        {
            const int relative = un - xn2;

            int zn = x.top() + relative;
            int zd = denominator;
            fraction z(zn, zd);
            z.simplify();

            if (swapped)
                z.negate();

            map_view.add_game(new dyadic_rational(z));
            return true;
        }

        assert((un & 0x1) == 0);
        un >>= 1;
    }
}
} // namespace

////////////////////////////////////////

void simplify_basic_all(sumgame_map_view& map_view)
{
    simplify_basic_nimber(map_view);
    simplify_basic_switch(map_view);
    simplify_basic_up_star(map_view);
    simplify_basic_integers_rationals(map_view);
};

void simplify_basic_nimber(sumgame_map_view& map_view)
{
    vector<game*>* nimbers = map_view.get_games_nullable(game_type<nimber>());

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

void simplify_basic_switch(sumgame_map_view& map_view)
{
    vector<game*>* switch_games = map_view.get_games_nullable(game_type<switch_game>());

    if (switch_games == nullptr)
    {
        return;
    }

    // Sort the switches by kind
    vector<switch_game*> proper_switches;
    vector<switch_game*> number_switches;

    vector<game*> consumed_switches;

    for (game* g : *switch_games)
    {
        switch_game* g_switch = cast_game<switch_game*>(g);
        switch (g_switch->kind())
        {
            case SWITCH_KIND_PROPER:
            {
                proper_switches.push_back(g_switch);
                break;
            }
            case SWITCH_KIND_CONVERTIBLE_NUMBER:
            {
                number_switches.push_back(g_switch);
            }
            case SWITCH_KIND_RATIONAL:
            case SWITCH_KIND_PROPER_NORMALIZED:
            {
                continue;
            }
        }
    }

    // Normalize proper switches
    for (switch_game* g_switch : proper_switches)
    {
        assert(g_switch->kind() == SWITCH_KIND_PROPER);

        fraction f1 = g_switch->left();
        fraction f2 = g_switch->right();

        // Compute mean, then subtract it
        fraction mean = f1;
        if (                                                //
            !fraction::safe_add_fraction(mean, f2)       || // mean = (f1 + f2)
            !mean.mul2_bottom(1)                         || // mean = (f1 + f2) / 2
            !fraction::safe_subtract_fraction(f1, mean)  || // f1 = f1 - mean
            !fraction::safe_subtract_fraction(f2, mean)     // f2 = f2 - mean
        )                                                   //
            continue;

        mean.simplify();
        f1.simplify();
        f2.simplify();

        consumed_switches.push_back(g_switch);
        if (mean.top() != 0)
        {
            dyadic_rational* new_rational = new dyadic_rational(mean);
            map_view.add_game(new_rational);
        }

        switch_game* new_switch = new switch_game(f1, f2);
        map_view.add_game(new_switch);
    }

    // Convert number switches
    for (switch_game* g_switch : number_switches)
    {
        assert(g_switch->kind() == SWITCH_KIND_CONVERTIBLE_NUMBER);

        const fraction& f1 = g_switch->left();
        const fraction& f2 = g_switch->right();

        if (convert_number_switch(f1, f2, map_view))
            consumed_switches.push_back(g_switch);
    }

    map_view.deactivate_games(consumed_switches);
}

void simplify_basic_up_star(sumgame_map_view& map_view)
{
    vector<game*>* up_stars = map_view.get_games_nullable(game_type<up_star>());

    if (up_stars == nullptr)
    {
        return;
    }

    int ups = 0;
    bool star = false;

    vector<game*> consumed_games; // if an addition will overflow, not all up_stars will be consumed

    for (game* g : *up_stars)
    {
        up_star* g_up_star = cast_game<up_star*>(g);

        if (!safe_add_negatable(ups, g_up_star->num_ups()))
        {
            continue;
        }

        star ^= g_up_star->has_star();
        consumed_games.push_back(g_up_star);
    }

    if (consumed_games.size() < 2 && (ups != 0 || star != false))
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
    vector<game*>* integers = map_view.get_games_nullable(game_type<integer_game>());
    vector<game*>* rationals = map_view.get_games_nullable(game_type<dyadic_rational>());

    vector<game*> consumed_integers;
    vector<game*> consumed_rationals;

    // add up integers
    int int_sum = 0;
    if (integers != nullptr)
    {
        for (game* g : *integers)
        {
            integer_game* g_integer = cast_game<integer_game*>(g);

            if (!safe_add_negatable(int_sum, g_integer->value()))
            {
                continue;
            }

            consumed_integers.push_back(g_integer);
        }
    }

    // add up rationals
    fraction rational_sum(0);
    if (rationals != nullptr)
    {
        for (game* g : *rationals)
        {
            dyadic_rational* g_rational = cast_game<dyadic_rational*>(g);
            fraction f(*g_rational);

            if (!fraction::safe_add_fraction(rational_sum, f))
            {
                continue;
            }

            consumed_rationals.push_back(g_rational);
        }
    }

    size_t n_consumed_games = consumed_integers.size() + consumed_rationals.size();

    fraction final_sum(int_sum, 1);
    bool final_sum_valid = fraction::safe_add_fraction(final_sum, rational_sum);

    bool integer_sum_useless = (consumed_integers.size() < 2 && int_sum != 0);
    bool rational_sum_useless = (consumed_rationals.size() < 2 && rational_sum.top() != 0);
    bool final_sum_useless = !final_sum_valid || (n_consumed_games < 2 && final_sum.top() != 0);


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
    if (!final_sum_useless)
    {
        assert(final_sum_valid);
        map_view.deactivate_games(consumed_integers);
        map_view.deactivate_games(consumed_rationals);
        insert_game(final_sum.top(), final_sum.bottom());
        return;
    }

    if (!integer_sum_useless)
    {
        map_view.deactivate_games(consumed_integers);
        insert_game(int_sum, 1);
    }

    if (!rational_sum_useless)
    {
        map_view.deactivate_games(consumed_rationals);
        insert_game(rational_sum.top(), rational_sum.bottom());
    }
}
