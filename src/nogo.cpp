//---------------------------------------------------------------------------
// Implementation of NoGo on a 2-dimensional rectangular board
//---------------------------------------------------------------------------
#include "nogo.h"

#include <string>
#include <cstddef>
#include <vector>
#include <iostream>
#include <cassert>
#include <ostream>

#include "throw_assert.h"
#include "cgt_basics.h"
#include "game.h"
#include "grid.h"

namespace {
// Remove extra rows and columns of BORDER
nogo_board shrink_board(const nogo_board& board)
{
    int n_rows = board.shape.first, n_cols = board.shape.second;
    int r_lo = n_rows-1, r_hi = 0, c_lo = n_cols-1, c_hi = 0;

    for (int r = 0; r < n_rows; r++) {
        for (int c = 0; c < n_cols; c++) {
            int p = r * n_cols + c;
            if (board[p] != BORDER) {
                r_lo = (r < r_lo) ? r : r_lo;
                r_hi = (r > r_hi) ? r : r_hi;
                c_lo = (c < c_lo) ? c : c_lo;
                c_hi = (c > c_hi) ? c : c_hi;
            }
        }
    }
    if (!(r_lo <= r_hi && c_lo <= c_hi)) {
        std::cout << r_lo << " " << r_hi << " " << c_lo << " " << c_hi << "\n";
        std::cout << board << "\n";
        assert(false);
    }
    assert(r_lo <= r_hi && c_lo <= c_hi);

    int_pair new_shape = {r_hi - r_lo + 1, c_hi - c_lo + 1};
    int new_size = new_shape.first * new_shape.second;

    std::vector<int> new_board;
    std::vector<int> new_immortal;
    for (int r = r_lo; r <= r_hi; r++) {
        for (int c = c_lo; c <= c_hi; c++) {
            int p = r * n_cols + c;
            new_board.push_back(board[p]);
            new_immortal.push_back(board.immortal[p]);
        }
    }
    assert((int)new_board.size() == new_size && (int)new_immortal.size() == new_size);

    return nogo_board(new_board, new_immortal, new_shape);
}
}

//////////////////////////////////////// nogo
nogo::nogo(std::string game_as_string) : grid(game_as_string)
{
    _immortal = std::vector<int>(size(), EMPTY);
    _immortal_copy = _immortal;
#ifdef NOGO_DEBUG
    THROW_ASSERT(is_legal());
#endif
}

nogo::nogo(const std::vector<int>& board, int_pair shape) : grid(board, shape)
{
    _immortal = std::vector<int>(size(), EMPTY);
    _immortal_copy = _immortal;
#ifdef NOGO_DEBUG
    THROW_ASSERT(is_legal());
#endif
}

nogo::nogo(const std::vector<int>& board, const std::vector<int>& immortal, int_pair shape)
    : grid(board, shape), _immortal(immortal)
{
    _immortal_copy = _immortal;
#ifdef NOGO_DEBUG
    THROW_ASSERT(is_legal());
#endif
}

void nogo::play(const move& m, bw to_play)
{
    game::play(m, to_play);

    const int to = m;
    assert(at(to) == EMPTY);

    replace(to, to_play);
    // playing on a marked 1-Go point turns to immortal
    if (_immortal[to] == to_play) {
        _immortal[to] = BORDER;
    }

    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();
        const int N = size();

        hash.toggle_value(2 + to, EMPTY);                   // update board
        hash.toggle_value(2 + to, to_play);
        hash.toggle_value(2 + N + to, _immortal_copy[to]);  // update immortal
        hash.toggle_value(2 + N + to, _immortal[to]);

        _mark_hash_updated();
    }
}

void nogo::undo_move()
{
    const move mc = last_move();
    game::undo_move();

    const int to = cgt_move::decode(mc);
    const bw player = cgt_move::get_color(mc);
    assert(at(to) == player);
    const int N = size();

    replace(to, EMPTY);
    
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        hash.toggle_value(2 + to, player);                  // update board
        hash.toggle_value(2 + to, EMPTY);
        hash.toggle_value(2 + N + to, _immortal[to]);       // update immortal
        hash.toggle_value(2 + N + to, _immortal_copy[to]);

        _mark_hash_updated();
    }

    _immortal[to] = _immortal_copy[to];
}

bool nogo::is_legal() const
{
    const int N = size();
    const nogo_board cboard(board(), immortal(), shape());  // board copy

    std::vector<bool> checked(N, false);
    for (int p = 0; p < N; p++) {
        if (checked[p] || !is_black_white(cboard[p]) || cboard.immortal[p] == BORDER)
            continue;
        
        bool legal = false;
        
        int color = cboard[p];
        std::vector<int> point_stack = { p };
        checked[p] = true;
        while (!point_stack.empty()) {
            int q = point_stack.back();
            point_stack.pop_back();

            std::vector<int> nbrs = nogo_rule::neighbors(cboard, q);
            for (int nbr : nbrs) {
                if (cboard[nbr] == EMPTY)
                    legal = true;
                if (cboard[nbr] == color && !checked[nbr]) {
                    legal |= cboard.immortal[nbr] == BORDER;
                    point_stack.push_back(nbr);
                    checked[nbr] = true;
                } 
            }
        }
        if (!legal)
            return false;
    }
    return true;
}

