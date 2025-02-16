#include "bounds2.h"
#include "cgt_up_star.h"
#include "cgt_dyadic_rational.h"
#include "clobber_1xn.h"
#include "sumgame.h"

#include <iostream>
#include <limits>
#include <cassert>
#include <memory>

using namespace std;

//////////////////////////////////////// Type declarations
class search_region
{
public:
    // inclusive interval
    search_region(bound_t low, bound_t high);

    // *this becomes lower half, returns upper half (both exclude midpoint)
    search_region split(bound_t midpoint);

    bool valid() const;
    void invalidate();
    bound_t get_midpoint() const;

    bound_t low;
    bound_t high;
};

class bounds_finder
{
public:
    bounds_finder();

    vector<game_bounds*> find_bounds(sumgame& sum, const vector<bounds_options>& options);

private:
    void _reset();
    void _flip_tie_break_rule();

    game_bounds* _make_bounds(sumgame& sum, const bounds_options& opt);
    void _step(search_region& region, game_scale scale, sumgame& sum, game_bounds& bounds);
    relation _get_step_comparison(sumgame& sum, game* inverse_scale_game, bool below_midpoint, int* solve_count);

    bool _g_less_or_equal_s(sumgame& sum, game* inverse_scale_game);
    bool _g_less_or_equal_s(sumgame& sum, bound_t scale_idx, game_scale scale);

    bool _g_greater_or_equal_s(sumgame& sum, game* inverse_scale_game);
    bool _g_greater_or_equal_s(sumgame& sum, bound_t scale_idx, game_scale scale);

    bool _validate_interval(bound_t min, bound_t max, game_scale scale, sumgame& sum, game_bounds& bounds);
    void _refine_bounds(game_scale scale, game_bounds& bounds, sumgame& sum);


    bool _assume_below_midpoint;
    int _step_count;
    int _search_count;

    vector<search_region> _regions;
    vector<search_region> _regions_next;

    friend void test_bounds2(); // TODO remove me
};

//////////////////////////////////////// helper functions
relation get_relation_from_outcomes(bool le_known, bool is_le, bool ge_known, bool is_ge)
{
    // Only handles the cases that should occur during bounds search

    assert(le_known || ge_known);

    if (le_known && ge_known)
    {
        if (!is_le && !is_ge) // 0 0
            return REL_FUZZY;
        if (!is_le && is_ge) // 0 1
            return REL_GREATER;
        if (is_le && !is_ge) // 1 0
            return REL_LESS;
        if (is_le && is_ge) // 1 1
            return REL_EQUAL;

        assert(false);
    }

    if (le_known && is_le)
    {
        assert(!ge_known);
        return REL_LESS_OR_EQUAL;
    }

    if (ge_known && is_ge)
    {
        assert(!le_known);
        return REL_GREATER_OR_EQUAL;
    }

    assert(false);
}

bool prune_region(const search_region& sr, const game_bounds& bounds)
{
    if (!sr.valid())
    {
        return true;
    }

    // regions don't overlap, so they shouldn't need to be "clipped";
    // either the entire region is OK, or the entire region is outside the bounds

    if (bounds.lower_valid() && (bounds.get_lower() > sr.high))
    {
        return true;
    }

    if (bounds.upper_valid() && (bounds.get_upper() < sr.low))
    {
        return true;
    }

    return false;
}

//////////////////////////////////////// game_scale functions
game* get_scale_game(bound_t scale_idx, game_scale scale)
{
    switch (scale)
    {
        case GAME_SCALE_UP_STAR:
        {
            return new up_star(scale_idx, true);
            break;
        }

        case GAME_SCALE_UP:
        {
            return new up_star(scale_idx, false);
            break;
        }

        case GAME_SCALE_DYADIC_RATIONAL:
        {
            return new dyadic_rational(scale_idx, 8);
            break;
        }

        default: 
        {
            assert(false);
            return nullptr;
            break;
        }
    }
}

game* get_inverse_scale_game(bound_t scale_idx, game_scale scale)
{
    return get_scale_game(-scale_idx, scale);
}

