#include "sheep.h"

#include <cassert>
#include <vector>
#include <cstddef>
#include <cmath>
#include <string>
#include <utility>
#include <iostream>
#include <algorithm>

#include "cgt_basics.h"
#include "cgt_move.h"
#include "grid.h"
#include "grid_location.h"
#include "game.h"
#include "safe_arithmetic.h"
#include "throw_assert.h"
#include "bounding_box.h"
#include "sheep_alt_generator.h"



using namespace std;
////////////////////////////////////////////////// class sheep_move_generator
class sheep_move_generator: public move_generator
{
public:
    sheep_move_generator(const sheep& g, bw to_play);

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
};

////////////////////////////////////////////////// helpers
namespace {
void check_valid_board(const sheep& g)
{
    const int SIZE = g.size();

    for (int i = 0; i < SIZE; i++)
    {
        const int herd = g.at(i);

        THROW_ASSERT(negate_is_safe(herd) &&      //
                     abs(herd) <= sheep::MAX_HERD //
        );
    }

    if (SIZE > 0)
        THROW_ASSERT(static_cast<unsigned int>(SIZE - 1) <
                     cgt_move::THREE_PART_MOVE_MAX);
}

} // namespace

////////////////////////////////////////////////// sheep methods

sheep::sheep(const string& game_as_string)
    : grid(game_as_string, GRID_TYPE_NUMBER)
{
    check_valid_board(*this);
}

sheep::sheep(const vector<int>& board, int_pair shape)
    : grid(board, shape, GRID_TYPE_NUMBER)
{
    check_valid_board(*this);
}

sheep::sheep(const pair<vector<int>, int_pair>& board_pair)
    : grid(board_pair, GRID_TYPE_NUMBER)
{
    check_valid_board(*this);
}

void sheep::play(const ::move& m, bw to_play)
{
    game::play(m, to_play);

    // Decode
    unsigned int point_start;
    unsigned int point_end;
    unsigned int target_herd_uns;
    cgt_move::decode_three_part_move(m, point_start, point_end,
                                     target_herd_uns);

    const int target_herd_abs = static_cast<int>(target_herd_uns);
    assert(target_herd_abs >= 0);

    const int target_herd = apply_herd_sign(target_herd_abs, to_play);

    const int old_start_herd = at(point_start);
    const int new_start_herd = old_start_herd - target_herd;

    // Check
    assert(herd_movable_and_belongs_to_player(old_start_herd,
                                              to_play) && // can move herd
           at(point_end) == 0 &&                          // destination empty
           abs(target_herd) < abs(old_start_herd) &&      // not moving too many
           abs(new_start_herd) < abs(old_start_herd) &&   // make start smaller
           abs(new_start_herd) >= 1                       // don't leave 0
    );

    // Apply
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        // Remove state
        hash.toggle_value(2 + point_start, old_start_herd);
        hash.toggle_value(2 + point_end, 0);

        // Add state
        hash.toggle_value(2 + point_start, new_start_herd);
        hash.toggle_value(2 + point_end, target_herd);

        _mark_hash_updated();
    }

    replace_int(point_start, new_start_herd);
    replace_int(point_end, target_herd);
}

void sheep::undo_move()
{
    const ::move m_enc = last_move();
    game::undo_move();

    const bw to_play = cgt_move::get_color(m_enc);
    const ::move m_dec = cgt_move::decode(m_enc);

    // Decode
    unsigned int point_start;
    unsigned int point_end;
    unsigned int target_herd_uns;
    cgt_move::decode_three_part_move(m_dec, point_start, point_end,
                                     target_herd_uns);

    const int target_herd_abs = static_cast<int>(target_herd_uns);
    assert(target_herd_abs >= 0);

    const int target_herd = apply_herd_sign(target_herd_abs, to_play);

    const int new_start_herd = at(point_start);
    const int old_start_herd = target_herd + new_start_herd;

    // Check
    assert(herd_belongs_to_player(at(point_start), to_play) && //
           herd_belongs_to_player(at(point_end), to_play) &&
           at(point_end) == target_herd //
    );                                  //

    assert(herd_movable_and_belongs_to_player(old_start_herd, to_play));

    // Apply
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        // Remove state
        hash.toggle_value(2 + point_start, new_start_herd);
        hash.toggle_value(2 + point_end, target_herd);

        // Add state
        hash.toggle_value(2 + point_start, old_start_herd);
        hash.toggle_value(2 + point_end, 0);

        _mark_hash_updated();
    }

    replace_int(point_start, old_start_herd);
    replace_int(point_end, 0);
}

