#include "bounds.h"
#include "cgt_up_star.h"
#include "cgt_dyadic_rational.h"
#include "clobber_1xn.h"
#include "sumgame.h"
#include "nogo_1xn.h"
#include "utilities.h"

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
    void _step(search_region& region, bound_scale scale, sumgame& sum, game_bounds& bounds);
    relation _get_step_comparison(sumgame& sum, game* inverse_scale_game, bool below_midpoint, int* solve_count);

    bool _g_less_or_equal_s(sumgame& sum, game* inverse_scale_game);
    bool _g_less_or_equal_s(sumgame& sum, bound_t scale_idx, bound_scale scale);

    bool _g_greater_or_equal_s(sumgame& sum, game* inverse_scale_game);
    bool _g_greater_or_equal_s(sumgame& sum, bound_t scale_idx, bound_scale scale);

    bool _validate_interval(bound_t min, bound_t max, bound_scale scale, sumgame& sum, game_bounds& bounds);
    void _refine_bounds(bound_scale scale, game_bounds& bounds, sumgame& sum);


    bool _assume_below_midpoint;
    int _step_count;
    int _search_count;

    vector<search_region> _regions;
    vector<search_region> _regions_next;

    friend void test_bounds();

};

//////////////////////////////////////// helper functions

namespace {

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

} // namespace

