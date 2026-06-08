#pragma once

#include <limits>
#include <cassert>
#include <optional>
#include <utility>
#include <vector>

#include "bounds.h"
#include "sumgame.h"
#include "game.h"

////////////////////////////////////////////////// class search_region
class search_region
{
public:
    // inclusive interval
    search_region(bound_t low, bound_t high);

    // *this becomes lower half, returns upper half (both exclude midpoint)
    search_region split(bound_t midpoint);

    inline bool valid() const
    {
        return low <= high;
    }

    inline void invalidate()
    {
        low = std::numeric_limits<bound_t>::max();
        high = std::numeric_limits<bound_t>::min();
    }

    bound_t get_midpoint() const;

    inline bool operator==(const search_region& rhs) const
    {
        assert(valid() && rhs.valid());
        return (low == rhs.low) && (high == rhs.high);
    }

    bound_t low;
    bound_t high;
};

////////////////////////////////////////////////// class bounds_finder
class bounds_finder
{
public:
    typedef std::optional<std::pair<bound_t, bound_t>> fuzzy_interval_t;

    bounds_finder();

    void reset();

    const fuzzy_interval_t& get_fuzzy_interval() const;

    std::vector<game_bounds_ptr> find_bounds(
        sumgame& sum, const std::vector<bounds_options>& options);

    search_region find_initial_interval(sumgame& sum, const bounds_options& opt,
                                        game_bounds* bounds);

    static void clip_using_fuzzy_interval(
        search_region& sr, std::vector<search_region>& regions_next,
        const std::pair<bound_t, bound_t>& fuzzy_interval);

    inline static constexpr bound_t INITIAL_INTERVAL_MAGNITUDE = 1;

private:
    void _flip_tie_break_rule();

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

    void _report_fuzzy_index(bound_t scale_idx);

    bool _assume_below_midpoint;
    int _step_count;
    int _search_count;

    fuzzy_interval_t _fuzzy_interval;

    std::vector<search_region> _regions;
    std::vector<search_region> _regions_next;
};

////////////////////////////////////////////////// bounds_finder methods
inline const bounds_finder::fuzzy_interval_t& bounds_finder::
    get_fuzzy_interval() const
{
    return _fuzzy_interval;
}
