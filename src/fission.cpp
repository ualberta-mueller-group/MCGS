#include "fission.h"

#include <vector>
#include <string>
#include <cassert>
#include <ostream>

#include "cgt_basics.h"
#include "cgt_move_new.h"
#include "grid.h"
#include "grid_location.h"
#include "print_move_helpers.h"
#include "throw_assert.h"

using namespace std;

////////////////////////////////////////////////// helper functions
namespace {
bool only_legal_colors(const std::vector<int>& board)
{
    for (const int& x : board)
        if (!(x == EMPTY || x == BLACK || x == BORDER))
            return false;
    return true;
}

inline void get_player_dirs(bw player, grid_dir& dir1, grid_dir& dir2)
{
    assert(is_black_white(player));

    dir1 = player == BLACK ? GRID_DIR_UP : GRID_DIR_LEFT;
    dir2 = player == BLACK ? GRID_DIR_DOWN : GRID_DIR_RIGHT;
}

} // namespace


////////////////////////////////////////////////// class fission_move_generator
class fission_move_generator: public move_generator
{
public:
    fission_move_generator(const fission& game, bw to_play);

    void operator++() override;
    operator bool() const override;
    ::move gen_move() const override;

private:
    void _increment(bool init);
    bool _is_move(const grid_location& query_loc);

    const fission& _game;
    grid_location _loc;
};

////////////////////////////////////////////////// fission methods
fission::fission(int n_rows, int n_cols)
    : grid(n_rows, n_cols, GRID_TYPE_COLOR)
#ifdef USE_GRID_HASH
      , _gh(FISSION_GRID_HASH_MASK)
#endif
{
    THROW_ASSERT(only_legal_colors(board_const()));
}

fission::fission(const vector<int>& board, int_pair shape)
    : grid(board, shape, GRID_TYPE_COLOR)
#ifdef USE_GRID_HASH
      , _gh(FISSION_GRID_HASH_MASK)
#endif
{
    THROW_ASSERT(only_legal_colors(board_const()));
}

fission::fission(const string& game_as_string)
    : grid(game_as_string, GRID_TYPE_COLOR)
#ifdef USE_GRID_HASH
      , _gh(FISSION_GRID_HASH_MASK)
#endif
{
    THROW_ASSERT(only_legal_colors(board_const()));
}

void fission::play(const ::move& m, bw to_play)
{
    game::play(m, to_play);

    // Get points

    int_pair coord_start;
    cgt_move_new::move2_unpack_coords(m, coord_start);

    grid_location loc_start(shape(), coord_start);
    assert(loc_start.valid());

    grid_dir dir1;
    grid_dir dir2;
    get_player_dirs(to_play, dir1, dir2);

    int point_start = loc_start.get_point();
    int_pair coord1, coord2;
    int point1, point2;

    {
        bool ok = true;
        ok &= loc_start.get_neighbor_coord(coord1, dir1);
        ok &= loc_start.get_neighbor_coord(coord2, dir2);
        assert(ok);

        point1 = grid_location::coord_to_point(coord1, shape());
        point2 = grid_location::coord_to_point(coord2, shape());
        
    }

    // Remove/place stones
    assert(
        at(point_start) == BLACK && //
        at(point1) == EMPTY &&      //
        at(point2) == EMPTY         //
            );

    replace(point_start, EMPTY);
    replace(point1, BLACK);
    replace(point2, BLACK);

    // Updat hash
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

#ifdef USE_GRID_HASH
        // Remove old state
        _gh.toggle_value(coord_start, BLACK);
        _gh.toggle_value(coord1, EMPTY);
        _gh.toggle_value(coord2, EMPTY);

        // Add new state
        _gh.toggle_value(coord_start, EMPTY);
        _gh.toggle_value(coord1, BLACK);
        _gh.toggle_value(coord2, BLACK);

        hash.__set_value(_gh.get_value());
#else
        // Remove old state
        hash.toggle_value(2 + point_start, BLACK);
        hash.toggle_value(2 + point1, EMPTY);
        hash.toggle_value(2 + point2, EMPTY);

        // Add new state
        hash.toggle_value(2 + point_start, EMPTY);
        hash.toggle_value(2 + point1, BLACK);
        hash.toggle_value(2 + point2, BLACK);
#endif

        _mark_hash_updated();
    }
}