//////////////////////////////////////// game_bounds
game_bounds::game_bounds(): 
    _lower(numeric_limits<bound_t>::max()), _lower_valid(false), _lower_relation(REL_FUZZY),
    _upper(numeric_limits<bound_t>::min()), _upper_valid(false), _upper_relation(REL_FUZZY)
{ }

void game_bounds::set_lower(bound_t lower, relation lower_relation)
{
    assert(lower_relation == REL_LESS
            || lower_relation == REL_LESS_OR_EQUAL 
            || lower_relation == REL_EQUAL
    );

    _set_lower(lower, lower_relation);

    if (lower_relation == REL_EQUAL)
    {
        _set_upper(lower, REL_EQUAL);
    }

    if (upper_valid())
    {
        assert(_lower <= _upper);
    }
}

void game_bounds::set_upper(bound_t upper, relation upper_relation)
{
    assert(upper_relation == REL_GREATER
            || upper_relation == REL_GREATER_OR_EQUAL 
            || upper_relation == REL_EQUAL
    );

    _set_upper(upper, upper_relation);

    if (upper_relation == REL_EQUAL)
    {
        _set_lower(upper, REL_EQUAL);
    }

    if (lower_valid())
    {
        assert(_lower <= _upper);
    }
}

void game_bounds::set_equal(bound_t lower_and_upper)
{
    _set_lower(lower_and_upper, REL_EQUAL);
    _set_upper(lower_and_upper, REL_EQUAL);

    assert(_lower_valid && _upper_valid);
    assert(_lower_relation == REL_EQUAL && _upper_relation == REL_EQUAL);
    assert(_lower == _upper);
}

bound_t game_bounds::get_midpoint() const
{
    assert(both_valid());

    // TODO Check for overflow?
    return (_lower + _upper) / 2;
}

void game_bounds::invalidate_lower()
{
    _lower_valid = false;
}

void game_bounds::invalidate_upper()
{
    _upper_valid = false;
}

void game_bounds::invalidate_both()
{
    _lower_valid = false;
    _upper_valid = false;
}

void game_bounds::_set_lower(bound_t lower, relation lower_relation)
{
    _lower = lower;
    _lower_relation = lower_relation;
    _lower_valid = true;
}

void game_bounds::_set_upper(bound_t upper, relation upper_relation)
{
    _upper = upper;
    _upper_relation = upper_relation;
    _upper_valid = true;
}

ostream& operator<<(ostream& os, const game_bounds& gb)
{
    // opening brace, lower bound
    if (gb.lower_valid())
    {
        switch (gb.get_lower_relation())
        {
            case REL_LESS_OR_EQUAL:
			{
                os << '[';
				break;
			}

            case REL_LESS:
			{
                os << '(';
				break;
			}

            case REL_EQUAL:
			{
                os << "==";
				break;
			}

            default:
			{
                assert(false);
                os << "[?!";
				break;
			}
        }
        os << gb.get_lower() << " ";
    } else
    {
        os << "[? ";
    }

    // upper bound, closing brace
    if (gb.upper_valid())
    {
        os << gb.get_upper();

        switch (gb.get_upper_relation())
        {
            case REL_GREATER_OR_EQUAL:
			{
                os << ']';
				break;
			}

            case REL_GREATER:
			{
                os << ')';
				break;
			}

            case REL_EQUAL:
			{
                os << "==";
				break;
			}

            default:
			{
                assert(false);
                os << "?!]";
				break;
			}
        }
    } else
    {
        os << "?]";
    }

    return os;
}

//////////////////////////////////////// search_region
search_region::search_region(bound_t low, bound_t high): low(low), high(high)
{ }

search_region search_region::split(bound_t midpoint)
{
    assert(valid());
    assert(low <= midpoint);
    assert(midpoint <= high);

    bound_t old_high = high;
    high = midpoint - 1;

    return search_region(midpoint + 1, old_high);
}

bool search_region::valid() const
{
    return low <= high;
}

