#include "domineering.h"
#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"
#include "throw_assert.h"
#include "grid_location.h"
#include "utilities.h"

#include <cassert>
#include <vector>
#include <string>

using namespace std;

//////////////////////////////////////////////////
// class domineering_move_generator

class domineering_move_generator: public move_generator
{
public:
    domineering_move_generator(const domineering& g, bw to_play);

    void operator++() override;
    operator bool() const override;
    ::move gen_move() const override;

private:
    void _increment(bool init);

    const domineering& _game;
    const grid_dir _orientation_dir;

    bool _has_move;

    grid_location _loc1;
    int _point1;

    int_pair _coord2;
    int _point2;
};

////////////////////////////////////////////////// helper functions

namespace {
// TODO clean up grid colors, make these functions use proper grid color
// functions/definitions from headers

bool has_only_valid_colors(const vector<int>& board)
{
    for (const int& tile : board)
        if (tile != EMPTY && tile != BORDER)
            return false;

    return true;
}

bool has_only_valid_colors(const string& board_string)
{
    for (const char& c : board_string)
        if (c != '.' && c != '#' && c != '|')
            return false;

    return true;
}

bool move_has_legal_orientation(int point1, int point2, bw player,
                                const int_pair& grid_shape)
{
    assert(is_black_white(player));

    if (                                                      //
        !grid_location::point_in_shape(point1, grid_shape) || //
        !grid_location::point_in_shape(point2, grid_shape)    //
        )                                                     //
        return false;

    const int_pair coord1 = grid_location::point_to_coord(point1, grid_shape);
    const int_pair coord2 = grid_location::point_to_coord(point2, grid_shape);

    // EMPTY invalid, BLACK vertical, RIGHT horizontal
    ebw orientation = EMPTY;

    const int abs_y = abs(coord1.first - coord2.first);
    const int abs_x = abs(coord1.second - coord2.second);

    if (abs_y + abs_x == 1)
    {
        if (abs_y == 1)
            orientation = BLACK;
        else
        {
            assert(abs_x == 1);
            orientation = WHITE;
        }
    }

    return orientation != EMPTY;
}

inline int encode_domineering_coord(const int_pair& coord)
{
    const int& r = coord.first;
    const int& c = coord.second;

    assert(0 == (r & ~get_bit_mask_lower<int>(7)));
    assert(0 == (c & ~get_bit_mask_lower<int>(7)));

    return r | (c << 7);
}

inline int_pair decode_domineering_coord(int encoded)
{
    assert(0 == (encoded & ~get_bit_mask_lower<int>(14)));

    const int DOMINEERING_MASK = get_bit_mask_lower<int>(7);

    const int r = encoded & DOMINEERING_MASK;
    const int c = (encoded >> 7) & DOMINEERING_MASK;

    return {r, c};
}

} // namespace


////////////////////////////////////////////////// domineering methods
domineering::domineering(int n_rows, int n_cols): grid(n_rows, n_cols)
#ifdef USE_GRID_HASH
    , _gh(GRID_HASH_ACTIVE_MASK_MIRRORS)
#endif
{
    assert(has_only_valid_colors(board_const()));
}

domineering::domineering(const std::vector<int>& board, int_pair shape) :
    grid(board, shape)
#ifdef USE_GRID_HASH
    , _gh(GRID_HASH_ACTIVE_MASK_MIRRORS)
#endif
{
    THROW_ASSERT(has_only_valid_colors(board));
}

domineering::domineering(const std::string& game_as_string):
    grid(game_as_string)
#ifdef USE_GRID_HASH
    , _gh(GRID_HASH_ACTIVE_MASK_MIRRORS)
#endif
{
    THROW_ASSERT(has_only_valid_colors(game_as_string));
}

