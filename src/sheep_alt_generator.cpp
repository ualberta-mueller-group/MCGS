#include "sheep_alt_generator.h"

#include <cassert>
#include <cmath>

#include "sheep.h"
#include "grid_location.h"

sheep_alt_generator::sheep_alt_generator(const sheep& g, bw to_play)
    : move_generator(to_play),
      _g(g),
      _player_step(to_play == BLACK ? 1 : -1),
      _first_pass(true),
      _herd_start(g.shape()),
      _target_end(g.shape())
{
    _increment(true);
}

void sheep_alt_generator::operator++()
{
    assert(*this);
    _increment(false);
}

sheep_alt_generator::operator bool() const
{
    return _has_move;
}

::move sheep_alt_generator::gen_move() const
{
    assert(*this);

    /*
       3 part move:
       _herd_start_idx
       _target_end_idx
       _target_size (as absolute)
    */
    assert(_herd_start_idx >= 0 && //
           _target_end_idx >= 0    //
    );

    return cgt_move::encode_three_part_move(_herd_start_idx, _target_end_idx,
                                            abs(_target_size));
}

bool sheep_alt_generator::_increment(bool init)
{
    assert(init || *this);

    if (init)
    {
        _has_move = false;
        _first_pass = true;

        if (_g.size() == 0)
            return false;
    }

    bool has_herd_start = _has_move;
    bool has_target_dir = _has_move;
    bool has_target_size = _has_move;

    while (true)
    {
        // Try to increment target size
        if (has_target_dir && _increment_target_size(!has_target_size))
        {
            _has_move = true;
            return true;
        }

        has_target_size = false;

        // Try to increment target dir
        if (has_herd_start && _increment_target_dir(!has_target_dir))
        {
            has_target_dir = true;
            continue;
        }

        has_target_dir = false;

        // Try to increment herd start
        if (_increment_herd_start(!has_herd_start))
        {
            has_herd_start = true;
            continue;
        }

        _has_move = false;

        if (_first_pass)
        {
            _first_pass = false;
            if (_increment_herd_start(true))
            {
                has_herd_start = true;
                continue;
            }
        }

        return false;
    }
}

bool sheep_alt_generator::_increment_herd_start(bool init)
{
    if (init)
    {
        _herd_start.set_coord({0, 0});
        _herd_start_idx = 0;
        _herd_start_size = 0;
    }

    assert(_herd_start.valid());

    if (!init)
        _herd_start.increment_position();

    if (!_herd_start.valid())
        return false;

    const bw player = to_play();
    assert(is_black_white(player));

    while (_herd_start.valid())
    {
        _herd_start_idx = _herd_start.get_point();
        _herd_start_size = _g.at(_herd_start_idx);

        if (herd_movable_and_belongs_to_player(_herd_start_size, player))
            return true;

        _herd_start.increment_position();
    }

    return false;
}

bool sheep_alt_generator::_increment_target_dir(bool init)
{
    if (init)
        _target_dir_idx = 0;
    else
        _target_dir_idx++;

    for (; _target_dir_idx < GRID_DIRS_HEX.size(); _target_dir_idx++)
    {
        const grid_dir dir = GRID_DIRS_HEX[_target_dir_idx];

        _target_end = _herd_start;
        assert(_target_end.valid());

        // Find real target location by moving 1 step at a time
        grid_location neighbor = _target_end;

        while (true)
        {
            // Get next step
            if (!neighbor.move(dir))
                break;

            // Is this step pathable?
            const int neighbor_point = neighbor.get_point();
            const int neighbor_val = _g.at(neighbor_point);

            if (neighbor_val != 0)
                break;

            // Allow this step
            _target_end = neighbor;
        }

        // Accept IFF target moved from the start
        // if (_target_end == _herd_start) // TODO equality operator?
        assert(_target_end.get_shape() == _herd_start.get_shape());
        if (_target_end.get_coord() == _herd_start.get_coord())
            continue;

        // Accept
        _target_end_idx = _target_end.get_point();
        assert(_g.at(_target_end_idx) == 0);
        return true;
    }

    return false;
}

bool sheep_alt_generator::_increment_target_size(bool init)
{
    assert(_increment_size_precondition());

    const bw player = to_play();
    assert(is_black_white(player) &&                  //
           _player_step == (player == BLACK ? 1 : -1) //
    );

    if (init)
    {
        _half_herd = _player_step;
        assert(abs(_half_herd) > 0);

        _target_size = _first_pass ? _half_herd : _player_step;
    }
    else
    {
        if (_first_pass)
            return false;

        const int abs_target_prev = abs(_target_size);
        assert(abs_target_prev < abs(_herd_start_size));

        _target_size += _player_step;
        assert(abs(_target_size) > abs_target_prev);
    }

    if (!_first_pass && _target_size == _half_herd)
    {
        const int abs_target_prev = abs(_target_size);
        assert(abs_target_prev < abs(_herd_start_size));

        _target_size += _player_step;
        assert(abs(_target_size) > abs_target_prev);
    }

    return _target_size != _herd_start_size;
}

// For debugging. NOTE: doesn't check pathing
bool sheep_alt_generator::_increment_size_precondition() const
{
    // Valid start point
    if (!_herd_start.valid())
        return false;

    assert(_herd_start.get_point() == _herd_start_idx);
    const int herd = _g.at(_herd_start_idx);

    assert(herd == _herd_start_size);

    if (!herd_movable_and_belongs_to_player(herd, to_play()))
        return false;

    // Valid end point
    assert(_target_end.valid() &&                     //
           _target_end.get_point() == _target_end_idx //
    );

    return _g.at(_target_end_idx) == 0;
}