void search_region::invalidate()
{
    low = numeric_limits<bound_t>::max();
    high = numeric_limits<bound_t>::min();
}

bound_t search_region::get_midpoint() const
{
    assert(valid());

    // TODO check for overflow?
    return (low + high) / 2;
}

//////////////////////////////////////// bounds_finder
bounds_finder::bounds_finder()
{ }

vector<game_bounds*> bounds_finder::find_bounds(sumgame& sum, const vector<bounds_options>& options)
{
    vector<game_bounds*> bounds_list;

    for (const bounds_options& opts : options)
    {
        assert(opts.min <= opts.max);

        _reset();

        game_bounds* gb = _make_bounds(sum, opts);
        bounds_list.push_back(gb);
    }

    return bounds_list;
}

void bounds_finder::_reset()
{
    _assume_below_midpoint = true;
    _step_count = 0;
    _search_count = 0;

    _regions.clear();
    _regions_next.clear();
}

void bounds_finder::_flip_tie_break_rule()
{
    _assume_below_midpoint = !_assume_below_midpoint;
}

game_bounds* bounds_finder::_make_bounds(sumgame& sum, const bounds_options& opt)
{
    _regions.clear();
    _regions_next.clear();
    _regions.push_back({opt.min, opt.max});

    game_bounds* bounds = new game_bounds();
    bool validated_interval = false; // true when we've checked that Gmin <= S <= Gmax

    while (!_regions.empty())
    {
        _regions_next.clear();

        for (search_region& sr : _regions)
        {
            // Skip if this region is invalid or outside of bounds
            if (prune_region(sr, *bounds))
            {
                continue;
            }

            // stop if no more work remaining
            if (bounds->lower_valid() && bounds->get_lower_relation() == REL_EQUAL)
            {
                _regions_next.clear();
                break;
            }

            // Do one step of binary search within the region
            _step(sr, opt.scale, sum, *bounds);
        }

        // verify that Gmin <= S <= Gmax if this isn't known after a few steps
        // TODO reduce threshold 3 --> 2? leave at 3?
        if (!bounds->both_valid() && !validated_interval && _step_count >= 3)
        {
            validated_interval = true;

            if (!_validate_interval(opt.min, opt.max, opt.scale, sum, *bounds))
            {
                break;
            }
        }

        swap(_regions, _regions_next);
    }

    if (bounds->both_valid())
    {
        _refine_bounds(opt.scale, *bounds, sum);
    }

    return bounds;
}

void bounds_finder::_step(search_region& region, game_scale scale, sumgame& sum, game_bounds& bounds)
{
    // S refers to sumgame
    // Gi refers to scale game

    _step_count++;
    assert(region.valid());

    int scale_idx = region.get_midpoint(); // i
    unique_ptr<game> inverse_scale_game(get_inverse_scale_game(scale_idx, scale)); // -Gi
 
    // Pick a test (>= or <=) to try first, based on what side of the bounds space
    // "scale_idx" is on
    bool below_midpoint = _assume_below_midpoint;
    bool did_tie_break = true;

    int midpoint = 0;

    if (bounds.both_valid())
    {
        midpoint = bounds.get_midpoint();
    }

    if (scale_idx != midpoint)
    {
        did_tie_break = false;
        below_midpoint = scale_idx < midpoint;
    }


    // Gi <relation> S
    int sumgame_solve_count = 0;
    relation rel = _get_step_comparison(sum, inverse_scale_game.get(), below_midpoint, &sumgame_solve_count);

    if (did_tie_break && sumgame_solve_count > 1)
    {
        _flip_tie_break_rule();
    }

    switch (rel)
    {
        // Gi >= S
        case REL_GREATER_OR_EQUAL:
        case REL_GREATER:
        {
            region.high = scale_idx - 1;
            bounds.set_upper(scale_idx, rel);
            break;
        }

        // Gi <= S
        case REL_LESS_OR_EQUAL:
        case REL_LESS:
        {
            region.low = scale_idx + 1;
            bounds.set_lower(scale_idx, rel);
            break;
        }

        // Gi fuzzy with S
        case REL_FUZZY:
        {
            _regions_next.push_back(region.split(scale_idx));
            break;
        }

        // Gi == S
        case REL_EQUAL:
        {
            region.invalidate();
            bounds.set_equal(scale_idx);
            break;
        }

        default:
        {
            assert(false);
            break;
        }
    };

    _regions_next.push_back(region);
}