move_generator* sheep::create_move_generator(bw to_play) const
{
#ifdef SHEEP_ALT_MOVE
    return new sheep_alt_generator(*this, to_play);
#else
    return new sheep_move_generator(*this, to_play);
#endif
}

void sheep::print(ostream& str) const
{
    str << "sheep: ";

    const int_pair& s = shape();
    const int ROWS = s.first;
    const int COLS = s.second;

    int idx = 0;
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            const int tile = at(idx + c);
            str << tile << ' ';
        }

        idx += COLS;
        if (r + 1 < ROWS)
            str << "| ";
    }
}

void sheep::print_move(std::ostream& str, const ::move& m) const
{
    unsigned int from_point;
    unsigned int to_point;
    unsigned int target_herd_abs;

    cgt_move::decode_three_part_move(m, from_point, to_point,
                                     target_herd_abs);

    std::cout << point_coord_as_string(from_point) << "-"
              << point_coord_as_string(to_point) << "-"
              << target_herd_abs;
}

game* sheep::inverse() const
{
    return new sheep(inverse_number_board(), shape());
}

////////////////////////////////////////////////// sheep_move_generator methods
sheep_move_generator::sheep_move_generator(const sheep& g, bw to_play)
    : move_generator(to_play),
      _g(g),
      _player_step(to_play == BLACK ? 1 : -1),
      _herd_start(g.shape()),
      _target_end(g.shape())
{
    _increment(true);
}

void sheep_move_generator::operator++()
{
    assert(*this);
    _increment(false);
}

sheep_move_generator::operator bool() const
{
    return _has_move;
}

::move sheep_move_generator::gen_move() const
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

bool sheep_move_generator::_increment(bool init)
{
    assert(init || *this);

    if (init)
    {
        _has_move = false;

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
        return false;
    }
}

bool sheep_move_generator::_increment_herd_start(bool init)
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

bool sheep_move_generator::_increment_target_dir(bool init)
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

bool sheep_move_generator::_increment_target_size(bool init)
{
    assert(_increment_size_precondition());

    const bw player = to_play();
    assert(is_black_white(player) &&                  //
           _player_step == (player == BLACK ? 1 : -1) //
    );

    if (init)
        _target_size = _player_step;
    else
    {
        const int abs_target_prev = abs(_target_size);
        assert(abs_target_prev < abs(_herd_start_size));

        _target_size += _player_step;
        assert(abs(_target_size) > abs_target_prev);
    }

    return _target_size != _herd_start_size;
}

// For debugging. NOTE: doesn't check pathing
bool sheep_move_generator::_increment_size_precondition() const
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

