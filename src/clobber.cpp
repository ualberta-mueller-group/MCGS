#include "clobber.h"
#include "cgt_basics.h"
#include <ostream>
#include "game.h"
#include "grid.h"
#include <string>
#include "cgt_move.h"
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <utility>
#include <vector>
#include "grid_utils.h"

using namespace std;

////////////////////////////////////////////////// move generator
class clobber_move_generator : public move_generator
{
public:
    clobber_move_generator(const clobber& game, bw to_play);

    void operator++() override;
    operator bool() const override;
    ::move gen_move() const override;

private:
    void _next_move(bool init);

    bool _increment();
    bool _increment_dir();

    const clobber& _game;
    grid_location _location;
    size_t _dir_idx;

    int _location_point;
    int _target_point;
    bool _has_move;
};

////////////////////////////////////////////////// clobber
clobber::clobber(int n_rows, int n_cols) : grid(n_rows, n_cols)
{
}

clobber::clobber(const vector<int>& board, int_pair shape)
    : grid(board, shape)
{
}

clobber::clobber(const string& game_as_string) : grid(game_as_string)
{
}

void clobber::play(const ::move& m, bw to_play)
{
    game::play(m, to_play);

    const int from = cgt_move::from(m);
    const int to = cgt_move::to(m);

    const bw opp = opponent(to_play);
    assert(checked_is_color(from, to_play));
    assert(checked_is_color(to, opp));

    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();
        hash.toggle_value(2 + from, to_play);
        hash.toggle_value(2 + to, opp);

        hash.toggle_value(2 + from, EMPTY);
        hash.toggle_value(2 + to, to_play);
        _mark_hash_updated();
    }

    replace(from, EMPTY);
    replace(to, to_play);
}

void clobber::undo_move()
{
    const ::move m_enc = last_move();
    game::undo_move();

    bw to_play;
    int to;
    int from = cgt_move::decode3(m_enc, &to, &to_play);

    const bw opp = opponent(to_play);

    assert(checked_is_color(from, EMPTY));
    assert(checked_is_color(to, to_play));

    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();
        hash.toggle_value(2 + from, EMPTY);
        hash.toggle_value(2 + to, to_play);

        hash.toggle_value(2 + from, to_play);
        hash.toggle_value(2 + to, opp);
        _mark_hash_updated();
    }

    replace(from, to_play);
    replace(to, opp);
}

bool clobber::is_move(const int& from, const int& to, bw to_play) const
{
    assert(is_black_white(to_play));

    const bw opp = opponent(to_play);
    return (at(from) == to_play) && (at(to) == opp);
}

#ifdef CLOBBER_SPLIT
namespace {

bool trim_game(vector<int>& board_dst, int_pair& shape_dst,
               const vector<int>& board_src, const int_pair& shape_src)
{
    assert((&board_dst != &board_src) && (&shape_dst != &shape_src));

    const int MAX_INVALID = -1;
    const int MIN_INVALID = max(shape_src.first, shape_src.second) + 1;

    // Find new shape
    int min_r = MIN_INVALID;
    int min_c = MIN_INVALID;

    int max_r = MAX_INVALID;
    int max_c = MAX_INVALID;

    for (grid_location loc(shape_src); loc.valid(); loc.increment_position())
    {
        int src_point = loc.get_point();

        if (board_src[src_point] != EMPTY)
        {
            int_pair src_coord = loc.get_coord();

            min_r = min(min_r, src_coord.first);
            max_r = max(max_r, src_coord.first);

            min_c = min(min_c, src_coord.second);
            max_c = max(max_c, src_coord.second);
        }
    }

    assert(min_r < MIN_INVALID &&
            min_c < MIN_INVALID &&
            max_r > MAX_INVALID &&
            max_c > MAX_INVALID);

    assert((min_r <= max_r) && (min_c <= max_c));

    shape_dst.first = max_r - min_r + 1;
    shape_dst.second = max_c - min_c + 1;

    assert(!grid_location::shape_is_empty(shape_dst));

    if (shape_dst == shape_src)
        return false;

    board_dst.clear();
    board_dst.resize(shape_dst.first * shape_dst.second, EMPTY);

    for (grid_location loc(shape_src); loc.valid(); loc.increment_position())
    {
        const int src_point = loc.get_point();
        const int tile = board_src[src_point];

        if (tile == EMPTY)
            continue;

        assert(is_black_white(tile));

        const int_pair src_coord = loc.get_coord();

        const int_pair dst_coord(src_coord.first - min_r, src_coord.second - min_c);
        const int dst_point = grid_location::coord_to_point(dst_coord, shape_dst);

        board_dst[dst_point] = tile;
    }

    return true;
}

} // namespace

