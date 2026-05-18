#include "gen_king_dirt.h"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "cgt_basics.h"
#include "cgt_move.h"
#include "gen_king_dirt_move.h"
#include "grid.h"
#include "game.h"
#include "grid_hash.h"
#include "grid_location.h"
#include "throw_assert.h"
#include "utilities.h"

using namespace std;

////////////////////////////////////////////////// helpers
namespace {
void update_grid_hash_params(grid_hash& gh, const std::vector<int>& params_old,
                             const std::vector<int>& params_new)
{
    assert(params_old.size() == gen_king_dirt::PARAM_COUNT && //
           params_new.size() == gen_king_dirt::PARAM_COUNT    //
    );

    const size_t params_size = params_new.size();
    for (size_t i = 0; i < params_size; i++)
    {
        const int p1 = params_old[i];
        const int p2 = params_new[i];

        if (p1 != p2)
        {
            gh.toggle_parameter(i, p1);
            gh.toggle_parameter(i, p2);
        }
    }
}

} // namespace

//////////////////////////////////////////////////
// class gen_king_dirt_move_generator

class gen_king_dirt_move_generator: public move_generator
{
public:
    gen_king_dirt_move_generator(const gen_king_dirt& g, bw to_play);

    void operator++() override;
    operator bool() const override;
    ::move gen_move() const override;

private:
    enum move_type_enum
    {
        MOVE_TYPE_NONE = 0,
        MOVE_TYPE_PLACE,
        MOVE_TYPE_SLIDE,
    };

    void _increment(bool init);

    bool _increment_place_move(bool init);

    bool _increment_slide_move(bool init);
    bool _increment_slide_from(bool init);
    bool _increment_slide_to(bool init);

    const gen_king_dirt& _g;

    bool _did_generate_place_move;

    move_type_enum _move_type;

    grid_location _loc1;

    size_t _slide_dir_idx;
    grid_location _loc2;

    static constexpr size_t N_DIRS = GRID_DIRS_ALL.size();
};

////////////////////////////////////////////////// gen_king_dirt methods
gen_king_dirt::gen_king_dirt(const vector<int>& params,
                             const vector<int>& board, int_pair shape)
    : grid(board, shape, GRID_TYPE_COLOR), _gh(grid_hash_mask<gen_king_dirt>())
{
    _init_params(params);
    _preprocess_board();
}

gen_king_dirt::gen_king_dirt(const vector<int>& params,
                             const string& game_as_string)
    : grid(game_as_string, GRID_TYPE_COLOR),
      _gh(grid_hash_mask<gen_king_dirt>())
{
    _init_params(params);
    _preprocess_board();
}

void gen_king_dirt::play(const ::move& m, bw to_play)
{
    game::play(m, to_play);
    _params_stack.push_back(_params);

    int_pair coord1, coord2;
    bool place_stone;
    gen_king_dirt_move::unpack_coords(m, coord1, coord2, place_stone);

    if (place_stone)
        _play_place_stone(coord1, to_play);
    else
        _play_slide_stone(coord1, coord2, to_play);
}

void gen_king_dirt::undo_move()
{
    const ::move m_enc = last_move();
    game::undo_move();

    const bw to_play = cgt_move::get_color(m_enc);

    int_pair coord1, coord2;
    bool place_stone;
    gen_king_dirt_move::unpack_coords(m_enc, coord1, coord2, place_stone);

    if (place_stone)
        _undo_move_place_stone(coord1, to_play);
    else
        _undo_move_slide_stone(coord1, coord2, to_play);

    _params_stack.pop_back();
}

move_generator* gen_king_dirt::create_move_generator(bw to_play) const
{
    return new gen_king_dirt_move_generator(*this, to_play);
}

void gen_king_dirt::print(ostream& str) const
{
    str << "gen_king_dirt<";
    str << get_black_unplaced() << ", ";
    str << get_white_unplaced() << ", ";
    str << get_must_place() << ">:";
    str << board_as_string();
}

void gen_king_dirt::print_move(ostream& str, const ::move& m,
                               ebw to_play) const
{
    assert(is_black_white(to_play));
    gen_king_dirt_move::print_as_coords(str, m, shape(), to_play);
}