void domineering::play(const ::move& m, bw to_play)
{
    game::play(m, to_play);

    const int enc1 = cgt_move::from(m);
    const int enc2 = cgt_move::to(m);

    const int_pair coord1 = decode_domineering_coord(enc1);
    const int_pair coord2 = decode_domineering_coord(enc2);

    const int point1 = grid_location::coord_to_point(coord1, shape());
    const int point2 = grid_location::coord_to_point(coord2, shape());

    // Both points within grid, and vertical/horizontal for BLACK/WHITE
    assert(move_has_legal_orientation(point1, point2, to_play, shape()));

    // Grid empty at move location
    assert(checked_is_color(point1, EMPTY));
    assert(checked_is_color(point2, EMPTY));

    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

#ifdef USE_GRID_HASH
        _gh.toggle_value(coord1, EMPTY);
        _gh.toggle_value(coord2, EMPTY);

        _gh.toggle_value(coord1, BORDER);
        _gh.toggle_value(coord2, BORDER);
        hash.__set_value(_gh.get_value());
#else
        // Remove EMPTY from hash
        hash.toggle_value(2 + point1, EMPTY);
        hash.toggle_value(2 + point2, EMPTY);

        // Add BORDER to hash
        hash.toggle_value(2 + point1, BORDER);
        hash.toggle_value(2 + point2, BORDER);
#endif

        _mark_hash_updated();
    }

    replace(point1, BORDER);
    replace(point2, BORDER);
}

void domineering::undo_move()
{
    const ::move m_enc = last_move();
    game::undo_move();

    bw to_play;
    int enc2;
    int enc1 = cgt_move::decode3(m_enc, &enc2, &to_play);

    const int_pair coord1 = decode_domineering_coord(enc1);
    const int_pair coord2 = decode_domineering_coord(enc2);

    const int point1 = grid_location::coord_to_point(coord1, shape());
    const int point2 = grid_location::coord_to_point(coord2, shape());

    assert(move_has_legal_orientation(point1, point2, to_play, shape()));
    assert(checked_is_color(point1, BORDER));
    assert(checked_is_color(point2, BORDER));

    if (_hash_updatable())
    {
        local_hash &hash = _get_hash_ref();
#ifdef USE_GRID_HASH
        _gh.toggle_value(coord1, BORDER);
        _gh.toggle_value(coord2, BORDER);

        _gh.toggle_value(coord1, EMPTY);
        _gh.toggle_value(coord2, EMPTY);
        hash.__set_value(_gh.get_value());
#else
        // Remove BORDER
        hash.toggle_value(2 + point1, BORDER);
        hash.toggle_value(2 + point2, BORDER);

        // Add EMPTY
        hash.toggle_value(2 + point1, EMPTY);
        hash.toggle_value(2 + point2, EMPTY);
#endif

        _mark_hash_updated();
    }

    replace(point1, EMPTY);
    replace(point2, EMPTY);
}

move_generator* domineering::create_move_generator(bw to_play) const
{
    return new domineering_move_generator(*this, to_play);
}

game* domineering::inverse() const
{
    const int_pair& s = shape();

    int_pair transpose_shape;
    transpose_shape.first = s.second;
    transpose_shape.second = s.first;

    return new domineering(transpose_board(board_const(), s), transpose_shape);
}

////////////////////////////////////////////////// split
namespace {

struct bounding_box
{
    bounding_box(int row_shift, int col_shift, int_pair shape):
        row_shift(row_shift), col_shift(col_shift), shape(shape)
    {
    }

    int row_shift;
    int col_shift;
    int_pair shape;
};

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
            const int val = src_board[src_point + c];
            dst_board.push_back(val);
        }

        src_point += src_shape.second;
    }

    return dst_board;
}

} // namespace

// Find all 4-connected components with at least 2 spaces