void nogo::_init_hash(local_hash& hash) const
{
    const int_pair board_shape = shape();
    const size_t N = size();

    // hash board shape
    hash.toggle_value(0, board_shape.first);
    hash.toggle_value(1, board_shape.second);

    // hash board
    for (size_t i = 0; i < N; i++)
        hash.toggle_value(i + 2, at(i));
    
    // hash immortal
    for (size_t i = 0; i < N; i++)
        hash.toggle_value(i + N + 2, _immortal[i]);
}

split_result nogo::_split_impl() const
{
    nogo_board temp_board(board(), immortal(), shape());
    std::vector<nogo_board> subboards = split_by_nogo::split(temp_board);

    split_result result = split_result(std::vector<game*>());
    for (auto& subboard : subboards)
    {
        result->push_back(new nogo(subboard.board, subboard.immortal, subboard.shape));
    }
    return result;
}

game* nogo::inverse() const
{
    std::vector<int> new_immortal = immortal();
    const int N = size();
    for (int p = 0; p < N; p++) {
        if (is_black_white(new_immortal[p])) {
            new_immortal[p] = opponent(new_immortal[p]);
        }
    }
    return new nogo(inverse_board(), new_immortal, shape());
}

std::ostream& operator<<(std::ostream& out, const nogo& g)
{
    out << g.board_as_string();
    return out;
}

//////////////////////////////////////// nogo_rule
bool nogo_rule::is_legal(nogo_board board, int p, int toplay)
{
    if (board[p] != EMPTY)
        return false;
    if (board.immortal[p] != toplay && board.immortal[p] != EMPTY)
        return false;

    board[p] = toplay;
    int opp = opponent(toplay);

    if (board.immortal[p] != toplay && !nogo_rule::has_liberty(board, p)) {
        board[p] = EMPTY;
        return false;
    }

    std::vector<int> nbrs = nogo_rule::neighbors(board, p);
    for (int nbr : nbrs) {
        if (board[nbr] == opp && !nogo_rule::has_liberty(board, nbr)) {
            board[p] = EMPTY;
            return false;
        }
    }

    board[p] = EMPTY;
    return true;
}

bool nogo_rule::has_liberty(const nogo_board& board, int p)
{
    assert(board[p] != EMPTY && board[p] != BORDER);
    if (board.immortal[p] == BORDER)
        return true;
    
    int size = board.size;
    std::vector<bool> markers = std::vector<bool>(size, false);

    int color = board[p];
    std::vector<int> point_stack = { p };
    markers[p] = true;

    while (!point_stack.empty()) {
        int i = point_stack.back();
        point_stack.pop_back();

        std::vector<int> nbrs = nogo_rule::neighbors(board, i);
        for (int nbr : nbrs) {
            if (board[nbr] == EMPTY)
                return true;
            if (board[nbr] == color) {
                if (board.immortal[nbr] == BORDER) {
                    return true;
                }
                if (!markers[nbr]) {
                    point_stack.push_back(nbr);
                    markers[nbr] = true;
                }
            }
        }
    }

    return false;
}

std::vector<int> nogo_rule::neighbors(const nogo_board& board, int p)
{
    int n_rows = board.shape.first;
    int n_cols = board.shape.second;

    int r = p / n_cols;
    int c = p % n_cols;

    std::vector<int> nbrs;
    if (r > 0 && board[p - n_cols] != BORDER)
        nbrs.push_back(p - n_cols); // up
    if (r < n_rows-1 && board[p + n_cols] != BORDER)
        nbrs.push_back(p + n_cols); // down
    if (c > 0 && board[p - 1] != BORDER)
        nbrs.push_back(p - 1);      // left
    if (c < n_cols-1 && board[p + 1] != BORDER)
        nbrs.push_back(p + 1);      // right

    return nbrs;
}

//////////////////////////////////////// split_by_nogo
void split_by_nogo::classify_empty_points(const nogo_board& board, std::vector<int>& point_markers)
{
    int size = board.size;
    for (int i = 0; i < size; i++) {
        if (board[i] == EMPTY) {
            if (board.immortal[i] == BORDER) {
                point_markers[i] = split_by_nogo::N_GO;
            }
            else {
                bool b_go = nogo_rule::is_legal(board, i, BLACK);
                bool w_go = nogo_rule::is_legal(board, i, WHITE);
                if (!b_go && !w_go)
                    point_markers[i] = split_by_nogo::N_GO;
                else if (b_go && !w_go)
                    point_markers[i] = split_by_nogo::B_GO;
                else if (!b_go && w_go)
                    point_markers[i] = split_by_nogo::W_GO;
                else
                    point_markers[i] = split_by_nogo::T_GO;
            }
        }
        else {
            point_markers[i] = split_by_nogo::OCC;
        }
    }
    return;
}

