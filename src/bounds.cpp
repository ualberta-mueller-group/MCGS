#include "bounds.h"
#include "cgt_basics.h"
#include "cgt_up_star.h"
#include "cgt_dyadic_rational.h"
#include "sumgame.h"
#include "sumgame_helpers.h"
#include "throw_assert.h"
#include "utilities.h"
#include "safe_arithmetic.h"

#include <iostream>
#include <limits>
#include <cassert>
#include <memory>
#include <vector>
#include <utility>

/*
    Largely untested optimization for infinitesimal games having large confusion
    intervals.

    When enabled, a "fuzzy interval" tracks min/max scale indices `i` for
    which the scale game `Si` is found to be fuzzy compared to the initial
    bounds query game `G`. Search regions are clipped to exclude this fuzzy
    interval.

    It is assumed that for i <= j, if `G <> Si` and `G <> Sj`, then
    for all k, `i <= k <= j`, `G <> Sk`.
*/
#define CLIP_FUZZY_INTERVAL

#define INITIAL_INTERVAL_MAGNITUDE 1
#define REPORT_FUZZY_IN_INITIAL_INTERVAL


using namespace std;

//////////////////////////////////////////////////
std::string bound_scale_to_string(bound_scale scale)
{
    switch (scale)
    {
        case BOUND_SCALE_UP_STAR:
            return "up_star";
        case BOUND_SCALE_UP:
            return "up";
        case BOUND_SCALE_DYADIC_RATIONAL:
            return "dyadic_rational";
    }

    assert(false);
}

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

    vector<game_bounds_ptr> find_bounds(sumgame& sum,
                                        const vector<bounds_options>& options);

private:
    void _reset();
    void _flip_tie_break_rule();

    search_region _find_initial_interval(sumgame& sum,
                                         const bounds_options& opt,
                                         game_bounds* bounds);

    game_bounds* _make_bounds(sumgame& sum, const bounds_options& opt);
    void _step(search_region& region, bound_scale scale, sumgame& sum,
               game_bounds& bounds);

    relation _get_step_comparison(sumgame& sum, game* inverse_scale_game,
                                  bool below_midpoint, int* solve_count);

    bool _g_less_or_equal_s(sumgame& sum, game* inverse_scale_game);
    bool _g_less_or_equal_s(sumgame& sum, bound_t scale_idx, bound_scale scale);

    bool _g_greater_or_equal_s(sumgame& sum, game* inverse_scale_game);
    bool _g_greater_or_equal_s(sumgame& sum, bound_t scale_idx,
                               bound_scale scale);

    bool _validate_interval(bound_t min, bound_t max, bound_scale scale,
                            sumgame& sum, game_bounds& bounds);

    void _refine_bounds(bound_scale scale, game_bounds& bounds, sumgame& sum);

#ifdef CLIP_FUZZY_INTERVAL
    void _report_fuzzy_index(bound_t scale_idx);
    void _clip_fuzzy_interval(search_region& sr, vector<search_region>& regions_next);
#endif

    bool _assume_below_midpoint;
    int _step_count;
    int _search_count;

#ifdef CLIP_FUZZY_INTERVAL
    optional<pair<bound_t, bound_t>> _fuzzy_interval;
#endif


    vector<search_region> _regions;
    vector<search_region> _regions_next;
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
    // either the entire region is OK, or the entire region is outside the
    // bounds

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
game_bounds::game_bounds(bound_scale scale)
    : _scale(scale),
      _lower(numeric_limits<bound_t>::max()),
      _lower_valid(false),
      _lower_relation(REL_FUZZY),
      _upper(numeric_limits<bound_t>::min()),
      _upper_valid(false),
      _upper_relation(REL_FUZZY)
{
}

void game_bounds::set_lower(bound_t lower, relation lower_relation)
{
    assert(lower_relation == REL_LESS || lower_relation == REL_LESS_OR_EQUAL ||
           lower_relation == REL_EQUAL);

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
    assert(upper_relation == REL_GREATER ||
           upper_relation == REL_GREATER_OR_EQUAL ||
           upper_relation == REL_EQUAL);

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

    THROW_ASSERT(add_is_safe(_lower, _upper));
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
    os << bound_scale_to_string(gb.get_scale()) << ": ";

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
    }
    else
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
    }
    else
    {
        os << "?]";
    }

    return os;
}

