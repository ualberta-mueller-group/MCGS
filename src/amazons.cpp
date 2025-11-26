#include "amazons.h"

#include <cstddef>
#include <cassert>
#include <vector>
#include <string>
#include <algorithm>
#include <utility>
#include <ostream>

#include "cgt_basics.h"
#include "cgt_move_new.h"
#include "grid.h"
#include "print_move_helpers.h"
#include "grid_location.h"
#include "throw_assert.h"

class amazons_move_generator: public move_generator
{
public:
    amazons_move_generator(const amazons& g, bw to_play);

    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    bool _increment(bool init);

    bool _increment_queen_start(bool init);
    bool _increment_queen_end(bool init);
    bool _increment_arrow_end(bool init);

    bool _point_is_empty(int point) const;


    const amazons& _game;

    bool _has_move;

    grid_location _queen_start;
    int _move1;

    size_t _queen_dir_idx;
    grid_location _queen_end;
    int _move2;

    size_t _arrow_dir_idx;
    grid_location _arrow_end;
    int _move3;

    static constexpr size_t N_DIRS = GRID_DIRS_ALL.size();
};

using namespace std;

////////////////////////////////////////////////// helper functions
namespace {

bool only_legal_colors(const std::vector<int>& board)
{
    for (const int& x : board)
        if (!is_empty_black_white(x) && x != BORDER)
            return false;
    return true;
}

} // namespace

////////////////////////////////////////////////// amazons methods
amazons::amazons(int n_rows, int n_cols)
    : grid(n_rows, n_cols, GRID_TYPE_COLOR)
#ifdef USE_GRID_HASH
      , _gh(AMAZONS_GRID_HASH_MASK)
#endif
{
    THROW_ASSERT(only_legal_colors(board_const()));
}

amazons::amazons(const std::vector<int>& board, int_pair shape)
    : grid(board, shape, GRID_TYPE_COLOR)
#ifdef USE_GRID_HASH
      , _gh(AMAZONS_GRID_HASH_MASK)
#endif
{
    THROW_ASSERT(only_legal_colors(board_const()));
}

amazons::amazons(const std::string& game_as_string)
    : grid(game_as_string, GRID_TYPE_COLOR)
#ifdef USE_GRID_HASH
      , _gh(AMAZONS_GRID_HASH_MASK)
#endif
{
    THROW_ASSERT(only_legal_colors(board_const()));
}

void amazons::play(const ::move& m, bw to_play)
{
    game::play(m, to_play);


    int_pair coord1, coord2, coord3;
    cgt_move_new::move6_unpack_coords(m, coord1, coord2, coord3);

    const int_pair& s = shape();
    const int point1 = grid_location::coord_to_point(coord1, s);
    const int point2 = grid_location::coord_to_point(coord2, s);
    const int point3 = grid_location::coord_to_point(coord3, s);

    // TODO check paths?

    // Queen end can't be queen start or arrow end
    assert(point2 != point1 && point2 != point3);

    // Move queen
    assert(at(point1) == to_play && at(point2) == EMPTY);
    replace(point1, EMPTY);
    replace(point2, to_play);

    // Shoot arrow
    assert(at(point3) == EMPTY);
    replace(point3, BORDER);

    // TODO this needs unit tests. And debug mode testing for play()/undo()
    // assert_restore_sumgame should catch it?
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

#ifdef USE_GRID_HASH
        // Remove old state
        _gh.toggle_value(coord1, to_play);
        _gh.toggle_value(coord2, EMPTY);

        if (point1 != point3)
            _gh.toggle_value(coord3, EMPTY);

        // Add new state
        _gh.toggle_value(coord2, to_play);
        _gh.toggle_value(coord3, BORDER);

        if (point1 != point3)
            _gh.toggle_value(coord1, EMPTY);

        hash.__set_value(_gh.get_value());
#else
        // Remove old state
        hash.toggle_value(2 + point1, to_play);
        hash.toggle_value(2 + point2, EMPTY);

        if (point1 != point3)
            hash.toggle_value(2 + point3, EMPTY);

        // Add new state
        hash.toggle_value(2 + point2, to_play);
        hash.toggle_value(2 + point3, BORDER);

        if (point1 != point3)
            hash.toggle_value(2 + point1, EMPTY);
#endif

        _mark_hash_updated();
    }
}

