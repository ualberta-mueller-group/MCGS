#include "toppling_dominoes.h"

#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <utility>

#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"
#include "strip.h"
#include "throw_assert.h"

using namespace std;

////////////////////////////////////////////////// Helper functions
namespace {

bool only_legal_colors(const std::vector<int>& board)
{
    for (const int& tile : board)
        if (!(tile == BLACK || tile == WHITE || tile == BORDER))
            return false;

    return true;
}

} // namespace

//////////////////////////////////////////////////
//class toppling_dominoes_move_generator

class toppling_dominoes_move_generator: public move_generator
{
public:
    toppling_dominoes_move_generator(const toppling_dominoes& g, bw to_play);

    void operator++() override;
    operator bool() const override;
    ::move gen_move() const override;


private:
    void _increment(bool init);

    const toppling_dominoes& _g;

    int _domino_idx;
    bool _topple_right;
};

////////////////////////////////////////////////// toppling_dominoes methods

toppling_dominoes::toppling_dominoes(const vector<int>& board)
    : _initial_dominoes(board)
{
    THROW_ASSERT(only_legal_colors(_initial_dominoes));

    _domino_start = 0;
    _domino_end = _initial_dominoes.size();
    _domino_flip_orientation = false;
}

toppling_dominoes::toppling_dominoes(const string& game_as_string)
    : _initial_dominoes(strip_utils::string_to_board(game_as_string))
{
    THROW_ASSERT(only_legal_colors(_initial_dominoes));

    _domino_start = 0;
    _domino_end = _initial_dominoes.size();
    _domino_flip_orientation = false;
}

void toppling_dominoes::play(const ::move& m, bw to_play)
{
    game::play(m, to_play);
    _index_stack.emplace_back(_domino_start, _domino_end);

    // Compute state update
    const pair<int, int> new_bounds = _move_to_bounds(m, to_play);
    const int& new_start = new_bounds.first;
    const int& new_end = new_bounds.second;

    assert(
        (_domino_start <= new_start) && //
        (new_end <= _domino_end)        //
    );

    assert(_domino_start != new_start || _domino_end != new_end);

    _domino_start = new_start;
    _domino_end = new_end;
}

void toppling_dominoes::undo_move()
{
    assert(
        num_moves_played() > 0 &&                                            //
        static_cast<unsigned int>(num_moves_played()) == _index_stack.size() //
    );

    game::undo_move();
    const pair<int, int> bounds_prev = _index_stack.back();
    _index_stack.pop_back();

    const int& new_start = bounds_prev.first;
    const int& new_end = bounds_prev.second;

    assert(
        (_domino_start >= new_start) && //
        (new_end >= _domino_end)        //
    );

    assert(_domino_start != new_start || _domino_end != new_end);

    _domino_start = new_start;
    _domino_end = new_end;
}

move_generator* toppling_dominoes::create_move_generator(bw to_play) const
{
    return new toppling_dominoes_move_generator(*this, to_play);
}

void toppling_dominoes::print(ostream& str) const
{
    str << "toppling_dominoes:";

    const int len = n_dominoes();
    for (int i = 0; i < len; i++)
        str << color_to_char(get_domino_at(i));
}

void toppling_dominoes::print_move(std::ostream& str, const ::move& m, ebw to_play) const
{
    assert(is_black_white(to_play));

    const int domino_virtual = cgt_move::move2_get_part_1(m);
    const int topple_right_virtual_int = cgt_move::move2_get_part_2(m);

    assert(topple_right_virtual_int == false || //
           topple_right_virtual_int == true);   //

    const bool topple_right_virtual = static_cast<bool>(topple_right_virtual_int);

    str << (domino_virtual + 1) << (topple_right_virtual ? 'R' : 'L');
}

game* toppling_dominoes::inverse() const
{
    vector<int> inv_board;

    const int len = n_dominoes();
    inv_board.reserve(len);

    for (int i = 0; i < len; i++)
    {
        const int tile = get_domino_at(i);
        int tile_inv = tile;

        if (tile != BORDER)
            tile_inv = opponent(tile);

        inv_board.push_back(tile_inv);
    }

    // TODO move board
    return new toppling_dominoes(inv_board);
}

game* toppling_dominoes::clone() const
{
    return new toppling_dominoes(*this);
}