//////////////////////////////////////// search_region
search_region::search_region(bound_t low, bound_t high) : low(low), high(high)
{
}

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

    THROW_ASSERT(add_is_safe(low, high));
    return (low + high) / 2;
}

//////////////////////////////////////// bounds_finder
bounds_finder::bounds_finder()
{
}

vector<game_bounds_ptr> bounds_finder::find_bounds(
    sumgame& sum, const vector<bounds_options>& options)
{
    vector<game_bounds_ptr> bounds_list;

    for (const bounds_options& opts : options)
    {
        assert(opts.min <= opts.max);

        _reset();

        game_bounds* gb = _make_bounds(sum, opts);
        bounds_list.push_back(game_bounds_ptr(gb));
    }

    return bounds_list;
}

void bounds_finder::_reset()
{
    _assume_below_midpoint = true;
    _step_count = 0;
    _search_count = 0;
#ifdef CLIP_FUZZY_INTERVAL
    _fuzzy_interval.reset();
#endif

    _regions.clear();
    _regions_next.clear();
}

void bounds_finder::_flip_tie_break_rule()
{
    _assume_below_midpoint = !_assume_below_midpoint;
}

search_region bounds_finder::_find_initial_interval(sumgame& sum,
                                                    const bounds_options& opt,
                                                    game_bounds* bounds)
{
    search_region sr(opt.min, opt.max);
    const bound_scale scale = opt.scale;

    /*
        TODO handle BOUND_SCALE_UP_STAR. This function makes several assumptions
        about scale indices:
            idx 0 == 0
            idx 1 > 0
            idx -1 < 0
    */
    switch (scale)
    {
        case BOUND_SCALE_UP_STAR:
            return sr;
        case BOUND_SCALE_UP:
        case BOUND_SCALE_DYADIC_RATIONAL:
            break;
    }

    const outcome_class oc_hint = opt.outcome_hint;

    // TODO remove this assert or hide it behind a debug flag
    //assert(LOGICAL_IMPLIES(oc_hint != outcome_class::U, oc_hint == get_sum_outcome(sum)));

    switch (oc_hint)
    {
        case outcome_class::P:
        {
            bounds->set_equal(0);
            sr.low = 0;
            sr.high = 0;
            return sr;
        }

        case outcome_class::N:
        {
            _report_fuzzy_index(0);
            sr.low = -INITIAL_INTERVAL_MAGNITUDE;
            sr.high = INITIAL_INTERVAL_MAGNITUDE;
            break;
        }

        case outcome_class::U:
        {
            sr.low = -INITIAL_INTERVAL_MAGNITUDE;
            sr.high = INITIAL_INTERVAL_MAGNITUDE;
            break;
        }

        case outcome_class::L:
        {
            sr.low = 0;
            sr.high = INITIAL_INTERVAL_MAGNITUDE;
            bounds->set_lower(0, REL_LESS);
            break;
        }

        case outcome_class::R:
        {
            sr.low = -INITIAL_INTERVAL_MAGNITUDE;
            sr.high = 0;
            bounds->set_upper(0, REL_GREATER);
            break;
        }
    }

    // Widen upper
    if (sr.high != 0)
    {
        while (1)
        {
            unique_ptr<game> g_inv(get_inverse_scale_game(sr.high, scale));
            const bool bound_ge_sum = _g_greater_or_equal_s(sum, g_inv.get());

            if (bound_ge_sum)
            {
                bounds->set_upper(sr.high, REL_GREATER_OR_EQUAL);
                break;
            }

#ifdef REPORT_FUZZY_IN_INITIAL_INTERVAL
            const bool bound_le_sum = _g_less_or_equal_s(sum, g_inv.get());
            if (!bound_le_sum)
                _report_fuzzy_index(sr.high);
#endif

            // Multiply by 2 == 2^1
            const bool double_ok = safe_mul2_shift(sr.high, 1);
            THROW_ASSERT(double_ok);
        }
    }

    // Widen lower
    if (sr.low != 0)
    {
        while (1)
        {
            unique_ptr<game> g_inv(get_inverse_scale_game(sr.low, scale));
            const bool bound_le_sum = _g_less_or_equal_s(sum, g_inv.get());

            if (bound_le_sum)
            {
                bounds->set_lower(sr.low, REL_LESS_OR_EQUAL);
                break;
            }

#ifdef REPORT_FUZZY_IN_INITIAL_INTERVAL
            const bool bound_ge_sum = _g_greater_or_equal_s(sum, g_inv.get());
            if (!bound_ge_sum)
                _report_fuzzy_index(sr.low);
#endif

            // Multiply by 2 == 2^1
            const bool double_ok = safe_mul2_shift(sr.low, 1);
            THROW_ASSERT(double_ok);
        }
    }

    //cout << "Initial region: ";
    //cout << "{" << sr.low << " " << sr.high << "}" << endl;

    return sr;
}