game* gen_king_dirt::inverse() const
{
    // Must swap black_unplaced and white_unplaced
    const int inv_black_unplaced = get_white_unplaced();
    const int inv_white_unplaced = get_black_unplaced();
    const bool inv_must_place = get_must_place();

    return new gen_king_dirt(
        {inv_black_unplaced, inv_white_unplaced, inv_must_place},
        inverse_board(), shape());
}

game* gen_king_dirt::clone() const
{
    return new gen_king_dirt(*this);
}

bool gen_king_dirt::has_unplaced_stones(bw for_player) const
{
    assert(is_black_white(for_player));

    const int unplaced =
        (for_player == BLACK) ? get_black_unplaced() : get_white_unplaced();

    return unplaced > 0;
}

void gen_king_dirt::_play_place_stone(const int_pair& place_coords, bw to_play)
{
    // Decode
    assert(is_black_white(to_play));
    const int place_point = grid_location::coord_to_point(place_coords, shape());

    assert(checked_is_color(place_point, EMPTY));

    // Update params
    const std::vector<int>& params_old = _params_stack.back();

    assert(_empty_space_count > 0);
    _empty_space_count--;

    assert(LOGICAL_IMPLIES(to_play == BLACK, params_old[PIDX_BLACK_UNPLACED] > 0));
    assert(LOGICAL_IMPLIES(to_play == WHITE, params_old[PIDX_WHITE_UNPLACED] > 0));

    if (to_play == BLACK)
        _params[PIDX_BLACK_UNPLACED]--;
    else
        _params[PIDX_WHITE_UNPLACED]--;

    _normalize_params(_empty_space_count, _params);

    // Update state
    replace(place_point, to_play);

    // Update hash
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        // Board
        _gh.toggle_value(place_coords, EMPTY);
        _gh.toggle_value(place_coords, to_play);

        // Params
        update_grid_hash_params(_gh, params_old, _params);

        hash.__set_value(_gh.get_value());
        _mark_hash_updated();
    }
}

void gen_king_dirt::_play_slide_stone(const int_pair& from_coords,
                                     const int_pair& to_coords, bw to_play)
{
    // Decode
    assert(is_black_white(to_play));

    const int from_point = grid_location::coord_to_point(from_coords, shape());
    const int to_point = grid_location::coord_to_point(to_coords, shape());

    assert(checked_is_color(from_point, to_play) && checked_is_color(to_point, EMPTY));
    assert(LOGICAL_IMPLIES(get_must_place(), !has_unplaced_stones(to_play)));

    // Update params
    const std::vector<int>& params_old = _params_stack.back();

    assert(_empty_space_count > 0);
    _empty_space_count--;

    _normalize_params(_empty_space_count, _params);

    // Update state
    replace(from_point, BORDER);
    replace(to_point, to_play);

    // Update hash
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        // Board
        _gh.toggle_value(from_coords, to_play);
        _gh.toggle_value(from_coords, BORDER);

        _gh.toggle_value(to_coords, EMPTY);
        _gh.toggle_value(to_coords, to_play);

        // Params
        update_grid_hash_params(_gh, params_old, _params);

        hash.__set_value(_gh.get_value());
        _mark_hash_updated();
    }
}

void gen_king_dirt::_undo_move_place_stone(const int_pair& place_coords,
                                           bw to_play)
{
    // Decode
    assert(is_black_white(to_play));
    const int place_point = grid_location::coord_to_point(place_coords, shape());

    assert(checked_is_color(place_point, to_play));

    const std::vector<int>& old_params = _params_stack.back();

    // Update hash
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        // Board
        _gh.toggle_value(place_coords, to_play);
        _gh.toggle_value(place_coords, EMPTY);

        // Params
        update_grid_hash_params(_gh, _params, old_params);

        hash.__set_value(_gh.get_value());
        _mark_hash_updated();
    }

    // Update params
    assert(_empty_space_count >= 0);
    _empty_space_count++;

    _params = old_params;

    // Update state
    replace(place_point, EMPTY);
}

