#pragma once

#include <vector>
#include "game.h"
#include <ostream>
#include <cstdint>
#include "sumgame.h"
#include <memory>

typedef int32_t bound_t;

enum bound_scale
{
    BOUND_SCALE_UP_STAR,
    BOUND_SCALE_UP,
    BOUND_SCALE_DYADIC_RATIONAL,
};

game* get_scale_game(bound_t scale_idx, bound_scale scale);
game* get_inverse_scale_game(bound_t scale_idx, bound_scale scale);

class game_bounds
{
public:
    game_bounds();

    void set_lower(bound_t lower, relation lower_relation);
    void set_upper(bound_t upper, relation upper_relation);
    void set_equal(bound_t lower_and_upper);

    bound_t get_midpoint() const;

    void invalidate_lower();
    void invalidate_upper();
    void invalidate_both();

    inline bool lower_valid() const
    {
        return _lower_valid;
    }

    inline bool upper_valid() const
    {
        return _upper_valid;
    }

    inline bool both_valid() const
    {
        return _lower_valid && _upper_valid;
    }

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

private:
    void _set_lower(bound_t lower, relation lower_relation);
    void _set_upper(bound_t upper, relation upper_relation);

    // TODO make these nicer for serialization?
    bound_t _lower;
    bool _lower_valid;
    relation _lower_relation;

    bound_t _upper;
    bool _upper_valid;
    relation _upper_relation;
};

std::ostream& operator<<(std::ostream& os, const game_bounds& gb);

struct bounds_options
{
    bound_scale scale;
    bound_t min;
    bound_t max;
};

// TODO return vector<game_bounds> instead of vector<game_bounds_ptr>?

typedef std::shared_ptr<game_bounds> game_bounds_ptr;

std::vector<game_bounds_ptr> find_bounds(sumgame& sum, const std::vector<bounds_options>& options);
std::vector<game_bounds_ptr> find_bounds(std::vector<game*>& games, const std::vector<bounds_options>& options);
std::vector<game_bounds_ptr> find_bounds(game* game, const std::vector<bounds_options>& options);

void test_bounds();