void fission::undo_move()
{
    const ::move m_enc = last_move();
    game::undo_move();

    // Decode move
    const bw player = cgt_move_new::get_color(m_enc);

    // Get points
    int_pair coord_start;
    cgt_move_new::move2_unpack_coords(m_enc, coord_start);

    grid_location loc_start(shape(), coord_start);
    assert(loc_start.valid());

    grid_dir dir1;
    grid_dir dir2;
    get_player_dirs(player, dir1, dir2);

    int point_start = loc_start.get_point();
    int_pair coord1, coord2;
    int point1, point2;

    {
        bool ok = true;
        ok &= loc_start.get_neighbor_coord(coord1, dir1);
        ok &= loc_start.get_neighbor_coord(coord2, dir2);
        assert(ok);

        point1 = grid_location::coord_to_point(coord1, shape());
        point2 = grid_location::coord_to_point(coord2, shape());
    }

    // Remove/place stones
    assert(
            at(point_start) == EMPTY && //
            at(point1) == BLACK &&      //
            at(point2) == BLACK         //
        );

    replace(point_start, BLACK);
    replace(point1, EMPTY);
    replace(point2, EMPTY);

    // Update hash

    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

#ifdef USE_GRID_HASH
        // Remove old state
        _gh.toggle_value(coord_start, EMPTY);
        _gh.toggle_value(coord1, BLACK);
        _gh.toggle_value(coord2, BLACK);

        // Add new state
        _gh.toggle_value(coord_start, BLACK);
        _gh.toggle_value(coord1, EMPTY);
        _gh.toggle_value(coord2, EMPTY);

        hash.__set_value(_gh.get_value());
#else
        // Remove old state
        hash.toggle_value(2 + point_start, EMPTY);
        hash.toggle_value(2 + point1, BLACK);
        hash.toggle_value(2 + point2, BLACK);

        // Add new state
        hash.toggle_value(2 + point_start, BLACK);
        hash.toggle_value(2 + point1, EMPTY);
        hash.toggle_value(2 + point2, EMPTY);
#endif
        _mark_hash_updated();
    }
}

move_generator* fission::create_move_generator(bw to_play) const
{
    return new fission_move_generator(*this, to_play);
}

void fission::print_move(std::ostream& str, const ::move& m) const
{
    // (stone coord)
    print_move2_as_coords(str, m, shape());
}

game* fission::inverse() const
{
    const int_pair& s = shape();

    int_pair transpose_shape;
    transpose_shape.first = s.second;
    transpose_shape.second = s.first;

    return new fission(grid::transpose_board(board_const(), s),
                       transpose_shape);
}

#ifdef USE_GRID_HASH
void fission::_init_hash(local_hash& hash) const
{
    _gh.init_from_grid(*this);
    hash.__set_value(_gh.get_value());
}
#endif

//////////////////////////////////////////////////
// fission_move_generator methods

fission_move_generator::fission_move_generator(const fission& game, bw to_play)
    : move_generator(to_play),
      _game(game),
      _loc(game.shape())
{
    _increment(true);
}

void fission_move_generator::operator++()
{
    assert(*this);
    _increment(false);
}

fission_move_generator::operator bool() const
{
    return _loc.valid();
}

::move fission_move_generator::gen_move() const
{
    assert(*this);

    const int_pair& coords = _loc.get_coord();
    return cgt_move_new::move2_create_from_coords(coords);
}

void fission_move_generator::_increment(bool init)
{
    assert(init || *this);

    // TODO logical_implies
    if (init)
        assert(!_loc.valid() || _loc.get_coord() == int_pair(0, 0));
    else
        _loc.increment_position();

    if (!_loc.valid())
        return;

    for (; _loc.valid(); _loc.increment_position())
    {
        const int point = _loc.get_point();

        if (_game.at(point) != BLACK)
            continue;

        if (!_is_move(_loc))
            continue;

        break;
    }
}

bool fission_move_generator::_is_move(const grid_location& query_loc)
{
    const bw player = to_play();
    assert(is_black_white(player));

    grid_dir dir1;
    grid_dir dir2;
    get_player_dirs(player, dir1, dir2);

    int point1;
    int point2;

    if (!query_loc.get_neighbor_point(point1, dir1))
        return false;

    if (!query_loc.get_neighbor_point(point2, dir2))
        return false;

    return                           //
        _game.at(point1) == EMPTY && //
        _game.at(point2) == EMPTY;   //
}