void gen_king_dirt::_undo_move_slide_stone(const int_pair& from_coords,
                                          const int_pair& to_coords, bw to_play)
{
    // Decode
    assert(is_black_white(to_play));

    const int from_point = grid_location::coord_to_point(from_coords, shape());
    const int to_point = grid_location::coord_to_point(to_coords, shape());

    assert(checked_is_color(from_point, BORDER) && checked_is_color(to_point, to_play));

    const std::vector<int>& params_old = _params_stack.back();

    // Update hash
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        // Board
        _gh.toggle_value(from_coords, BORDER);
        _gh.toggle_value(from_coords, to_play);

        _gh.toggle_value(to_coords, to_play);
        _gh.toggle_value(to_coords, EMPTY);

        // Params
        update_grid_hash_params(_gh, _params, params_old);

        hash.__set_value(_gh.get_value());
        _mark_hash_updated();
    }

    // Update params
    assert(_empty_space_count >= 0);
    _empty_space_count++;

    _params = params_old;

    // Update state
    replace(from_point, to_play);
    replace(to_point, EMPTY);
}

////////////////////////////////////////////////////////////////////////////////
//-------------------- Split function ------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// TODO extract some of this functionality from games which reimplement it

namespace {
struct bounding_box
{
    bounding_box(int row_shift, int col_shift, int_pair shape);

    int row_shift;
    int col_shift;
    int_pair shape;
};

bounding_box::bounding_box(int row_shift, int col_shift, int_pair shape)
    : row_shift(row_shift),
      col_shift(col_shift),
      shape(shape)
{
}

vector<int> trim_to_bounding_box(const vector<int>& src_board,
                                 const int_pair& src_shape,
                                 const bounding_box& dst_box)
{
    vector<int> dst_board;

    const int row_shift = dst_box.row_shift;
    const int col_shift = dst_box.col_shift;
    const int_pair dst_shape = dst_box.shape;

    assert(!grid_location::shape_is_empty(dst_shape));

    const int dst_size = dst_shape.first * dst_shape.second;
    dst_board.reserve(dst_size);

    int src_point = col_shift + (row_shift * src_shape.second);

    for (int r = 0; r < dst_shape.first; r++)
    {
        for (int c = 0; c < dst_shape.second; c++)
        {
            const int src_val = src_board[src_point + c];
            dst_board.push_back(src_val);
        }

        src_point += src_shape.second;
    }

    return dst_board;
}

} // namespace

split_result gen_king_dirt::_split_impl() const
{
    if (get_black_unplaced() > 0 || get_white_unplaced() > 0)
        return _split_with_unplaced_stones();
    else
        return _split_without_unplaced_stones();
}