game_bounds* bounds_finder::_make_bounds(sumgame& sum,
                                         const bounds_options& opt)
{
    game_bounds* bounds = new game_bounds(opt.scale);

    bool validated_interval =
        false; // true when we've checked that Gmin <= S <= Gmax

    _regions.clear();
    _regions_next.clear();

    _regions.push_back(_find_initial_interval(sum, opt, bounds));
    validated_interval = true; // Validated by `_find_initial_interval(...)`

    //_regions.push_back({opt.min, opt.max});


    // i.e. 0
    if (bounds->is_equal())
        _regions.clear();

    while (!_regions.empty())
    {
        _regions_next.clear();

        for (search_region& sr : _regions)
        {

#ifdef CLIP_FUZZY_INTERVAL
            if (_fuzzy_interval.has_value() && sr.valid())
                _clip_fuzzy_interval(sr, _regions_next);
#endif
            // Skip if this region is invalid or outside of bounds
            if (prune_region(sr, *bounds))
            {
                continue;
            }

            // stop if no more work remaining
            if (bounds->lower_valid() &&
                bounds->get_lower_relation() == REL_EQUAL)
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

void bounds_finder::_step(search_region& region, bound_scale scale,
                          sumgame& sum, game_bounds& bounds)
{
    // S refers to sumgame
    // Gi refers to scale game

    _step_count++;
    assert(region.valid());

    int scale_idx = region.get_midpoint(); // i
    unique_ptr<game> inverse_scale_game(
        get_inverse_scale_game(scale_idx, scale)); // -Gi

    // Pick a test (>= or <=) to try first, based on what side of the bounds
    // space "scale_idx" is on
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
    relation rel = _get_step_comparison(sum, inverse_scale_game.get(),
                                        below_midpoint, &sumgame_solve_count);

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

#ifdef CLIP_FUZZY_INTERVAL
            _report_fuzzy_index(scale_idx);
#endif

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

relation bounds_finder::_get_step_comparison(sumgame& sum,
                                             game* inverse_scale_game,
                                             bool below_midpoint,
                                             int* solve_count)
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
    }
    else
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

    relation rel =
        relation_from_search_results(le_known, is_le, ge_known, is_ge);

    assert(rel != REL_UNKNOWN);
    return rel;
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

bool bounds_finder::_g_less_or_equal_s(sumgame& sum, bound_t scale_idx,
                                       bound_scale scale)
{
    unique_ptr<game> inverse_scale_game(
        get_inverse_scale_game(scale_idx, scale));
    return _g_less_or_equal_s(sum, inverse_scale_game.get());
}

bool bounds_finder::_g_greater_or_equal_s(sumgame& sum,
                                          game* inverse_scale_game)
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

bool bounds_finder::_g_greater_or_equal_s(sumgame& sum, bound_t scale_idx,
                                          bound_scale scale)
{
    unique_ptr<game> inverse_scale_game(
        get_inverse_scale_game(scale_idx, scale));
    return _g_greater_or_equal_s(sum, inverse_scale_game.get());
}

bool bounds_finder::_validate_interval(bound_t min, bound_t max,
                                       bound_scale scale, sumgame& sum,
                                       game_bounds& bounds)
{
    if (!bounds.lower_valid())
    {
        unique_ptr<game> inverse_scale_game(
            get_inverse_scale_game(min, scale)); // -Gmin
        // Gmin <relation> S
        relation rel =
            _get_step_comparison(sum, inverse_scale_game.get(), true, nullptr);

        if (rel != REL_LESS_OR_EQUAL && rel != REL_LESS && rel != REL_EQUAL)
        {
            return false;
        }
        else
        {
            bounds.set_lower(min, rel);
        }
    }

    if (!bounds.upper_valid())
    {
        unique_ptr<game> inverse_scale_game(
            get_inverse_scale_game(max, scale)); // -Gmax
        // Gmax <relation> S
        relation rel =
            _get_step_comparison(sum, inverse_scale_game.get(), false, nullptr);

        if (rel != REL_GREATER_OR_EQUAL && rel != REL_GREATER &&
            rel != REL_EQUAL)
        {
            return false;
        }
        else
        {
            bounds.set_upper(max, rel);
        }
    }

    return true;
}

void bounds_finder::_refine_bounds(bound_scale scale, game_bounds& bounds,
                                   sumgame& sum)
{
    if (bounds.lower_valid() &&
        bounds.get_lower_relation() == REL_LESS_OR_EQUAL)
    {
        const bound_t lower = bounds.get_lower();

        bool is_ge = _g_greater_or_equal_s(sum, lower, scale);
        relation rel = is_ge ? REL_EQUAL : REL_LESS;

        bounds.set_lower(lower, rel);
    }

    if (bounds.upper_valid() &&
        bounds.get_upper_relation() == REL_GREATER_OR_EQUAL)
    {
        const bound_t upper = bounds.get_upper();

        bool is_le = _g_less_or_equal_s(sum, upper, scale);
        relation rel = is_le ? REL_EQUAL : REL_GREATER;

        bounds.set_upper(upper, rel);
    }
}


#ifdef CLIP_FUZZY_INTERVAL
void bounds_finder::_report_fuzzy_index(bound_t scale_idx)
{
    if (_fuzzy_interval.has_value())
    {
        bound_t& interval_low = _fuzzy_interval->first;
        bound_t& interval_high = _fuzzy_interval->second;

        interval_low = min(interval_low, scale_idx);
        interval_high = max(interval_high, scale_idx);
    }
    else
        _fuzzy_interval.emplace(scale_idx, scale_idx);
}

void bounds_finder::_clip_fuzzy_interval(search_region& sr,
                               vector<search_region>& regions_next)
{
    assert(sr.valid() && _fuzzy_interval.has_value());

    const bound_t interval_left = _fuzzy_interval->first;
    const bound_t interval_right = _fuzzy_interval->second;
    assert(interval_left <= interval_right);

    bound_t& p1 = sr.low;
    bound_t& p2 = sr.high;
    assert(p1 <= p2);

    const bool p1_right_of_interval = p1 > interval_right;
    const bool p2_left_of_interval = p2 < interval_left;

    if (p1_right_of_interval || p2_left_of_interval)
        return;

    const bool p1_inside_interval = p1 >= interval_left;
    const bool p2_inside_interval = p2 <= interval_right;

    if (p1_inside_interval)
        p1 = interval_right + 1;
    if (p2_inside_interval)
        p2 = interval_left - 1;
    if (!(p1_inside_interval || p2_inside_interval))
    {
        // p1 and p2 are both outside the interval. The interval is entirely
        // between p1 and p2. Split the search region into 2
        const bound_t r1_p1 = p1;
        const bound_t r1_p2 = interval_left - 1;

        const bound_t r2_p1 = interval_right + 1;
        const bound_t r2_p2 = p2;

        sr.low = r1_p1;
        sr.high = r1_p2;

        _regions_next.push_back({r2_p1, r2_p2});
    }
}
#endif

////////////////////////////////////////
vector<game_bounds_ptr> find_bounds(sumgame& sum,
                                    const vector<bounds_options>& options)
{
    bw old_to_play = sum.to_play();

    bounds_finder bf;
    vector<game_bounds_ptr> bounds_list = bf.find_bounds(sum, options);

    sum.set_to_play(old_to_play);
    return bounds_list;
}

vector<game_bounds_ptr> find_bounds(vector<game*>& games,
                                    const vector<bounds_options>& options)
{
    sumgame sum(BLACK);
    sum.add(games);
    return find_bounds(sum, options);
}

vector<game_bounds_ptr> find_bounds(game* game,
                                    const vector<bounds_options>& options)
{
    sumgame sum(BLACK);
    sum.add(game);
    return find_bounds(sum, options);
}