relation bounds_finder::_get_step_comparison(sumgame& sum, game* inverse_scale_game, bool below_midpoint, int* solve_count)
{
    bool le_known = false;
    bool is_le = false;

    bool ge_known = false;
    bool is_ge = false;


    auto test_le = [&]() -> void
    {
        assert(!le_known);
        le_known = true;
        is_le = _g_less_or_equal_s(sum, inverse_scale_game);
    };

    auto test_ge = [&]() -> void
    {
        assert(!ge_known);
        ge_known = true;
        is_ge = _g_greater_or_equal_s(sum, inverse_scale_game);
    };

    auto is_conclusive = [&]() -> bool
    {
        return (le_known && is_le) || (ge_known && is_ge);
    };

    if (below_midpoint)
    {
        test_le();
        if (!is_conclusive())
            test_ge();
    } else
    {
        test_ge();
        if (!is_conclusive())
            test_le();
    }

    // (<= or >=) or FUZZY; No EQUAL
    assert(is_conclusive() || (le_known && ge_known));

    if (solve_count != nullptr)
    {
        *solve_count = (int) le_known + (int) ge_known;
    }

    return get_relation_from_outcomes(le_known, is_le, ge_known, is_ge);
}

bool bounds_finder::_g_less_or_equal_s(sumgame& sum, game* inverse_scale_game)
{
    /*
        B wins   W wins
        ?        0

        S - Gi >= 0
        S >= Gi
        Gi <= S
    */

    _search_count++;
    sum.set_to_play(WHITE);
    return !sum.solve_with_games(inverse_scale_game);
}

bool bounds_finder::_g_less_or_equal_s(sumgame& sum, bound_t scale_idx, game_scale scale)
{
    unique_ptr<game> inverse_scale_game(get_inverse_scale_game(scale_idx, scale));
    return _g_less_or_equal_s(sum, inverse_scale_game.get());
}

bool bounds_finder::_g_greater_or_equal_s(sumgame& sum, game* inverse_scale_game)
{
    /*
        B wins   W wins
        0        ?

        S - Gi <= 0
        S <= Gi
        Gi >= S
    */

    _search_count++;
    sum.set_to_play(BLACK);
    return !sum.solve_with_games(inverse_scale_game);
}

bool bounds_finder::_g_greater_or_equal_s(sumgame& sum, bound_t scale_idx, game_scale scale)
{
    unique_ptr<game> inverse_scale_game(get_inverse_scale_game(scale_idx, scale));
    return _g_greater_or_equal_s(sum, inverse_scale_game.get());
}

bool bounds_finder::_validate_interval(bound_t min, bound_t max, game_scale scale, sumgame& sum, game_bounds& bounds)
{
    if (!bounds.lower_valid())
    {
        unique_ptr<game> inverse_scale_game(get_inverse_scale_game(min, scale)); // -Gmin
        // Gmin <relation> S
        relation rel = _get_step_comparison(sum, inverse_scale_game.get(), true, nullptr);

        if (rel != REL_LESS_OR_EQUAL && rel != REL_LESS && rel != REL_EQUAL)
        {
            return false;
        } else
        {
            bounds.set_lower(min, rel);
        }
    }

    if (!bounds.upper_valid())
    {
        unique_ptr<game> inverse_scale_game(get_inverse_scale_game(max, scale)); // -Gmax
        // Gmax <relation> S
        relation rel = _get_step_comparison(sum, inverse_scale_game.get(), false, nullptr);
        
        if (rel != REL_GREATER_OR_EQUAL && rel != REL_GREATER && rel != REL_EQUAL)
        {
            return false;
        } else
        {
            bounds.set_upper(max, rel);
        }
    }

    return true;
}