/*
    Converts stones to BORDER if they can't reach EMPTY
*/
split_result gen_king_dirt::_split_with_unplaced_stones() const
{
    assert(get_black_unplaced() > 0 || get_white_unplaced() > 0);

    split_result result;

    if (size() == 0)
    {
        assert(!result.has_value());
        return result;
    }

    // Constants
    const int grid_size = size();
    const int_pair grid_shape = shape();
    // Sentinel values for invalid row/col coordinates
    const int MIN_INVALID = max(grid_shape.first, grid_shape.second) + 1;
    const int MAX_INVALID = -1;

    // Search state
    vector<bool> closed_set(grid_size, false);
    vector<grid_location> open_stack;

    // Current component state
    vector<int> component(grid_size, BORDER);
    int src_border_count = 0;
    int dst_non_border_count = 0;

    /*
        NOTE: these can still be invalid after search completes. Check
        that `dst_non_border_count > 0` before using
    */
    int min_row = MIN_INVALID;
    int max_row = MAX_INVALID;
    int min_col = MIN_INVALID;
    int max_col = MAX_INVALID;

    auto copy_tile = [&](int point, const int_pair& coord, int value) -> void
    {
        assert(is_empty_black_white(value));

        component[point] = value;

        min_row = min(min_row, coord.first);
        max_row = max(max_row, coord.first);
        min_col = min(min_col, coord.second);
        max_col = max(max_col, coord.second);

        dst_non_border_count++;
    };

    // Main search loop
    for (grid_location loc(grid_shape); loc.valid(); loc.increment_position())
    {
        const int point_start = loc.get_point();

        if (closed_set[point_start])
            continue;

        const int tile_start = at(point_start);

        if (tile_start == BORDER)
        {
            src_border_count++;
            closed_set[point_start] = true;
            continue;
        }

        if (tile_start != EMPTY)
            continue;

        assert(open_stack.empty());
        open_stack.emplace_back(loc);
        closed_set[point_start] = true;

        while (!open_stack.empty())
        {
            const grid_location loc1 = open_stack.back();
            open_stack.pop_back();

            const int point1 = loc1.get_point();

            // Already closed, and not copied to component
            assert(closed_set[point1]);
            assert(component[point1] == BORDER);

            const int tile1 = at(point1);
            assert(is_empty_black_white(tile1));

            copy_tile(point1, loc1.get_coord(), tile1);

            grid_location loc2(grid_shape);
            for (grid_dir dir : GRID_DIRS_ALL)
            {
                loc2.set_coord(loc1.get_coord());

                if (!loc2.move(dir))
                    continue;

                const int point2 = loc2.get_point();

                if (closed_set[point2])
                    continue;

                closed_set[point2] = true;

                const int tile2 = at(point2);

                if (tile2 == BORDER)
                {
                   src_border_count++;
                   continue;
                }

                assert(is_empty_black_white(tile2));
                open_stack.emplace_back(loc2);
            }
        }
    }

    assert(!result.has_value());
    if (dst_non_border_count == 0)
    {
        result.emplace();
        return result;
    }

    const int dst_border_count = grid_size - dst_non_border_count;

    assert((min_row <= max_row) && //
           (min_col <= max_col)    //
    );
    bounding_box box(min_row, min_col,
                     int_pair(max_row - min_row + 1, max_col - min_col + 1));

    if (box.shape == shape() &&              //
        src_border_count == dst_border_count //
    )
        return result;

    result.emplace();
    assert(result.has_value());

    if (box.shape != shape())
        component = trim_to_bounding_box(component, shape(), box);

    result->push_back(new gen_king_dirt(_params, component, box.shape));

    return result;
}

