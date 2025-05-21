//---------------------------------------------------------------------------
// Implementation of NoGo on a 2-dimensional rectangular board
//---------------------------------------------------------------------------
#include "nogo.h"

#include "cgt_basics.h"
#include "game.h"
#include "grid.h"
#include <string>
#include <vector>
#include <cassert>
#include <ostream>
#include "grid_utils.h"
#include "throw_assert.h"

//////////////////////////////////////// nogo
nogo::nogo(std::string game_as_string) : grid(game_as_string)
{
#ifdef NOGO_DEBUG
    THROW_ASSERT(is_legal());
#endif
}

nogo::nogo(const std::vector<int>& board, int_pair shape) : grid(board, shape)
{
#ifdef NOGO_DEBUG
    THROW_ASSERT(is_legal());
#endif
}

void nogo::play(const move& m, bw to_play)
{
    game::play(m, to_play);

    const int to = m;
    assert(at(to) == EMPTY);

    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        hash.toggle_value(2 + to, EMPTY);
        hash.toggle_value(2 + to, to_play);

        _mark_hash_updated();
    }

    replace(to, to_play);
}

void nogo::undo_move()
{
    const move mc = last_move();
    game::undo_move();

    const int to = cgt_move::decode(mc);
    const bw player = cgt_move::get_color(mc);
    assert(at(to) == player);

    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        hash.toggle_value(2 + to, player);
        hash.toggle_value(2 + to, EMPTY);

        _mark_hash_updated();
    }

    replace(to, EMPTY);
}

bool nogo::is_legal() const
{
    const int N = size();
    const int_pair& grid_shape = shape();

    if (N == 0)
        return true;

    static_assert(BLACK == 0 && WHITE == 1 && EMPTY == 2);
    static const int MASKS[] = {1, 2, 0};
    static const int FULL_MASK = 3;

    std::vector<bool> closed(N, false);
    std::vector<grid_location> open;

    int n_connected = 0; // legal IFF n_connected reaches N

    grid_location start(grid_shape);

    do
    {
        // Find next EMPTY tile that hasn't been closed
        assert(open.empty());
        const int start_point = start.get_point();
        if (closed[start_point] || at(start_point) != EMPTY)
            continue;

        open.push_back(start);
        closed[start_point] = true;
        n_connected++;
        if (n_connected == N)
            return true;

        while (!open.empty())
        {
            const grid_location loc1 = open.back();
            open.pop_back();

            const int point1 = loc1.get_point();
            const int color1 = at(point1);
            assert(is_empty_black_white(color1));
            const int mask1 = MASKS[color1];
            assert(closed[point1]);

            for (grid_dir dir : GRID_DIRS_CARDINAL)
            {
                grid_location loc2 = loc1;
                if (!loc2.move(dir))
                    continue;

                const int point2 = loc2.get_point();
                if (closed[point2])
                    continue;

                const int color2 = at(point2);
                assert(is_empty_black_white(color2));
                const int mask2 = MASKS[color2];
                if ((mask1 | mask2) == FULL_MASK) // tiles are BLACK and WHITE
                    continue;

                open.push_back(loc2);
                closed[point2] = true;
                n_connected++;
                if (n_connected == N)
                    return true;
            }
        }
    } while (start.increment_position());

    assert(n_connected <= N);
    return n_connected == N;
}

split_result nogo::_split_impl() const
{
    return split_result(); // no split
}

game* nogo::inverse() const
{
    return new nogo(inverse_board(), shape());
}

std::ostream& operator<<(std::ostream& out, const nogo& g)
{
    out << g.board_as_string();
    return out;
}

//////////////////////////////////////// nogo_rule
bool nogo_rule::is_legal(nogo_board nboard, int p, int toplay)
{
    std::vector<int>& board = nboard.board;

    if (board[p] != EMPTY)
        return false;

    board[p] = toplay;
    int opp_color = 1 - toplay;

    if (!nogo_rule::has_liberty(nboard, p))
    {
        board[p] = EMPTY;
        return false;
    }

    std::vector<int> nbrs = nogo_rule::neighbors(nboard, p);
    for (int nbr : nbrs)
    {
        if (board[nbr] == opp_color && !nogo_rule::has_liberty(nboard, nbr))
        {
            board[p] = EMPTY;
            return false;
        }
    }

    board[p] = EMPTY;
    return true;
}

bool nogo_rule::has_liberty(const nogo_board& nboard, int p)
{
    const std::vector<int>& board = nboard.board;

    int size = (int) board.size();
    std::vector<bool> markers = std::vector<bool>(size, false);

    int color = board[p];
    std::vector<int> point_stack = {p};
    markers[p] = true;

    while (!point_stack.empty())
    {
        int p = point_stack.back();
        point_stack.pop_back();

        std::vector<int> nbrs = nogo_rule::neighbors(nboard, p);
        for (int nbr : nbrs)
        {
            if (board[nbr] == EMPTY)
                return true;
            if (board[nbr] == color && !markers[nbr])
            {
                point_stack.push_back(nbr);
                markers[nbr] = true;
            }
        }
    }

    return false;
}

std::vector<int> nogo_rule::neighbors(const nogo_board& nboard, int p)
{
    const std::vector<int> board = nboard.board;
    int n_rows = nboard.shape.first;
    int n_cols = nboard.shape.second;

    int r = p / nboard.shape.second;
    int c = p % nboard.shape.second;

    std::vector<int> nbrs;
    if (r > 0 && board[p - n_cols] != BORDER)
        nbrs.push_back(p - n_cols); // up
    if (r < n_rows - 1 && board[p + n_cols] != BORDER)
        nbrs.push_back(p + n_cols); // down
    if (c > 0 && board[p - 1] != BORDER)
        nbrs.push_back(p - 1); // left
    if (c < n_cols - 1 && board[p + 1] != BORDER)
        nbrs.push_back(p + 1); // right

    return nbrs;
}

//---------------------------------------------------------------------------

class nogo_move_generator : public move_generator
{
public:
    nogo_move_generator(const nogo& game, bw to_play);
    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    void _find_next_move();
    bool _is_legal();

    const nogo& _game;
    int _current; // current stone location to test
};

inline nogo_move_generator::nogo_move_generator(const nogo& game, bw to_play)
    : move_generator(to_play), _game(game), _current(0)
{
    if (_game.size() > 0 && !_is_legal())
    {
        _find_next_move();
    }
}

void nogo_move_generator::operator++()
{
    _find_next_move();
}

inline void nogo_move_generator::_find_next_move()
{
    _current++;

    int num = (int) _game.size();
    while (_current < num && !_is_legal())
    {
        _current++;
    }
}

inline bool nogo_move_generator::_is_legal()
{
    return nogo_rule::is_legal({_game.board(), _game.shape()}, _current,
                               to_play());
}

nogo_move_generator::operator bool() const
{
    return _current < _game.size();
}

move nogo_move_generator::gen_move() const
{
    assert(operator bool());
    return _current;
}

//---------------------------------------------------------------------------

move_generator* nogo::create_move_generator(bw to_play) const
{
    return new nogo_move_generator(*this, to_play);
}

//---------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const nogo_board& nboard)
{
    const char COLOR2CHAR[] = {'X', 'O', '.', '#'};
    int size = (int) nboard.board.size();
    int n_cols = nboard.shape.second;
    std::string sboard;
    for (int i = 0; i < size; i++)
    {
        sboard += COLOR2CHAR[nboard.board[i]];
        sboard += ((i + 1) % n_cols == 0) ? '\n' : ' ';
    }
    os << sboard;
    return os;
}