void bounds_finder::_refine_bounds(game_scale scale, game_bounds& bounds, sumgame& sum)
{
    if (bounds.lower_valid() && bounds.get_lower_relation() == REL_LESS_OR_EQUAL)
    {
        const bound_t lower = bounds.get_lower();

        bool is_ge = _g_greater_or_equal_s(sum, lower, scale);
        relation rel = is_ge ? REL_EQUAL : REL_LESS;

        bounds.set_lower(lower, rel);
    }

    if (bounds.upper_valid() && bounds.get_upper_relation() == REL_GREATER_OR_EQUAL)
    {
        const bound_t upper = bounds.get_upper();

        bool is_le = _g_less_or_equal_s(sum, upper, scale);
        relation rel = is_le ? REL_EQUAL : REL_GREATER;

        bounds.set_upper(upper, rel);
    }
}


////////////////////////////////////////
inline vector<game_bounds*> find_bounds(sumgame& sum, const vector<bounds_options>& options)
{
    bounds_finder bf;
    return bf.find_bounds(sum, options);
}

inline vector<game_bounds*> find_bounds(vector<game*>& games, const vector<bounds_options>& options)
{
    sumgame sum(BLACK);
    sum.add_vec(games);
    return find_bounds(sum, options);
}

inline vector<game_bounds*> find_bounds(game* game, const vector<bounds_options>& options)
{
    sumgame sum(BLACK);
    sum.add(game);
    return find_bounds(sum, options);
}


////////////////////////////////////////

