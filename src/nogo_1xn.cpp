//---------------------------------------------------------------------------
// Implementation of NoGo on a 1-dimensional 1xn board
//---------------------------------------------------------------------------
#include "nogo_1xn.h"

#include <cassert>
#include <cstddef>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>

#include "cgt_move_new.h"
#include "print_move_helpers.h"
#include "throw_assert.h"
#include "iobuffer.h"
#include "cgt_basics.h"
#include "game.h"
#include "strip.h"
#include "utilities.h"

using std::string, std::pair, std::unique_ptr;
using std::vector;

//////////////////////////////////////// helper functions
namespace {
vector<int> block_simplify(const vector<int>& board)
{
    vector<int> result;
    const int N = board.size();
    result.reserve(N);

    int prev = EMPTY;

    for (int i = 0; i < N; i++)
    {
        const int& tile = board[i];

        if (tile == EMPTY || tile != prev)
            result.push_back(tile);

        prev = tile;
    }

    return result;
}

bool only_legal_colors(const std::vector<int>& board)
{
    for (const int& x : board)
        if (!is_empty_black_white(x))
            return false;
    return true;
}

} // namespace

//////////////////////////////////////// nogo_1xn
nogo_1xn::nogo_1xn(string game_as_string) : strip(game_as_string)
{
    THROW_ASSERT(only_legal_colors(board_const()));
#ifdef NOGO_DEBUG
    THROW_ASSERT(is_legal());
#endif
}

nogo_1xn::nogo_1xn(const vector<int>& board) : strip(board)
{
    THROW_ASSERT(only_legal_colors(board_const()));
#ifdef NOGO_DEBUG
    THROW_ASSERT(is_legal());
#endif
}

bool nogo_1xn::is_legal() const
{
    const int N = size();

    if (N == 0)
        return true;

    int p1, p2;
    bool left_reaches_empty = false;

    for (int i = 0; i < N; i++)
    {
        p1 = at(i);
        p2 = (i == (N - 1)) ? BORDER : at(i + 1);

        if (p1 == EMPTY || p2 == EMPTY)
        {
            left_reaches_empty = true;
            continue;
        }

        if (p1 == p2)
        {
            continue;
        }

        if (!left_reaches_empty)
            return false;

        left_reaches_empty = false;
    }

    return true;
}

void nogo_1xn::play(const move& m, bw to_play)
{
    game::play(m, to_play);

    const int to = cgt_move_new::move1_get_part_1(m);
    assert(at(to) == EMPTY);

    // incremental hash
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();
        hash.toggle_value(to, EMPTY);
        hash.toggle_value(to, to_play);
        _mark_hash_updated();
    }

    replace(to, to_play);
}

void nogo_1xn::undo_move()
{
    const move mc = last_move();
    game::undo_move();

    const int to = cgt_move_new::move1_get_part_1(mc);
    const bw player = cgt_move_new::get_color(mc);
    assert(at(to) == player);

    // incremental hash
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();
        hash.toggle_value(to, player);
        hash.toggle_value(to, EMPTY);
        _mark_hash_updated();
    }

    replace(to, EMPTY);
}

void nogo_1xn::save_impl(obuffer& os) const
{
    _save_board(os, board_const());
}

dyn_serializable* nogo_1xn::load_impl(ibuffer& is)
{
    return new nogo_1xn(_load_board(is));
}

/*
   implements "XO split" from
   Henry's paper
*/
split_result nogo_1xn::_split_impl() const
{
    if (board_const().empty())
        return {};

    vector<int> simplified = block_simplify(board_const());
    const size_t N = simplified.size();

    vector<pair<size_t, size_t>> subgame_ranges;
    size_t subgame_start = 0;

    for (size_t i = 0; i < N; i++)
    {
        const int& color = simplified[i];
        const int& color_prev = i > 0 ? simplified[i - 1] : EMPTY;

        if (color != EMPTY && color_prev == opponent(color))
        {
            // found an XO or OX split
            subgame_ranges.emplace_back(subgame_start, i - subgame_start);
            subgame_start = i;
        }
    }

    // remainder at end
    if (N > 0)
        subgame_ranges.emplace_back(subgame_start, N - subgame_start);

    if (subgame_ranges.size() < 2)
        return {};

    split_result result = split_result(vector<game*>());

    for (const pair<size_t, size_t>& range : subgame_ranges)
        result->push_back(
            new nogo_1xn(vector_substr(simplified, range.first, range.second)));

    return result;
}