/*
    Similar to Amazons split
*/
split_result gen_king_dirt::_split_without_unplaced_stones() const
{
    assert(get_black_unplaced() == 0 && get_white_unplaced() == 0);

    split_result result;

    if (size() == 0)
    {
        assert(!result.has_value());
        return result;
    }

    // Constants
    const int grid_size = size();
    const int_pair grid_shape = shape();
    // Sentinel values for invalid row/col coordinates
    const int MIN_INVALID = max(grid_shape.first, grid_shape.second) + 1;
    const int MAX_INVALID = -1;

    // Search state
    vector<bool> closed_set(grid_size, false);
    vector<grid_location> open_stack;

    // Current component state
    vector<int> component;
    int component_empties;
    int component_coloreds;

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
            component.resize(grid_size, BORDER);
        else
            std::fill(component.begin(), component.end(), BORDER);

        component_empties = 0;
        component_coloreds = 0;

        min_row = MIN_INVALID;
        max_row = MAX_INVALID;
        min_col = MIN_INVALID;
        max_col = MAX_INVALID;
    };

    auto copy_tile = [&](int point, const int_pair& coord, int value) -> void
    {
        assert(is_empty_black_white(value));

        component[point] = value;

        min_row = min(min_row, coord.first);
        max_row = max(max_row, coord.first);
        min_col = min(min_col, coord.second);
        max_col = max(max_col, coord.second);

        if (value == EMPTY)
            component_empties++;
        else
            component_coloreds++;
    };

    // Main search loop
    for (grid_location loc(grid_shape); loc.valid(); loc.increment_position())
    {
        const int point_start = loc.get_point();

        if (closed_set[point_start] || at(point_start) == BORDER)
            continue;

        reset_component();
        assert(open_stack.empty());

        open_stack.emplace_back(loc);
        closed_set[point_start] = true;

        while (!open_stack.empty())
        {
            const grid_location loc1 = open_stack.back();
            open_stack.pop_back();

            const int point1 = loc1.get_point();

            // Already closed, and not copied to component
            assert(closed_set[point1]);
            assert(component[point1] == BORDER);

            const int tile1 = at(point1);
            copy_tile(point1, loc1.get_coord(), tile1);

            grid_location loc2(grid_shape);
            for (grid_dir dir : GRID_DIRS_ALL)
            {
                loc2.set_coord(loc1.get_coord());

                if (!loc2.move(dir))
                    continue;

                const int point2 = loc2.get_point();

                if (closed_set[point2])
                    continue;

                const int tile2 = at(point2);

                if (tile2 == BORDER)
                    continue;

                assert(is_empty_black_white(tile2));
                closed_set[point2] = true;
                open_stack.emplace_back(loc2);
            }
        }

        // Meaningful component? Save it
        if (component_coloreds > 0 && component_empties > 0)
        {
            component_vec.emplace_back(std::move(component));

            assert(
                    (min_row <= max_row) && //
                    (min_col <= max_col)    //
                    );

            const int row_shift = min_row;
            const int col_shift = min_col;

            const int row_dim = max_row - min_row + 1;
            const int col_dim = max_col - min_col + 1;

            bounding_boxes.emplace_back(row_shift, col_shift,
                                        int_pair(row_dim, col_dim));
        }
    }

    assert(component_vec.size() == bounding_boxes.size());
    assert(!result.has_value());

    if (component_vec.size() == 1 &&              //
        bounding_boxes.back().shape == shape() && //
        component_vec.back() == board_const()     //
    )
    {
        assert(!result.has_value());
        return result;
    }

    result.emplace();
    assert(result.has_value());

    const vector<int> new_params(PARAM_COUNT, 0);

    const size_t n_components = component_vec.size();
    for (size_t i = 0; i < n_components; i++)
    {
        const vector<int>& comp_board = component_vec[i];
        const bounding_box& box = bounding_boxes[i];

        gen_king_dirt* g = new gen_king_dirt(
            new_params,                                        //
            trim_to_bounding_box(comp_board, grid_shape, box), //
            box.shape                                          //
        );

        result->push_back(g);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

void gen_king_dirt::_init_hash(local_hash& hash) const
{
    _gh.init_from_board_and_type_with_params<int, int>(
        board_const(), shape(), ::game_type<gen_king_dirt>(), _params);

    hash.__set_value(_gh.get_value());
}

void gen_king_dirt::_init_params(const vector<int>& params)
{
    _params = params;
    THROW_ASSERT(params.size() == PARAM_COUNT);

    THROW_ASSERT(params[PIDX_BLACK_UNPLACED] >= 0 &&        //
                 params[PIDX_WHITE_UNPLACED] >= 0 &&        //
                 in_interval(params[PIDX_MUST_PLACE], 0, 1) //
    );
}

void gen_king_dirt::_preprocess_board()
{
    _empty_space_count = 0;

    const vector<int>& board = board_const();
    for (const int x : board)
    {
        if (x == EMPTY)
        {
            _empty_space_count++;
            continue;
        }

        THROW_ASSERT(is_black_white(x) || x == BORDER);
    }

    _normalize_params(_empty_space_count, _params);
}

void gen_king_dirt::_normalize_params(int& empty_count, std::vector<int>& params)
{
    int& black_unplaced = params[PIDX_BLACK_UNPLACED];
    int& white_unplaced = params[PIDX_WHITE_UNPLACED];
    int& must_place = params[PIDX_MUST_PLACE];

    black_unplaced = std::min(black_unplaced, empty_count);
    white_unplaced = std::min(white_unplaced, empty_count);

    if (black_unplaced == 0 && white_unplaced == 0)
        must_place = 0;

    assert(black_unplaced >= 0 &&        //
           white_unplaced >= 0 &&        //
           in_interval(must_place, 0, 1) //
    );
}

//////////////////////////////////////////////////
// gen_king_dirt_move_generator methods

gen_king_dirt_move_generator::gen_king_dirt_move_generator(
    const gen_king_dirt& g, bw to_play)
    : move_generator(to_play),
      _g(g),
      _did_generate_place_move(false),
      _move_type(MOVE_TYPE_PLACE),
      _loc1(g.shape()),
      _loc2(g.shape())
{
    _increment(true);
}

void gen_king_dirt_move_generator::operator++()
{
    assert(*this);
    _increment(false);
}

gen_king_dirt_move_generator::operator bool() const
{
    return _move_type != MOVE_TYPE_NONE;
}

::move gen_king_dirt_move_generator::gen_move() const
{
    assert(*this);

    int_pair coord1(0, 0);
    int_pair coord2(0, 0);
    const bool place_stone = (_move_type == MOVE_TYPE_PLACE);

    assert(_loc1.valid());
    coord1 = _loc1.get_coord();

    if (!place_stone)
    {
        assert(_loc2.valid());
        coord2 = _loc2.get_coord();
    }

    return gen_king_dirt_move::create_from_coords(coord1, coord2, place_stone);
}

void gen_king_dirt_move_generator::_increment(bool init)
{
    assert(init || *this);

    if (init)
    {
        _move_type = MOVE_TYPE_PLACE;

        if (grid_location::shape_is_empty(_g.shape()))
        {
            _move_type = MOVE_TYPE_NONE;
            assert(!(*this));
            return;
        }
    }

    assert(_move_type == MOVE_TYPE_PLACE || _move_type == MOVE_TYPE_SLIDE);

    if (_move_type == MOVE_TYPE_PLACE)
    {
        if (_increment_place_move(init))
        {
            _did_generate_place_move = true;
            assert(*this);
            return;
        }
    }

    const bool init_slide = (_move_type == MOVE_TYPE_PLACE);
    _move_type = MOVE_TYPE_SLIDE;

    const bool success = _increment_slide_move(init_slide);
    if (!success)
        _move_type = MOVE_TYPE_NONE;

    assert(success == *this);
}

bool gen_king_dirt_move_generator::_increment_place_move(bool init)
{
    if (init)
    {
        if (!_g.has_unplaced_stones(to_play()))
            return false;

        _loc1.reset_position();
    }
    else
    {
        assert(_loc1.valid());
        _loc1.increment_position();
    }

    assert(_g.has_unplaced_stones(to_play()));

    while (1)
    {
        if (!_loc1.valid())
            return false;

        const int point1 = _loc1.get_point();

        const int color = _g.at(point1);
        if (color == EMPTY)
            return true;

        _loc1.increment_position();
    }

    return false;
}

bool gen_king_dirt_move_generator::_increment_slide_move(bool init)
{
    if (init)
    {
        if (_g.get_must_place() && _did_generate_place_move)
            return false;
    }

    bool has_from = !init;
    bool has_to = !init;

    while (1)
    {
        // Try to increment "To" coord
        if (has_from && _increment_slide_to(!has_to))
            return true;

        has_to = false;

        // Try to increment "From" coord
        if (_increment_slide_from(!has_from))
        {
            has_from = true;
            continue;
        }

        has_from = false;

        return false;
    }

    return false;
}

bool gen_king_dirt_move_generator::_increment_slide_from(bool init)
{
    if (init)
        _loc1.reset_position();
    else
    {
        assert(_loc1.valid());
        _loc1.increment_position();
    }

    const int player = to_play();
    assert(is_black_white(player));

    while (1)
    {
        if (!_loc1.valid())
            return false;

        const int point1 = _loc1.get_point();

        const int color = _g.at(point1);
        if (color == player)
            return true;

        _loc1.increment_position();
    }

    return false;
}

bool gen_king_dirt_move_generator::_increment_slide_to(bool init)
{
    if (init)
        _slide_dir_idx = 0;
    else
    {
        assert(in_range(_slide_dir_idx, 0, N_DIRS));
        _slide_dir_idx++;
    }

    assert(_slide_dir_idx >= 0);

    while (1)
    {
        if (!(_slide_dir_idx < N_DIRS))
            return false;

        const grid_dir dir = GRID_DIRS_ALL[_slide_dir_idx];

        _loc2 = _loc1;
        assert(_loc2.valid());

        const bool success = _loc2.move(dir);

        if (!success)
        {
            _slide_dir_idx++;
            continue;
        }

        const int point2 = _loc2.get_point();

        const int color = _g.at(point2);
        if (color == EMPTY)
            return true;

        _slide_dir_idx++;
    }

    return false;
}