#ifdef SHEEP_SPLIT
////////////////////////////////////////////////// split
/*
   Find all 6-connected components
*/
split_result sheep::_split_impl() const
{
    if (size() == 0)
        return {};

    split_result result = split_result(vector<game*>());

    // Constants
    const int grid_size = size();
    const int_pair grid_shape = shape();
    const int MIN_INVALID = max(grid_shape.first, grid_shape.second) + 1;
    const int MAX_INVALID = -1;

    // Search state
    vector<bool> closed_set(grid_size, false);
    vector<grid_location> open_stack;

    // Current component
    vector<int> component;
    bool component_has_empty;
    bool component_has_herd;

    int min_row;
    int max_row;
    int min_col;
    int max_col;

    // Components
    vector<vector<int>> component_vec;
    vector<bounding_box> bounding_boxes;

    // Helpers
    auto reset_component = [&]() -> void
    {
        if (component.empty())
            component.resize(grid_size, 1);
        else
            for (int& herd : component)
                herd = 1;

        component_has_herd = false;
        component_has_empty = false;

        min_row = MIN_INVALID;
        max_row = MAX_INVALID;
        min_col = MIN_INVALID;
        max_col = MAX_INVALID;
    };

    auto copy_herd = [&](int point, const int_pair& coord, int herd) -> void
    {
        assert(
            herd != 1 &&                                              //
            herd != -1 &&                                             //
            negate_is_safe(herd) &&                                   //
            abs(herd) <= MAX_HERD &&                                  //
            grid_location::coord_to_point(coord, grid_shape) == point //
        );

        min_row = min(min_row, coord.first);
        max_row = max(max_row, coord.first);
        min_col = min(min_col, coord.second);
        max_col = max(max_col, coord.second);

        if (herd == 0)
            component_has_empty = true;
        else
            component_has_herd = true;

        component[point] = herd;
    };

    // Main search loop
    for (grid_location loc(grid_shape); loc.valid(); loc.increment_position())
    {
        const int point_start = loc.get_point();

        if (closed_set[point_start])
            continue;

        const int herd_start = at(point_start);
        if (herd_start == 1 || herd_start == -1)
            continue;

        reset_component();
        assert(open_stack.empty());

        open_stack.emplace_back(loc);
        closed_set[point_start] = true;

        while (!open_stack.empty())
        {
            grid_location loc1 = open_stack.back();
            open_stack.pop_back();

            const int point1 = loc1.get_point();

            assert(closed_set[point1] && component[point1] == 1);
            const int herd1 = at(point1);
            assert(herd1 != 1 && herd1 != -1);
            copy_herd(point1, loc1.get_coord(), herd1);

            // Find neighbors
            grid_location loc2(grid_shape);
            for (grid_dir dir : GRID_DIRS_HEX)
            {
                loc2 = loc1;

                if (!loc2.move(dir))
                    continue;

                const int point2 = loc2.get_point();

                if (closed_set[point2])
                    continue;

                const int herd2 = at(point2);

                if (herd2 == 1 || herd2 == -1)
                    continue;

                closed_set[point2] = true;
                open_stack.emplace_back(loc2);
            }
        }

        // Only save component if it has moves
        if (component_has_herd && component_has_empty)
        {
            component_vec.emplace_back(std::move(component));

            assert(min_row <= max_row &&     //
                   min_col <= max_col &&     //
                   min_row != MIN_INVALID && //
                   max_row != MAX_INVALID && //
                   min_col != MIN_INVALID && //
                   max_col != MAX_INVALID    //
            );

            const int row_shift = min_row;
            const int col_shift = min_col;

            const int row_dim = max_row - min_row + 1;
            const int col_dim = max_col - min_col + 1;

            assert(row_dim > 0 && //
                   col_dim > 0    //
            );

            bounding_boxes.emplace_back(row_shift, col_shift,
                                        int_pair(row_dim, col_dim));
        }
    }

    assert(component_vec.size() == bounding_boxes.size() && //
           result->empty()                                  //
    );

    if (component_vec.size() == 1 &&              //
        bounding_boxes.back().shape == shape() && //
        component_vec.back() == board_const()     //
    )
        return {};

    const size_t n_components = component_vec.size();
    for (size_t i = 0; i < n_components; i++)
    {
        const vector<int>& component_board = component_vec[i];
        const bounding_box& box = bounding_boxes[i];

        sheep* new_game = new sheep(std::pair<vector<int>, int_pair>(
            trim_to_bounding_box(component_board, grid_shape, box), box.shape));

        result->push_back(new_game);
    }

    return result;
}
#endif
