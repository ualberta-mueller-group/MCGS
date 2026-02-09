#pragma once

#include <type_traits>
#include <vector>
#include "game.h"
#include <ostream>
#include <cstdint>
#include "sumgame.h"
#include <memory>
#include <cassert>

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

    inline bool operator==(const game_bounds& rhs) const
    {
        return                                          //
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
    bound_t _lower;
    bool _lower_valid;
    relation _lower_relation;

    bound_t _upper;
    bool _upper_valid;
    relation _upper_relation;

    friend struct serializer<game_bounds*>;
};

template <>
struct serializer<game_bounds*>
{
    static_assert(std::is_same_v<bound_t, int32_t>);

    inline static void save(obuffer& os, const game_bounds* bounds)
    {
        os.write_i32(bounds->_lower);
        os.write_bool(bounds->_lower_valid);
        os.write_enum<relation>(bounds->_lower_relation);

        os.write_i32(bounds->_upper);
        os.write_bool(bounds->_upper_valid);
        os.write_enum<relation>(bounds->_upper_relation);
    }

    inline static game_bounds* load(ibuffer& is)
    {
        game_bounds* bounds = new game_bounds();

        bounds->_lower = is.read_i32();
        bounds->_lower_valid = is.read_bool();
        bounds->_lower_relation = is.read_enum<relation>();

        bounds->_upper = is.read_i32();
        bounds->_upper_valid = is.read_bool();
        bounds->_upper_relation = is.read_enum<relation>();

        return bounds;
    }
};

std::ostream& operator<<(std::ostream& os, const game_bounds& gb);

struct bounds_options
{
    inline bounds_options(bound_scale scale, bound_t min, bound_t max)
        : scale(scale), min(min), max(max)
    {
    }

    bound_scale scale;
    bound_t min;
    bound_t max;
};

typedef std::shared_ptr<game_bounds> game_bounds_ptr;

// TODO return vector<game_bounds> instead of vector<game_bounds_ptr>?
std::vector<game_bounds_ptr> find_bounds(
    sumgame& sum, const std::vector<bounds_options>& options);

std::vector<game_bounds_ptr> find_bounds(
    std::vector<game*>& games, const std::vector<bounds_options>& options);

std::vector<game_bounds_ptr> find_bounds(
    game* game, const std::vector<bounds_options>& options);

