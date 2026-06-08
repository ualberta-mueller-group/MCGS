#pragma once

#include <vector>
#include <ostream>
#include <cstdint>
#include <memory>
#include <cassert>
#include <string>
#include <array>

#include "sumgame.h"
#include "cgt_basics.h"
#include "game.h"
#include "serializer.h"

typedef int32_t bound_t;

enum bound_scale
{
    BOUND_SCALE_UP_STAR = 0,
    BOUND_SCALE_UP,
    BOUND_SCALE_DYADIC_RATIONAL,
};

inline static constexpr std::array<bound_scale, 3> BOUND_SCALES_ALL
{
    BOUND_SCALE_UP_STAR,
    BOUND_SCALE_UP,
    BOUND_SCALE_DYADIC_RATIONAL,
};

bool scale_is_infinitesimal(bound_scale scale);

std::string bound_scale_to_string(bound_scale scale);

game* get_scale_game(bound_t scale_idx, bound_scale scale);
game* get_inverse_scale_game(bound_t scale_idx, bound_scale scale);

class game_bounds
{
public:
    game_bounds(bound_scale scale);

    bound_scale get_scale() const { return _scale; }

    void set_scale(bound_scale scale) { _scale = scale; }

    void set_lower(bound_t lower, relation lower_relation);
    void set_upper(bound_t upper, relation upper_relation);
    void set_equal(bound_t lower_and_upper);

    bound_t get_midpoint() const;

    void invalidate_lower();
    void invalidate_upper();
    void invalidate_both();

    inline bool lower_valid() const { return _lower_valid; }

    inline bool upper_valid() const { return _upper_valid; }

    inline bool both_valid() const { return _lower_valid && _upper_valid; }

    inline bound_t get_lower() const
    {
        assert(lower_valid());
        return _lower;
    }

    inline bound_t get_upper() const
    {
        assert(upper_valid());
        return _upper;
    }

    inline relation get_lower_relation() const
    {
        assert(lower_valid());
        return _lower_relation;
    }

    inline relation get_upper_relation() const
    {
        assert(upper_valid());
        return _upper_relation;
    }

    inline bool is_equal() const
    {
        return both_valid() && (get_lower_relation() == REL_EQUAL);
    }

    inline bool operator==(const game_bounds& rhs) const
    {
        return                                          //
            (_scale == rhs._scale) &&                   //
            (_lower == rhs._lower) &&                   //
            (_lower_valid == rhs._lower_valid) &&       //
            (_lower_relation == rhs._lower_relation) && //
            (_upper == rhs._upper) &&                   //
            (_upper_valid == rhs._upper_valid) &&       //
            (_upper_relation == rhs._upper_relation);   //
    }

    inline bool operator!=(const game_bounds& rhs) const
    {
        return !(*this == rhs);
    }
    
private:
    void _set_lower(bound_t lower, relation lower_relation);
    void _set_upper(bound_t upper, relation upper_relation);

    // TODO make these nicer for serialization?
    bound_scale _scale;

    bound_t _lower;
    bool _lower_valid;
    relation _lower_relation;

    bound_t _upper;
    bool _upper_valid;
    relation _upper_relation;

    friend struct serializer<game_bounds>;
};

std::ostream& operator<<(std::ostream& os, const game_bounds& gb);

struct bounds_options
{
    inline bounds_options(bound_scale scale, bound_t min, bound_t max, outcome_class outcome_hint = outcome_class::U)
        : scale(scale), min(min), max(max), outcome_hint(outcome_hint)
    {
    }

    bound_scale scale;
    bound_t min;
    bound_t max;
    outcome_class outcome_hint;
};

typedef std::shared_ptr<game_bounds> game_bounds_ptr;

// TODO return vector<game_bounds> instead of vector<game_bounds_ptr>?
std::vector<game_bounds_ptr> find_bounds(
    sumgame& sum, const std::vector<bounds_options>& options);

std::vector<game_bounds_ptr> find_bounds(
    std::vector<game*>& games, const std::vector<bounds_options>& options);

std::vector<game_bounds_ptr> find_bounds(
    game* game, const std::vector<bounds_options>& options);