//////////////////////////////////////// bound_scale functions
game* get_scale_game(bound_t scale_idx, bound_scale scale)
{
    switch (scale)
    {
        case BOUND_SCALE_UP_STAR:
        {
            return new up_star(scale_idx, true);
            break;
        }

        case BOUND_SCALE_UP:
        {
            return new up_star(scale_idx, false);
            break;
        }

        case BOUND_SCALE_DYADIC_RATIONAL:
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

game* get_inverse_scale_game(bound_t scale_idx, bound_scale scale)
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

    if (lower_valid())
    {
        assert(lower >= _lower);
    }

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


    if (upper_valid())
    {
        assert(upper <= _upper);
    }

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

    assert(!addition_wraps(_lower, _upper));
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

    assert(!addition_wraps(low, high));
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
        // TODO consider tweaking this later (2 --> 3 ???)
        if (!bounds->both_valid() && !validated_interval && _step_count >= 2)
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

void bounds_finder::_step(search_region& region, bound_scale scale, sumgame& sum, game_bounds& bounds)
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

    if (did_tie_break && sumgame_solve_count > 1 && rel != REL_FUZZY)
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

    return relation_from_search_results(le_known, is_le, ge_known, is_ge);
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

bool bounds_finder::_g_less_or_equal_s(sumgame& sum, bound_t scale_idx, bound_scale scale)
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

bool bounds_finder::_g_greater_or_equal_s(sumgame& sum, bound_t scale_idx, bound_scale scale)
{
    unique_ptr<game> inverse_scale_game(get_inverse_scale_game(scale_idx, scale));
    return _g_greater_or_equal_s(sum, inverse_scale_game.get());
}

bool bounds_finder::_validate_interval(bound_t min, bound_t max, bound_scale scale, sumgame& sum, game_bounds& bounds)
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

void bounds_finder::_refine_bounds(bound_scale scale, game_bounds& bounds, sumgame& sum)
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
vector<game_bounds*> find_bounds(sumgame& sum, const vector<bounds_options>& options)
{
    bw old_to_play = sum.to_play();

    bounds_finder bf;
    vector<game_bounds*> bounds_list = bf.find_bounds(sum, options);

    sum.set_to_play(old_to_play);
    return bounds_list;
}

vector<game_bounds*> find_bounds(vector<game*>& games, const vector<bounds_options>& options)
{
    sumgame sum(BLACK);
    sum.add(games);
    return find_bounds(sum, options);
}

vector<game_bounds*> find_bounds(game* game, const vector<bounds_options>& options)
{
    sumgame sum(BLACK);
    sum.add(game);
    return find_bounds(sum, options);
}

void test_bounds()
{
    /*
    vector<string> cases {
        "XOOOOX..XXXOXXXOXO.OX.XO",
        ".OXXXOOOOXXOOX.XOXX.O.XX",
        ".OXXXXXX.XOOOXOXOO.X.OOX",
        "XOXX.OXOOXXOX.OXXOX.XO.O",
        "OXXXOXXOOX.X.OX.O.OXXOXO",
        "XOOOXXOXOXO.O.XXXOXOXX..",
        ".OOOXXOXXO.XX.XXOOOXX.XO",
        "O.X.OXOX.XOXXX.XOXXXOOOO",
        "X.OOOXXOOXOOXX.XXOO.X.XX",
        "XXXXO.XX.OO.OOOXO.XXOXOX",
        ".XXOOXXOXO.OXOOOXOX.X.XX",
        "OOXX..X.XXXXXOXOOXOOOO.X",
        ".XOOOXX.OXXO.OXOX.XOXXXO",
        "O..XXXXXOXOOOXOXXXO..XOO",
        "OXXOXOOO.OXXXOOX.OX.X.XX",
        "XOXOOXO.XXOO.XX.O.OXXXXO",
        "OX.OX.OXXXXOO.XX.OOOXXOX",
        "XOXX.OOXOXX.XXX.O.XOXOOO",
        ".OXXOOOOOXXXXOXX.X.XOOX.",
        "XOOOX..XOXOOX.XXXOOXXXO.",
        ".XO.XXXXOX.XXOO.XOOOOXXO",
        "OXXXXOXOXOX.OXO.X.XO.OOX",
        "XO.XOXOXOOOOX.XXX.X.XXOO",
        "XX.XXOOXXOXO.OXXXOOO.XO.",
        ".X.OO.OOXXOXXXXOOXXXX.OO",
        "XO.OX.XXOOOXOO.XXX.XOOXX",
        "XOXXXX.OXOXOOOX.OX.XOOX.",
        "OXOOXXXOO.XX.XXXOXOOXO..",
        "OOOOOO..X.XOO.XOXXXXXXXX",
        "OOXOXOOXX.X.OOXXX.OXXXO.",
        ".XOOOOOX.O.XXXXOOXXXXOX.",
        "X.XOX.XXOOOO.OXOOX.OXXXX",
        "XO.OOXXXX.OXXOX.OX.OXXOO",
        "..OXXXXOXXOXX.XOO.OOXOOX",
        "OX.X.XO.OOXXXOXOOXO.XXXO",
        ".OXXOX.XXOOOOX.XX.XOXXOO",
        "OOXOXO.XO.XXOX.O.XXOXXXO",
        "OOXOOX.X.XOXXXX.OOX.XOOX",
        "OOX.OOOXXOXXO..XOXXX.OXX",
        "OXO.XX.OXXXOO.OX.OXOXOXX",
        "OOXXOX.OXXXXOO.OXXOXOX..",
        "OXXO.XXXXXOOOXXOOX.OX..O",
        "OOOOXXX.O.XO.XXXXOOXO.XX",
        "XXX.O.XXXOOOOX.XOXXXOOO.",
        ".OO.OXXOXX.XX.XXOOXOXOOX",
        "XOXXO..OOOOOXXXOXX.XXO.X",
        "OXX.XOXXXOO.XXOOXXX.OOO.",
        "OOOXX.OXO.OXXOXOX.XXX.XO",
        "XOO.XOXXXOX..XOX.OXXOXOO",
        "OXXOOXXXXX.XOO..OOOXXX.O",
        "O..XXXOX.OXXOOXOXOOX.XXO",
        ".OXOXOO.XO.XOXXXXX.OOXXO",
        "OOXOO.X..XXXOXXOXOXXXO.O",
        "OX.OOXXXXOOOX.OXO.XXXX.O",
        "OOOO.X.OOXXXXXX..OOOXXXX",
        ".XOXXXXXO.XXOOXOOXO.XO.O",
        "XXOOXXX..OXXOOXOXO.O.OXX",
        "XXOXXO..XXOOXOOX.OOXXX.O",
        "XOXO.XOXXOXOXXXO..O.XOXO",
        "OXXOOXO.XOXXO.OXOOXXX.X.",
        "XOOX.X.XOOXX.OOXO.XOXXXO",
        "XO..OXXOOXOOOXXX.XOX.XOX",
        ".XXOOOXOXXOXXX.OXO.XOO.X",
        "OXXOXOOXOO.XXXXO...XXOXO",
        "OOXXXXX.X.X.OOXO.OXXXOOO",
        "OX.OXXOO.XOOXXOXX.X.XOOX",
        ".OXXOOXXX..OXO.XXOXOOXOX",
        "OXXXOXOOXXXXOO.O..XOOXX.",
        "XXOX.O.XOXOXXX.O.OOXXXOO",
        "X.XOOXOXOXXXOX.OOOXOX..X",
        "XOO.XXOXXXXO.OOO.XOX.XOX",
        ".O.OXXXOXOOOXX.OOXXXX.XO",
        "OO.XXOXOXX.XOXOXXXO.OOX.",
        "XOX.XXOX..OXO.OOXXXOXOOX",
        ".OXXXOXOX.OXX.OO.OOXOXXX",
        "XOX.XOXXOOXX.XOOX.XOXOO.",
        "XXXXOOX..OXOOOXX.XOOXOX.",
        "OOXXOXXXOO...OX.OXXXOOXX",
        "X.OX.XOOXXXX..OXXOOOOXOX",
        "OXXOXX.XOOX..XOOXOO.XOXX",
        "OXOO.X.XOOXXXXXOXX.OOO.X",
        "OXXO.XOOOOOXX..XOXXO.XXX",
        "XOOOXXO..OOOXXO.XOXXX.XX",
        "X..XOXXOXOOOOOXOXXX..XOX",
        "XOXO..OOXXXOXXO.X.OXXXOO",
        "OXXXXXOOOXX.XO.O.X.OXOXO",
        "OOO..OXOX..XXOXXXOOXXXXO",
        "OOOXOX...O.OXXXXXXXXOOOX",
        "XOOXXO.OOX.OX..XOXXXOOXX",
        "XX.OOOOXX.OOXXXX..XOOXOX",
        "O.X.XOXOOXOXXXOOXXX.X.OO",
        "XXX.X.O..XXOOOXOXXOOOXOX",
        "X.OXXX.X.XXOOXOOOO.XXXOO",
        "XOXXOOXX.XOXXXOOX...OXOO",
        "XOXO.OXOXX.X.X.OOXOXXXOO",
        "XX.XO.XOXOOXOXOO.XXOXXO.",
        ".XXXOOXOX..XOXXXOO.OOOXX",
        "XO.XXOXO.OXXOOXO.XOXOXX.",
        "XXOXXO..OOXXXXOOOO.OXXX.",
        ".OXOXOXXXOO.XXOO.XXOO.XX",
    };
*/


    vector<string> cases {
        "XXOXOOOOO.XO.OXOXXX.O.XX",
        "XO.XO.XXXOO.OXX.XOOXOXOO",
        "XOXO.OOOXXO.XOXOOX.XOX.X",
        "XOXOOXOO.XOOXX.OX..OXXOX",
        "OX.XXO.O.OX.XOXOXXXXOOOO",
        "O.XOXOXOXXXOXXO.OOXX..OO",
        "OOOXXXXO.XXOOO.OO..XOXXX",
        ".XXOXXOOOXXX.OXO.OOO.OXX",
        ".XXOOOXOO.XOOOXXX.XOO.XX",
        "O..OX.OOOXXXOOXOXOOXX.XX",
        "OOXOXO.XOXX.O.OOOXXXXXO.",
        "XXOXXXOXXOX..OX.O.OOOOXO",
        "XXXX.OOOX.OO.XXOXOX.XOOO",
        ".OXXX.XXOOOXOOOXOXOOXX..",
        "OXOOO.XXXXX.XX.XO.OXOOOO",
        "XOOX.X.OXXXX.XX.OOOOOOOX",
        "X.OOOXXXXOOOO...OXXOXOXX",
        "OO.X.XOXXXXXOOOOXXO.OXO.",
        "XOO.OOXOXXO.XXOOX.XX.OXO",
        "OOOX.XOXXOXOXX.OOX.X.OXO",
        "OXXX.OOXOXOOXOOX.X.OX.OX",
        "O.OOOX..OOOXXXX.XXOXXOXO",
        "XOXXXX.XOOXXXOOOOOOX.O..",
        "OXXO..OXXOOOXXOXXO.XO.OX",
        "OOOXX.XOOO..XXXOXX.OOXOX",
        "XXOOXOXXOXOO.OO..OXXOXX.",
        "XOXOOOOXXX.XO.OO.XXO.OXX",
        "XO.XXX.XXOXOOOOXOOX.OO.X",
        "O.X.XOXOOOXXOXO.OOXOXXX.",
        "OOXO..XXOXOXOXOOOX.XXX.O",
        ".XXXOO.X.XOX.OOOXOOOXXOX",
        ".OXXOXOXX.OXOOX.OXXXO.OO",
        "OXXXO.XOOXOO.X.XOOXOXO.X",
        "O.OXOOO..OXOXXOXOX.XOXXX",
        "X.XO.OXXOOXO.XXOOO.OXOXX",
        "..XXOO.OOOXX.OXXXXOOXXOO",
        ".OOXXXXXO.XXO.XOXOOOXOO.",
        "XO.OOXX.O.OOXXXOXOOX.OXX",
        "O.OXOXXO.XOO.XXXXOO.OXXO",
        "XX.XOX..OOXOX.XOOXOOXXOO",
        "XO.O.XOO.OX.OXXXXOOXXXOO",
        "XXXO.OOXOXX.X.OXXOOOOO.X",
        "XOXO.XXX.O.XXXOXOOXOO.OO",
        "OXOX.OX.X.X.OOXOOOXOXOXX",
        "OXXXXOO.X.OOOXXXOOOOX.X.",
        "X.XXO.XOXOOOOX.OX.XXOXOO",
        "XXXO.XOOOXOOXO.OXXOXX.O.",
        "OXXXOXXX..XOOXO.XO.OOOXO",
        "X.XX.OOOOXXXOOXX.XOOOXO.",
        "XO..X.OXOXOO.XXXXOOOXXOO",
        "X.OOOXXOOO.XXOXOX.XOX.XO",
        "OXXXOX.XOXOO.OXX.XO.XOOO",
        "OOXOXOXX.XXXXOX.OOOO.O.X",
        ".XOOOOXXOOOXXXOXO..XX.XO",
        ".XOOOOXX.OX.XXOX.OXXXOOO",
        "O.OXXOXXXOOOO.OXOXOXX..X",
        "O.XX.XOOOO.XXOX.XOXOOOXX",
        "O..XXOXOO.OXXXOXO.XOXXOO",
        "XXOOO.XOOX..OXXXXOO.XOXO",
        "XXOXO.XXOO.XXO.OOXOXOX.O",
        "XX.OXXX.X.OO.XOXOOOOXOXO",
        "XOOOXOXOOXX.X.XXOOOOXX..",
        ".XOX.XXO.XOO.OXXOXOOOOXX",
        "OOX.XOXXOXXXOO.O..XOOXOX",
        "OXXXO.OOXOXXO.OOO.OXXX.X",
        "X.OOXXXOOXXOOOOX.O.XXX.O",
        "XXOXOX..XXOOXO.OO.XXOXOO",
        "O.OXOXOXXOXXO.OO..XXXOOX",
        "OOOXOOXX.OXXXXX..XOOOO.X",
        "OOO.XOX.XOOXXXX.XOX.XOOO",
        "XOXOOXX.OXXXOOOOOX.XOX..",
        "O.XXOXX.XOXOOXX.OOOX.XOO",
        "OO..X.XOXXXXX.OOOOOXOOXX",
        "OOXOOOXOX.XOXXXOX.XOOX..",
        "XXO..XOXXOXOOXOOOOO.XXX.",
        ".XXOXO.XOXXO.OOXXOXO.XOO",
        ".O.OXXOOOXOXOXXXOXX..OXO",
        "OXXOO...XOXOOXOOXXOX.OXX",
        "OOXOOXOOO.XXOX.XX..XXXOO",
        "XOOOOOO.XXX.XXOOOX.OXX.X",
        "OX.XXXOO..XXOOOXXO.OXXOO",
        "OXOXOXO..XXO.OOXOXXXX.OO",
        "XX.XX.O.OXXOOXXOOXO.OOXO",
        ".O.XOX.XXXOXOX.XOXOOOXOO",
        ".XOOXXXO.OOOOXXX.XO.OXOX",
        "X.XXO.OXOXXXOOXOO..OOXXO",
        "XXXX.XO.OOOXOOOX.XX.OOXO",
        "XO.XOOO.OXOXXXO.O.XOXXOX",
        "O..OXOXOOXX.OOXXO.OXXOXX",
        "O.XXOXXX..OOOXX.OXXOOOOX",
        "XOOX.OXXO.OO.XXO.OXXXXOO",
        "XXOXXO.OOO.O..XOXOXXOXOX",
        "XOXOXO.OOOX.XOXX..XXOXOO",
        "XOO.XOXOOXXO.XXOX.OXOO.X",
        "OOXXXOXXOOOO..XOX..XXOOX",
        "OXX.OXOOOOXO.XXXXOO.XO.X",
        ".XOOXXO.OOO.OOOXXXXX.XXO",
        "OXOXXXOOO.XXX..XOXOO.OXO",
        "OXOX..XOOOXX.XO.OOXOOXXX",
        "OXOXX.XOOX.OXO.OXXXOXO.O",
        "XXOXX.OOX.OXO.OXOXX.OOXO",
        "OX..XXXXOOOOOO.OXXXOXXO.",
        "OOOXOO.XOX.XOXXXOX.X.OOX",
        "OOXO.OOXOOX.XXXXO.OXXXO.",
        "OOO..OOXOX.XXX.XOOXOXXOX",
        "OX.XOXOOXOXX.OOX.OXO.OXX",
        "..XXOXOO.XOOXOOOXOXX.XOX",
        "XXOO.OOXXXXXO.O..XXXOOOO",
        "O.XXXXXXXX..OOXOX.OOOOOO",
        "OOXOX.O.X.OOXXXXOOXOXO.X",
        "XOOXOO.XOX.OXXXOXOXX.O.O",
        "XOXXOO.OO.X.XX.OOXXOOXOX",
        "XOOXXXOXXOXXO.XOOO..OO.X",
        "XXO.OXXXOXOOXXOXOXO...OO",
        "XO.XO.XOXO.XOX.OOOOXXXXO",
        "XXOO.O.OO.XXOXXOOOXXOX.X",
        "OOOXOXOX.XOOO...XXOXXOXX",
        "XXOXOOOOOOOXXOXXXX..X.O.",
        "OXOXXXOXXOO..OOXXX.X.OOO",
        ".X..OOOOO.XXXXOOXXXOXOXO",
    };

    //cases.resize(1);

    //cases = {
    //    "..."
    //};

    int total = 0;
    for (const string& str : cases)
    {
        //game* g = new nogo_1xn(str);
        game* g = new clobber_1xn(str);
        //game* g = new up_star(1, false);
        sumgame sum(BLACK);
        sum.add(g);


        cout << *g << endl;

        bounds_finder bf;
        vector<game_bounds*> bounds_list = bf.find_bounds(sum, {{BOUND_SCALE_UP, -32, 32}});
        total += bf._step_count;
        cout << bf._step_count << endl;
        cout << endl;


        delete g;
    }

    cout << "Total: " << total << endl;


}