#ifdef DOMINEERING_SPLIT
split_result domineering::_split_impl() const
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
    vector<grid_location> open_stack;
    vector<bool> closed_set(grid_size, false);

    // Found components
    vector<vector<int>> component_vec;
    vector<bounding_box> bounding_boxes;

    // Current component
    vector<int> component;
    size_t component_empty_count = 0;
    int min_row = MIN_INVALID;
    int max_row = MAX_INVALID;
    int min_col = MIN_INVALID;
    int max_col = MAX_INVALID;

    auto reset_component = [&]() -> void
    {
        component_empty_count = 0;

        // component
        if (component.empty())
            component.resize(grid_size, BORDER);
        else
            for (int& tile : component)
                tile = BORDER;

        // bounding box
        min_row = MIN_INVALID;
        max_row = MAX_INVALID;
        min_col = MIN_INVALID;
        max_col = MAX_INVALID;
    };

    auto mark_component = [&](int point, const int_pair& coord) -> void
    {
        component[point] = EMPTY;

        min_row = min(min_row, coord.first);
        max_row = max(max_row, coord.first);
        min_col = min(min_col, coord.second);
        max_col = max(max_col, coord.second);

        component_empty_count++;
    };

    // Main loop
    for (grid_location loc(grid_shape); loc.valid(); loc.increment_position())
    {
        const int point_start = loc.get_point();

        if (closed_set[point_start] || at(point_start) != EMPTY)
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
            assert(at(point1) == EMPTY);
            assert(closed_set[point1]);

            assert(component[point1] == BORDER);
            mark_component(point1, loc1.get_coord());

            grid_location loc2(grid_shape);
            for (grid_dir dir : GRID_DIRS_CARDINAL)
            {
                loc2.set_coord(loc1.get_coord());
                if (!loc2.move(dir))
                    continue;

                const int point2 = loc2.get_point();

                if (closed_set[point2] || at(point2) != EMPTY)
                    continue;

                closed_set[point2] = true;
                open_stack.emplace_back(loc2);
            }
        }

        if (component_empty_count >= 2)
        {
            // Save this component
            component_vec.emplace_back(std::move(component));

            assert((min_row <= max_row) && (min_col <= max_col));

            const int row_dim = max_row - min_row + 1;
            const int col_dim = max_col - min_col + 1;

            bounding_boxes.emplace_back(min_row, min_col,
                                        int_pair(row_dim, col_dim));
        }
    }

    assert(component_vec.size() == bounding_boxes.size());

    if (component_vec.size() == 1 && bounding_boxes.back().shape == grid_shape)
        return {};

    const size_t n_components = component_vec.size();
    for (size_t i = 0; i < n_components; i++)
    {
        const vector<int>& comp = component_vec[i];
        const bounding_box& box = bounding_boxes[i];

        result->push_back(new domineering(
            trim_to_bounding_box(comp, grid_shape, box), box.shape));
    }

    return result;
}
#endif

#ifdef USE_GRID_HASH
void domineering::_init_hash(local_hash& hash) const
{
    const int_pair &s = shape();

    _gh.reset(s);
    _gh.toggle_type(game_type());

    int pos = 0;
    for (int r = 0; r < s.first; r++)
    {
        for (int c = 0; c < s.second; c++)
            _gh.toggle_value(r, c, at(pos + c));

        pos += s.second;
    }

    hash.__set_value(_gh.get_value());
    //hash.toggle_value(0, _gh.get_value());
}
#endif

//////////////////////////////////////////////////
// domineering_move_generator methods

domineering_move_generator::domineering_move_generator(const domineering& g,
                                                       bw to_play)
    : move_generator(to_play),
      _game(g),
      _orientation_dir(to_play == BLACK ? GRID_DIR_DOWN : GRID_DIR_RIGHT),
      _loc1(g.shape())
{
    assert(is_black_white(to_play));
    _increment(true);
}

void domineering_move_generator::operator++()
{
    assert(*this);
    _increment(false);
}

domineering_move_generator::operator bool() const
{
    return _has_move;
}

::move domineering_move_generator::gen_move() const
{
    assert(*this);
    //return cgt_move::two_part_move(_point1, _point2);

    const int_pair& coord1 = _loc1.get_coord();
    const int_pair& coord2 = _coord2;

    const int enc1 = encode_domineering_coord(coord1);
    const int enc2 = encode_domineering_coord(coord2);

    return cgt_move::two_part_move(enc1, enc2);
}

void domineering_move_generator::_increment(bool init)
{
    assert(init || *this);

    if (init)
    {
        assert(!_loc1.valid() || _loc1.get_point() == 0);
    }
    else
        _loc1.increment_position();

    // Find legal move
    _has_move = false;

#define GEN_LOOP_CONTINUE()             \
    {                                   \
        _loc1.increment_position(); \
        continue;                       \
    }                                   \

    // Check if current location has a legal move, increment if not
    while (_loc1.valid())
    {
        // Point 1 empty
        _point1 = _loc1.get_point();

        if (_game.at(_point1) != EMPTY)
            GEN_LOOP_CONTINUE();

        // Point2 exists, is empty
        bool point2_valid =
            _loc1.get_neighbor_coord(_coord2, _orientation_dir);

        if (point2_valid)
            _point2 = grid_location::coord_to_point(_coord2, _loc1.get_shape());

        if (!point2_valid || _game.at(_point2) != EMPTY)
            GEN_LOOP_CONTINUE();

        _has_move = true;
        break;
    }
}