void split_by_nogo::identify_walls(const nogo_board& board, const std::vector<int>& point_markers, std::vector<bool>& wall_markers)
{
    int size = board.size;

    std::vector<bool> black_wall_markers(size, false);
    std::vector<bool> white_wall_markers(size, false);

    for (int p = 0; p < size; p++) {
        if (point_markers[p] == split_by_nogo::N_GO) {
            if (!black_wall_markers[p]) {
                mark_wall_at_nogo(board, p, BLACK, point_markers, black_wall_markers);
            }
            if (!white_wall_markers[p]) {
                mark_wall_at_nogo(board, p, WHITE, point_markers, white_wall_markers);
            }
        }
    }

    // merge black and white wall markers and immortal points
    for (int p = 0; p < size; p++) {
        wall_markers[p] = black_wall_markers[p] || white_wall_markers[p] || board.immortal[p] == BORDER;
    }
    return;
}

void split_by_nogo::mark_wall_at_nogo(const nogo_board& board, int p, int color, const std::vector<int>& point_markers, std::vector<bool>& wall_markers)
{
    int size = board.size;
    const int C_GO = (color == BLACK) ? split_by_nogo::B_GO : split_by_nogo::W_GO;
    std::vector<int> playable_empty_points;

    std::vector<bool> stack_markers(size, false);
    std::vector<int> point_stack = { p };
    stack_markers[p] = true;
    wall_markers[p] = true;

    while (!point_stack.empty()) {
        int q = point_stack.back();
        point_stack.pop_back();

        std::vector<int> q_nbrs = nogo_rule::neighbors(board, q);
        for (int q_nbr : q_nbrs) {
            if (!stack_markers[q_nbr] &&
                    (board[q_nbr] == color ||
                        point_markers[q_nbr] == C_GO ||
                        point_markers[q_nbr] == split_by_nogo::N_GO
                    )
                ) {
                point_stack.push_back(q_nbr);
                stack_markers[q_nbr] = true;
                wall_markers[q_nbr] = true;
                if (point_markers[q_nbr] == C_GO && board.immortal[q_nbr] != BORDER)
                    playable_empty_points.push_back(q_nbr);
            }
        }
    }
    return;
}

nogo_board split_by_nogo::mark_region_at_point(const nogo_board& board, int p, const std::vector<int>& point_markers, const std::vector<bool>& wall_markers, std::vector<bool>& region_markers)
{
    assert(!wall_markers[p] || board[p] == EMPTY);
    int size = board.size;
    std::vector<int> region_board(size, BORDER);
    std::vector<int> region_immortal(size, EMPTY);

    std::vector<bool> stack_markers(size, false);
    std::vector<int> point_stack = { p };
    stack_markers[p] = true;
    region_markers[p] = true;
    region_board[p] = board[p];
    region_immortal[p] = (wall_markers[p]) ? point_markers[p] : board.immortal[p];

    while (!point_stack.empty()) {
        int q = point_stack.back();
        point_stack.pop_back();

        std::vector<int> q_nbrs = nogo_rule::neighbors(board, q);
        for (int q_nbr : q_nbrs) {
            if (stack_markers[q_nbr])
                continue;
            region_board[q_nbr] = board[q_nbr];
            if (wall_markers[q_nbr] && board[q_nbr] != EMPTY) {
                region_immortal[q_nbr] = BORDER;
            }
            else {
                point_stack.push_back(q_nbr);
                stack_markers[q_nbr] = true;
                region_markers[q_nbr] = true;
                if (wall_markers[q_nbr])
                    region_immortal[q_nbr] = point_markers[q_nbr];
                else
                    region_immortal[q_nbr] = board.immortal[q_nbr];
            }
        }
    }

    nogo_board region(region_board, region_immortal, board.shape);
    return shrink_board(region);
}

std::vector<nogo_board> split_by_nogo::split(const nogo_board& board)
{
    int size = board.size;
    std::vector<int> point_markers(size, -1);
    std::vector<bool> wall_markers(size, false);
    std::vector<bool> region_markers(size, false);
    split_by_nogo::classify_empty_points(board, point_markers);
    split_by_nogo::identify_walls(board, point_markers, wall_markers);

    std::vector<nogo_board> regions;

    for (int p = 0; p < size; p++) {
        if (!region_markers[p] && board[p] != BORDER && point_markers[p] != split_by_nogo::N_GO) {
            if (!wall_markers[p] || board[p] == EMPTY) {
                nogo_board region = split_by_nogo::mark_region_at_point(board, p, point_markers, wall_markers, region_markers);
                regions.push_back(region);
            }
        }
    }
    
    return regions;
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
    return nogo_rule::is_legal({_game.board(), _game.immortal(), _game.shape()}, _current,
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