void amazons::undo_move()
{
    const ::move m_enc = last_move();
    game::undo_move();

    const bw to_play = cgt_move_new::get_color(m_enc);


    int_pair coord1, coord2, coord3;
    cgt_move_new::move6_unpack_coords(m_enc, coord1, coord2, coord3);

    const int_pair& s = shape();
    const int point1 = grid_location::coord_to_point(coord1, s);
    const int point2 = grid_location::coord_to_point(coord2, s);
    const int point3 = grid_location::coord_to_point(coord3, s);

    // TODO check paths?

    // Queen end can't be queen start or arrow end
    assert(point2 != point1 && point2 != point3);

    // Remove arrow
    assert(at(point3) == BORDER);
    replace(point3, EMPTY);

    // Move queen back
    assert(at(point1) == EMPTY && at(point2) == to_play);
    replace(point1, to_play);
    replace(point2, EMPTY);

    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();
#ifdef USE_GRID_HASH
        // Remove old state
        _gh.toggle_value(coord2, to_play);
        _gh.toggle_value(coord3, BORDER);

        if (point1 != point3)
            _gh.toggle_value(coord1, EMPTY);

        // Add new state
        _gh.toggle_value(coord1, to_play);
        _gh.toggle_value(coord2, EMPTY);

        if (point1 != point3)
            _gh.toggle_value(coord3, EMPTY);

        hash.__set_value(_gh.get_value());
#else
        // Remove old state
        hash.toggle_value(2 + point2, to_play);
        hash.toggle_value(2 + point3, BORDER);

        if (point1 != point3)
            hash.toggle_value(2 + point1, EMPTY);

        // Add new state
        hash.toggle_value(2 + point1, to_play);
        hash.toggle_value(2 + point2, EMPTY);

        if (point1 != point3)
            hash.toggle_value(2 + point3, EMPTY);
#endif

        _mark_hash_updated();
    }
}

move_generator* amazons::create_move_generator(bw to_play) const
{
    return new amazons_move_generator(*this, to_play);
}

void amazons::print_move(std::ostream& str, const ::move& m) const
{
    // (from, to, arrow destination)
    print_move6_as_coords(str, m, shape());
}

game* amazons::inverse() const
{
    return new amazons(inverse_board(), shape());
}

////////////////////////////////////////////////// split
#ifdef AMAZONS_SPLIT


namespace {

//////////////////////////////////////// struct bounding_box
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

    const int& row_shift = dst_box.row_shift;
    const int& col_shift = dst_box.col_shift;
    const int_pair& dst_shape = dst_box.shape;

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

/*
    Find 8-connected components, following BLACK/WHITE/EMPTY, having at least
    one empty space and at least one stone
*/
split_result amazons::_split_impl() const
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
            for (int& tile : component)
                tile = BORDER;

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
            grid_location loc1 = open_stack.back();
            open_stack.pop_back();

            const int point1 = loc1.get_point();

            // Already closed, and copied to component
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
    assert(result->empty());

    if (component_vec.size() == 1 && bounding_boxes.back().shape == shape() &&
        component_vec.back() == board_const())
        return {};

    const size_t n_components = component_vec.size();
    for (size_t i = 0; i < n_components; i++)
    {
        const vector<int>& comp_board = component_vec[i];
        const bounding_box& box = bounding_boxes[i];

        result->push_back(new amazons(trim_to_bounding_box(comp_board, grid_shape, box), box.shape));
    }

    return result;
}
#endif