void test_bounds2()
{
    vector<string> tests
    {

        ".OOXXO.X.XOXX.XXXOOXXOO.O",
"OOOXX.X.X.OOXXOOX.XX.XXOO",
"XXOXXOOXXX...O.X.OXOXXOOO",
"OO.XOXXOXXXXO.O.OXX..OXOX",
"X.XXOXOOXX.OOXXXXO..OO.OX",
"XX.OXOOXXOOXXX..XOOX.OXO.",
"XO..XXOXOOX.OXXXX.OOXXO.O",
"OXX...O.XXOXXO.OXXXXXOOOO",
"OXOXXOXOO.OOXXX.X.XX.OOX.",
"XXOX..OXXXOOOXOXX.OO.XOX.",
"O.XOOOXO..XX.XXOOXX.OXXOX",
"OXXO.XOOX..XXOOXX.OOX.XXO",
"OXOOOX.XOOX.X.OXXXXO..XXO",
"XOOXOXXOXXO.XXO..XXOXO..O",
"OOO..XOXOO.XXXXXXXOX.O.XO",
"XXX..OXOXXOXOOXXOOOX...XO",
"OOXXXXXXOO.XXXXX.OO..OO.O",
"XXOXXOOOX...OOXX.OOXX.XXO",
".OXXOOXOOXXXOXO.XXO...OXX",
"XX.O.XXX.OXOOXXXOXX.OOO.O",
"OXXOOXOOX.X.O..XOX.OXXXXO",
"X.XOO.OOOXXOX.XXXOXXO.O.X",
".OOOXXOXO.XOX.XXXXOXO.OX.",
"XOO.XO.XX.OXOO.OXX.XXOOXX",
"X.OOXOOXXX...XXXXOXO.OOOX",
"XOOXOO.X.OX.OOXXXOXX..OXX",
"XXOX.XXOOOXOXX.XO..X.OOXO",
"OOXXO.XXXXX.OXXO..OO.OXXO",
".X..XOOXOX.XOXO.XOXXXOXOO",
"X.OOOXOXOOXXX.OXXX..X.OXO",
"XOXX.OXXXOO.OXOOXOXOX.X..",
"OXXX..XXXXOOXOOXX..OOOX.O",
".XOXXX.O..OOOOOXX.XXOOXXX",
"XOXOXO..OO..XOXOXXOXXX.XO",
"O..OX.OXOXXOXXXX..OXXOXOO",
"XXOX.X..OOXOOXOOX.OX.XXXO",
"O..OO.XXOOX.XOXXXX.OXOXXO",
"O.XOO.XXOOO..OXX.XXXOOXXX",
"X.XO.OXOXOXOO.X.XOXOXX.XO",
"OXX.OOXOXOXX.X.OOXXOX.O.X",
"O.OXX.X.XOX.OOXXOOXOOX.XX",
"O.XO.XX..XXOXOO.OXXXXOOXO",
".XXXXXOOO.OX.OOO.OX.XXXOX",
"XXXXXOOOX.O.O.X.XO.XOOOXX",
"OXOOOOXX...XXXXOXO.XXO.XO",
"X.OXX.O.XOOXXXOOXOXOXXO..",
"OXX.XXO.XOO..XOOOOX.XXOXX",
".O...XOOXXXXOX.OXOOXXOOXX",
"OOX...XOXOXOXXXOX.OOX.XOX",
"XXXXO.OXOOOXOXX.OO.XXX..O",
"XXOX.XOOXXO.OOXX.OX.X.OXO",
"OX.XXOOOOOOOXX.XOXX.XXX..",
".X.XXXOXXOOXX.OOX.OX.OOXO",
"OXXOOXXO.XOX...XXOX.OXOOX",
"X.OXXOXX.OOXO.OXOXXX.X.OO",
"O.XX.O.XXOXOX.OO.XXOOXXXO",
"OO..XXXX..XXOXX.XOOOOOOXX",
"XXOOXO..XXOXO.XXXOX.OO.XO",
"XO.X.XOXXOXOX.XOXXOOXO..O",
"XOXO..XXXX.OOOOXXXOX..OOX",
"OXXXO.OXXO.OOXX...XXOOXXO",
"XO.OXOXX.XOXXXXOXO.OOOX..",
"OO..XOXXXXX.XXXO.OXOXOO.O",
"XXXO.XXOOXOXO.XO.XXO.XOO.",
"X.O.X.XX.X.OOXOOOXXOOXXOX",
".XX.OXXOOOX.XXOX.OO.XOOXX",
"XOXXXOXXOXOOX...XOO.XO.XO",
"XOXXXOOOOX.XOX.OO..XXXO.X",
"..OOXOOX..XXX.OOXOXOXXOXX",
"OXXXOXO.XOXOOX..XXXOO.O.X",
"XXOO.OXXOX.X.X.XOOXXOXO.O",
"XXX.OOXO..OOXXXXX.OXO.OOX",
"OXXOXO..OXXO.X.O.XOXOXXXO",
"O.O.OXOX..X.OXXXXOXOXXOOX",
"XO.XXOOXOOXXXXXXO.O...XOO",
"OXX.OOOO.X.X.XXXO.OXOXXXO",
"XX.O.XOXOXXOXXOO.OXXOXO..",
"XOOX.OXOXOXO..XXOO.OXXX.X",
".OO.OXXOXO.OXXXXO.XOX.XOX",
"XOX.XOXX.OXOX.OOXOXOX.X.O",



    };



    const bound_t R = 16;


    int total_searches = 0;

    /*
       Clobber tests, UP_STAR

       R = 16
       predict: 760
       assume greater: 908
       assume less: 907

       2022 Clobber: 1126
    */
    for (const string& str : tests)
    {
        game* g = new clobber_1xn(str);

        sumgame sum(BLACK);
        sum.add(g);

        cout << *g << endl;

        bounds_finder bf;
        vector<game_bounds*> bounds_list = bf.find_bounds(sum, {{GAME_SCALE_UP_STAR, -R, R}});

        assert(bounds_list.size() == 1);
        assert(bounds_list[0]->both_valid());

        total_searches += bf._search_count;
        cout << bf._search_count << " " << *bounds_list[0] << endl;


        for (game_bounds* gb : bounds_list)
        {
            delete gb;
        }
    }

    cout << "Total: " << total_searches << endl;

}