const vector<int> toppling_dominoes::current_dominoes() const
{
    vector<int> dominoes;

    const int SIZE = n_dominoes();
    for (int i = 0; i < SIZE; i++)
        dominoes.push_back(get_domino_at(i));

    return dominoes;
}

pair<int, int> toppling_dominoes::_move_to_bounds(::move m, bw to_play) const
{
    assert(is_black_white(to_play));

    const int domino_virtual = cgt_move::move2_get_part_1(m);
    const int topple_right_virtual_int = cgt_move::move2_get_part_2(m);

    assert(topple_right_virtual_int == false || //
           topple_right_virtual_int == true);   //

    const int domino_real = _idx_virtual_to_real(domino_virtual);

    const bool topple_right_virtual =
        static_cast<bool>(topple_right_virtual_int);

    const bool topple_right_real =
        _domino_flip_orientation ? !topple_right_virtual : topple_right_virtual;

    const int& real_start = _domino_start;
    const int& real_end = _domino_end;

    assert(_initial_dominoes[domino_real] == BORDER ||
           _initial_dominoes[domino_real] == to_play);

    if (!topple_right_real)
        return {domino_real + 1, real_end}; // LEFT
    else
        return {real_start, domino_real}; // RIGHT
}

void toppling_dominoes::_init_hash(local_hash& hash) const
{
    const int len = n_dominoes();

    for (int i = 0; i < len; i++)
        hash.toggle_value(i, get_domino_at(i));
}

void toppling_dominoes::_normalize_impl()
{
    const bool old_orientation = _domino_flip_orientation;
    bool should_flip = false;

    const int len = n_dominoes();

    for (int i = 0; i < len; i++)
    {
        assert(_domino_flip_orientation == old_orientation);
        const int tile_now = get_domino_at(i);

        _flip();
        const int tile_flipped = get_domino_at(i);
        _flip();

        if (tile_flipped == tile_now)
            continue;

        should_flip = tile_flipped < tile_now;
        break;
    }

    assert(_domino_flip_orientation == old_orientation);

    _normalize_did_flip.push_back(should_flip);

    if (should_flip)
        _flip();
    else if (_hash_updatable())
        _mark_hash_updated();
}

void toppling_dominoes::_undo_normalize_impl()
{
    const bool should_flip = _normalize_did_flip.back();
    _normalize_did_flip.pop_back();

    if (!should_flip)
    {
        if (_hash_updatable())
            _mark_hash_updated();

        return;
    }

    _flip();
}

relation toppling_dominoes::_order_impl(const game* rhs) const
{
    const toppling_dominoes* other =
        reinterpret_cast<const toppling_dominoes*>(rhs);

    assert(dynamic_cast<const toppling_dominoes*>(rhs) == other);

    const int len1 = n_dominoes();
    const int len2 = other->n_dominoes();

    if (len1 != len2)
        return len1 < len2 ? REL_LESS : REL_GREATER;


    for (int i = 0; i < len1; i++)
    {
        const int val1 = get_domino_at(i);
        const int val2 = other->get_domino_at(i);

        if (val1 != val2)
            return val1 < val2 ? REL_LESS : REL_GREATER;
    }

    return REL_EQUAL;
}

//////////////////////////////////////////////////
// toppling_dominoes_move_generator methods

toppling_dominoes_move_generator::toppling_dominoes_move_generator(
    const toppling_dominoes& g, bw to_play)
    : move_generator(to_play),
      _g(g)
{
    _increment(true);
}

void toppling_dominoes_move_generator::operator++()
{
    assert(*this);
    _increment(false);
}

toppling_dominoes_move_generator::operator bool() const
{
    return _domino_idx < _g.n_dominoes();
}

::move toppling_dominoes_move_generator::gen_move() const
{
    assert(*this);

    // Virtual idx
    return cgt_move::move2_create(_domino_idx, _topple_right);
}

void toppling_dominoes_move_generator::_increment(bool init)
{
    assert(init || *this);

    if (init)
    {
        _domino_idx = 0;
        _topple_right = false;
    }
    else
    {
        if (!_topple_right)
            _topple_right = true;
        else
        {
            _topple_right = false;
            _domino_idx++;
        }
    }

    const bw color = to_play();

    while (_domino_idx < _g.n_dominoes())
    {
        const int tile = _g.get_domino_at(_domino_idx);

        if (tile == color || tile == BORDER)
            break;

        _topple_right = false;
        _domino_idx++;
    }
}