#ifdef USE_GRID_HASH
void amazons::_init_hash(local_hash& hash) const
{
    _gh.init_from_grid(*this);
    hash.__set_value(_gh.get_value());
}
#endif

//////////////////////////////////////////////////
// amazons_move_generator methods

amazons_move_generator::amazons_move_generator(const amazons& g, bw to_play)
    : move_generator(to_play),
      _game(g),
      _has_move(false),
      _queen_start(g.shape()),
      _queen_end(g.shape()),
      _arrow_end(g.shape())
{
    _increment(true);
}

void amazons_move_generator::operator++()
{
    assert(*this);
    _increment(false);
}

amazons_move_generator::operator bool() const
{
    return _has_move;
}

::move amazons_move_generator::gen_move() const
{
    assert(*this);

    const int_pair& coord1 = _queen_start.get_coord();
    const int_pair& coord2 = _queen_end.get_coord();
    const int_pair& coord3 = _arrow_end.get_coord();

    return cgt_move_new::move6_create_from_coords(coord1, coord2, coord3);
}

bool amazons_move_generator::_increment(bool init)
{
    assert(init || *this);

    if (init)
    {
        _has_move = false;

        if (_game.size() == 0)
            return false;
    }

    bool has_queen_start = _has_move;
    bool has_queen_end = _has_move;
    bool has_arrow_end = _has_move;

    while (true)
    {
        // Try to increment arrow
        if (has_queen_end && _increment_arrow_end(!has_arrow_end))
        {
            _has_move = true;
            return true;
        }

        has_arrow_end = false;

        // Try to increment queen end
        if (has_queen_start && _increment_queen_end(!has_queen_end))
        {
            has_queen_end = true;
            continue;
        }

        has_queen_end = false;

        // Try to increment queen
        if (_increment_queen_start(!has_queen_start))
        {
            has_queen_start = true;
            continue;
        }

        _has_move = false;
        return false;
    }
}

bool amazons_move_generator::_increment_queen_start(bool init)
{
    if (init)
        assert(_queen_start.get_coord() == int_pair(0, 0));
    else
        _queen_start.increment_position();

    while (_queen_start.valid())
    {
        _move1 = _queen_start.get_point();

        if (_game.at(_move1) == to_play())
            return true;

        _queen_start.increment_position();
    }

    return false;
}

bool amazons_move_generator::_increment_queen_end(bool init)
{
    if (init)
    {
        _queen_dir_idx = 0;
        _queen_end.set_coord(_queen_start.get_coord());
    }

    assert(_queen_dir_idx < N_DIRS);

    while (true)
    {
        const grid_dir dir = GRID_DIRS_ALL[_queen_dir_idx];

        if (_queen_end.move(dir))
        {
            _move2 = _queen_end.get_point();

            if (_point_is_empty(_move2))
                return true;
        }

        _queen_dir_idx++;
        if (_queen_dir_idx >= N_DIRS)
            return false;

        _queen_end.set_coord(_queen_start.get_coord());
    }

    return false;
}

bool amazons_move_generator::_increment_arrow_end(bool init)
{
    if (init)
    {
        _arrow_dir_idx = 0;
        _arrow_end.set_coord(_queen_end.get_coord());
    }

    assert(_arrow_dir_idx < N_DIRS);

    while (true)
    {
        // Try to move in current direction
        const grid_dir dir = GRID_DIRS_ALL[_arrow_dir_idx];

        if (_arrow_end.move(dir))
        {
            _move3 = _arrow_end.get_point();

            if (_point_is_empty(_move3))
                return true;
        }

        // Otherwise increment direction
        _arrow_dir_idx++;
        if (_arrow_dir_idx >= N_DIRS)
            return false;

        _arrow_end.set_coord(_queen_end.get_coord());
    }

    return false;
}

inline bool amazons_move_generator::_point_is_empty(int point) const
{
    if (point == _move1)
        return true;

    return _game.at(point) == EMPTY;
}
