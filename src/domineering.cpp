#include "domineering.h"
#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"
#include "throw_assert.h"
#include "grid_location.h"

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

    grid_location _location;
    bool _has_move;
    int _point1;
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

} // namespace


////////////////////////////////////////////////// domineering methods
domineering::domineering(int n_rows, int n_cols): grid(n_rows, n_cols)
{
}

domineering::domineering(const std::vector<int>& board, int_pair shape) :
    grid(board, shape)
{
    THROW_ASSERT(has_only_valid_colors(board));
}

domineering::domineering(const std::string& game_as_string):
    grid(game_as_string)
{
    THROW_ASSERT(has_only_valid_colors(game_as_string));
}

void domineering::play(const ::move& m, bw to_play)
{
    game::play(m, to_play);

    const int point1 = cgt_move::from(m);
    const int point2 = cgt_move::to(m);

    // Both points within grid, with vertical/orientation for BLACK/WHITE
    assert(move_has_legal_orientation(point1, point2, to_play, shape()));

    // Grid empty at move location
    assert(checked_is_color(point1, EMPTY));
    assert(checked_is_color(point2, EMPTY));

    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        // Remove EMPTY from hash
        hash.toggle_value(2 + point1, EMPTY);
        hash.toggle_value(2 + point2, EMPTY);

        // Add BORDER to hash
        hash.toggle_value(2 + point1, BORDER);
        hash.toggle_value(2 + point2, BORDER);

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
    int point2;
    int point1 = cgt_move::decode3(m_enc, &point2, &to_play);

    assert(move_has_legal_orientation(point1, point2, to_play, shape()));
    assert(checked_is_color(point1, BORDER));
    assert(checked_is_color(point2, BORDER));

    if (_hash_updatable())
    {
        local_hash &hash = _get_hash_ref();

        // Remove BORDER
        hash.toggle_value(2 + point1, BORDER);
        hash.toggle_value(2 + point2, BORDER);

        // Add EMPTY
        hash.toggle_value(2 + point1, EMPTY);
        hash.toggle_value(2 + point2, EMPTY);

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

/*
split_result domineering::_split_impl() const
{

}
*/

//////////////////////////////////////////////////
// domineering_move_generator methods

domineering_move_generator::domineering_move_generator(const domineering& g,
                                                       bw to_play)
    : move_generator(to_play),
      _game(g),
      _orientation_dir(to_play == BLACK ? GRID_DIR_DOWN : GRID_DIR_RIGHT),
      _location(g.shape())
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
    return cgt_move::two_part_move(_point1, _point2);
}

void domineering_move_generator::_increment(bool init)
{
    assert(init || *this);

    if (init)
    {
        assert(!_location.valid() || _location.get_point() == 0);
    }
    else
        _location.increment_position();

    // Find legal move
    _has_move = false;

#define GEN_LOOP_CONTINUE()             \
    {                                   \
        _location.increment_position(); \
        continue;                       \
    }                                   \

    // Check if current location has a legal move, increment if not
    while (_location.valid())
    {
        // Point 1 empty
        _point1 = _location.get_point();

        if (_game.at(_point1) != EMPTY)
            GEN_LOOP_CONTINUE();

        // Point2 exists, is empty
        bool point2_valid =
            _location.get_neighbor_point(_point2, _orientation_dir);

        if (!point2_valid || _game.at(_point2) != EMPTY)
            GEN_LOOP_CONTINUE();

        _has_move = true;
        break;
    }
}