void nogo_1xn::_normalize_impl()
{
    std::vector<int> simplified = block_simplify(board_const());

    const bool size_changed = simplified.size() != board_const().size();
    const bool do_mirror = strip::_should_mirror(simplified);

    if (!size_changed && !do_mirror)
    {
        if (_hash_updatable())
            _mark_hash_updated();

        _normalize_did_change.push_back(false);
        return;
    }

    if (do_mirror)
        simplified = vector_reversed(simplified);

    _normalize_did_change.push_back(true);
    _normalize_boards.push_back(board_const());
    _set_board(simplified);
}

void nogo_1xn::_undo_normalize_impl()
{
    const bool did_change = _normalize_did_change.back();
    _normalize_did_change.pop_back();

    if (!did_change)
    {
        if (_hash_updatable())
            _mark_hash_updated();

        return;
    }

    _set_board(_normalize_boards.back());
    _normalize_boards.pop_back();
}

game* nogo_1xn::inverse() const
{
    return new nogo_1xn(inverse_board());
}

void nogo_1xn::print_move(std::ostream& str, const move& m) const
{
    // to
    print_move1_as_points(str, m);
}

std::ostream& operator<<(std::ostream& out, const nogo_1xn& g)
{
    out << g.board_as_string();
    return out;
}

//////////////////////////////////////// nogo_1xn_move_generator
class nogo_1xn_move_generator : public move_generator
{
public:
    nogo_1xn_move_generator(const nogo_1xn& game, bw to_play);
    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    int _at(int p) const { return _game.at(p); }

    bool _is_legal(int p) const;
    void _find_next_move();

    const nogo_1xn& _game;
    int _current; // current stone location to test
};

inline nogo_1xn_move_generator::nogo_1xn_move_generator(const nogo_1xn& game,
                                                        bw to_play)
    : move_generator(to_play), _game(game), _current(0)
{
    if (_game.size() > 0 && !_is_legal(_current))
        _find_next_move();
}

void nogo_1xn_move_generator::operator++()
{
    _find_next_move();
}

inline void nogo_1xn_move_generator::_find_next_move()
{
    _current++;

    int num = (int) _game.size();
    while (_current < num && !_is_legal(_current))
    {
        _current++;
    }
}

inline bool nogo_1xn_move_generator::_is_legal(int p) const
{
    if (_at(p) != EMPTY)
        return false;

    int num = _game.size();

    int current, previous = (p == 0) ? to_play() : _game.at(0);
    bool has_liberty = previous == EMPTY;
    for (int i = 1; i < num; i++)
    {
        current = (p == i) ? to_play() : _game.at(i);

        if (current == EMPTY)
        {
            has_liberty = true;
        }
        else if (current != previous && previous != EMPTY)
        {
            if (has_liberty)
            {
                has_liberty = false;
            }
            else
            {
                return false;
            }
        }

        previous = current;
    }
    if (!has_liberty) // last block is not empty and has no liberty
        return false;

    return true;
}

nogo_1xn_move_generator::operator bool() const
{
    return _current < _game.size();
}

move nogo_1xn_move_generator::gen_move() const
{
    assert(operator bool());
    return cgt_move_new::move1_create(_current);
}

//---------------------------------------------------------------------------

move_generator* nogo_1xn::create_move_generator(bw to_play) const
{
    return new nogo_1xn_move_generator(*this, to_play);
}

//---------------------------------------------------------------------------