split_result clobber::_split_impl() const
{
    if (size() == 0)
        return {};

    split_result result = split_result(vector<game*>());

    const int_pair grid_shape = shape();
    const int grid_size = size();

    vector<bool> closed_set(grid_size, false);

    vector<grid_location> open_stack;

    constexpr int BLACK_MASK = 1;
    constexpr int WHITE_MASK = 1 << 1;
    constexpr int FULL_MASK = BLACK_MASK | WHITE_MASK;
    int component_color_mask = 0;

    vector<vector<int>> component_vec;
    vector<int> component;

    auto reset_vars = [&]() -> void
    {
        assert(open_stack.empty());
        component_color_mask = 0;

        if (component.empty())
        {
            component.resize(grid_size, EMPTY);
        }
        else
        {
            for (int& val : component)
                val = EMPTY;
        }
    };

    for (grid_location start(grid_shape); start.valid();
         start.increment_position())
    {

        int start_point = start.get_point();
        if (closed_set[start_point])
            continue;

        int start_color = at(start_point);
        if (start_color == EMPTY)
            continue;

        reset_vars();
        open_stack.push_back(start);

        while (!open_stack.empty())
        {
            // Get next location, close it
            grid_location loc1 = open_stack.back();
            open_stack.pop_back();

            int point1 = loc1.get_point();
            if (closed_set[point1])
                continue;
            closed_set[point1] = true;

            int color1 = at(point1);
            if (color1 == EMPTY)
                continue;
            else if (color1 == BLACK)
                component_color_mask |= BLACK_MASK;
            else if (color1 == WHITE)
                component_color_mask |= WHITE_MASK;
            else
                assert(false);

            component[point1] = color1;

            // Get neighbors
            grid_location loc2 = loc1;
            for (grid_dir dir : GRID_DIRS_CARDINAL)
            {
                if (loc2.move(dir))
                {
                    open_stack.push_back(loc2);
                    loc2 = loc1;
                }
            }
        }

        if (component_color_mask == FULL_MASK)
        {
            component_vec.emplace_back(std::move(component));
            assert(component.empty());
        }
    }

    assert(result.has_value() && result->empty());

    // Empty or "dead" board with non-zero area
    if (component_vec.size() == 0)
        return result;

    int_pair new_shape;
    vector<int> new_board;
    new_board.reserve(board_const().size());

    result->reserve(component_vec.size());

    // Trim all components
    const size_t n_components = component_vec.size();
    for (size_t i = 0; i < n_components; i++)
    {
        const vector<int>& board_untrimmed = component_vec[i];

        const bool different = trim_game(new_board, new_shape, board_untrimmed,
                                         grid_shape);

        clobber* g_new = nullptr;

        if (!different)
        {
            if (i == 0 && component_vec.size() == 1)
            {
                const vector<int>& boardconst = board_const();
                assert(board_untrimmed.size() == boardconst.size());

                if (board_untrimmed == boardconst)
                    return {};
            }

            g_new = new clobber(board_untrimmed, grid_shape);
        }
        else
        {
            assert(!grid_location::shape_is_empty(new_shape));
            g_new = new clobber(new_board, new_shape);
        }

        result->push_back(g_new);
    }

    return result;
}
#endif

move_generator* clobber::create_move_generator(bw to_play) const
{
    return new clobber_move_generator(*this, to_play);
}

void clobber::print(ostream& str) const
{
    str << "clobber:" << board_as_string();
}

game* clobber::inverse() const
{
    return new clobber(inverse_board(), shape());
}

////////////////////////////////////////////////// move generator implementation
clobber_move_generator::clobber_move_generator(const clobber& game, bw to_play)
    : move_generator(to_play),
      _game(game),
      _location(game.shape()),
      _dir_idx(0),
      _location_point(0),
      _target_point(0),
      _has_move(false)
{
    if (_game.size() != 0)
        _next_move(true);
}

void clobber_move_generator::operator++()
{
    assert(*this);
    _next_move(false);
}

clobber_move_generator::operator bool() const
{
    return _has_move;
}

::move clobber_move_generator::gen_move() const
{
    assert(*this);
    assert(_location.valid());

    return cgt_move::two_part_move(_location_point, _target_point);
}

void clobber_move_generator::_next_move(bool init)
{
    assert(init || *this);
    assert(_dir_idx < GRID_DIRS_CARDINAL.size());

    _has_move = false;

    if (!_location.valid())
        return;

    if (!init && !_increment())
        return;

    const int player = to_play();

    while (true)
    {
        const grid_dir dir = GRID_DIRS_CARDINAL[_dir_idx];

        bool valid_neighbor = _location.get_neighbor_point(_target_point, dir);
        if (valid_neighbor)
        {
            _location_point = _location.get_point();
            _has_move = _game.is_move(_location_point, _target_point, player);
        }

        if (_has_move || !_increment())
            break;
    }
}

inline bool clobber_move_generator::_increment()
{
    if (_increment_dir())
        return true;

    _dir_idx = 0;
    _location.increment_position();
    return _location.valid();
}

inline bool clobber_move_generator::_increment_dir()
{
    assert(_dir_idx < GRID_DIRS_CARDINAL.size());
    _dir_idx++;
    return _dir_idx < GRID_DIRS_CARDINAL.size();
}
