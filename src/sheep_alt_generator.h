#pragma once

#include <cstddef>

#include "sheep.h"
#include "grid_location.h"

/*
    Uses move3 representation:
    1. Target herd value (signed)
    2. Herd start index
    3. Herd end index
*/
class sheep_alt_generator: public move_generator
{
public:
    sheep_alt_generator(const sheep& g, bw to_play);

    void operator++() override;
    operator bool() const override;
    ::move gen_move() const override;

private:
    bool _increment(bool init);

    bool _increment_herd_start(bool init);
    bool _increment_target_dir(bool init);
    bool _increment_target_size(bool init);

    bool _increment_size_precondition() const;

    const sheep& _g;
    const int _player_step;

    bool _has_move;

    bool _first_pass;

    // Part 1
    grid_location _herd_start;
    int _herd_start_idx;
    int _herd_start_size;

    // Part 2
    size_t _target_dir_idx;
    grid_location _target_end;
    int _target_end_idx;

    // Part 3
    int _target_size;
    int _half_herd;
};

