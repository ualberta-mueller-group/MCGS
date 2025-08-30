//---------------------------------------------------------------------------
// Implementation of Clobber on a 1-dimensional strip
//---------------------------------------------------------------------------
#include "clobber_1xn.h"

#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"
#include "strip.h"
#include "utilities.h"
#include "iobuffer.h"
#include <cassert>
#include <utility>
#include <vector>
#include <cstddef>
#include <ostream>

class clobber_1xn_db_game_generator;

using std::string, std::pair, std::vector;

//////////////////////////////////////////////////

namespace {
void get_subgame_boundaries(const std::vector<int>& board,
                            std::vector<std::pair<size_t, size_t>>& boundaries)
{
    const size_t N = board.size();

    bool in_chunk = false;
    size_t chunk_start = 0;
    bool found_black = false;
    bool found_white = false;

    for (size_t i = 0; i < N; i++)
    {
        const int& color = board[i];
        assert(is_empty_black_white(color));

        if (color != EMPTY)
        {
            // new chunk
            if (!in_chunk)
            {
                assert(!found_black && !found_white);

                in_chunk = true;
                chunk_start = i;
            }

            if (color == BLACK)
                found_black = true;
            else
                found_white = true;
        }
        else
        {
            // end of chunk
            if (in_chunk && found_black && found_white)
                boundaries.emplace_back(chunk_start, i - chunk_start);

            in_chunk = false;
            chunk_start = 0;
            found_black = false;
            found_white = false;
        }
    }

    if (in_chunk && found_black && found_white)
        boundaries.emplace_back(chunk_start, N - chunk_start);
}

} // namespace

//////////////////////////////////////////////////
clobber_1xn::clobber_1xn(const vector<int>& board) : strip(board)
{
}

clobber_1xn::clobber_1xn(std::string game_as_string) : strip(game_as_string)
{
}

void clobber_1xn::play(const move& m, bw to_play)
{
    game::play(m, to_play);

    const int from = cgt_move::from(m);
    const int to = cgt_move::to(m);
    assert(at(from) == to_play);
    assert(at(to) == opponent(to_play));

    // incremental hash
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        hash.toggle_value(from, to_play);
        hash.toggle_value(to, opponent(to_play));

        hash.toggle_value(from, EMPTY);
        hash.toggle_value(to, to_play);
        _mark_hash_updated();
    }

    replace(from, EMPTY);
    replace(to, to_play);
}

void clobber_1xn::undo_move()
{
    const move mc = last_move();
    game::undo_move();

    const move m = cgt_move::decode(mc);
    const int from = cgt_move::from(m);
    const int to = cgt_move::to(m);

    const bw player = cgt_move::get_color(mc);
    assert(at(from) == EMPTY);
    assert(at(to) == player);

    // incremental hash
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        hash.toggle_value(from, EMPTY);
        hash.toggle_value(to, player);

        hash.toggle_value(from, player);
        hash.toggle_value(to, opponent(player));
        _mark_hash_updated();
    }

    replace(from, player);
    replace(to, opponent(player));
}

void clobber_1xn::save_impl(obuffer& os) const
{
    _save_board(os, board_const());
}

dyn_serializable* clobber_1xn::load_impl(ibuffer& is)
{
    return new clobber_1xn(_load_board(is));
}

split_result clobber_1xn::_split_impl() const
{
    const vector<int>& board = board_const();

    vector<pair<size_t, size_t>> boundaries;
    get_subgame_boundaries(board, boundaries);
    const size_t n_subgames = boundaries.size();

    if (n_subgames < 2)
        return {};

    split_result sr = split_result(vector<game*>());
    for (const pair<size_t, size_t>& range : boundaries)
        sr->push_back(
            new clobber_1xn(vector_substr(board, range.first, range.second)));

    return sr;
}

void clobber_1xn::_normalize_impl()
{
    const vector<int>& board = board_const();
    const size_t board_size = board.size();

    vector<pair<size_t, size_t>> boundaries;
    get_subgame_boundaries(board, boundaries);
    const size_t n_subgames = boundaries.size();

    // Empty, or subgame is whole board
    if (                                       //
        board_size == 0 ||                     //
        (                                      //
            n_subgames == 1 &&                 //
            boundaries[0].first == 0 &&        //
            boundaries[0].second == board_size //
            )                                  //
        )                                      //
    {
        const bool do_mirror = strip::_should_mirror(board);

        if (do_mirror)
        {
            _normalize_did_change.push_back(true);
            _normalize_boards.emplace_back(board);

            _set_board(vector_reversed(board));
            return;
        }

        if (_hash_updatable())
            _mark_hash_updated();

        _normalize_did_change.push_back(false);
        return;
    }

    _normalize_did_change.push_back(true);
    _normalize_boards.emplace_back(board);

    vector<int> new_board;
    new_board.reserve(board_size);

    for (size_t i = 0; i < n_subgames; i++)
    {
        const pair<size_t, size_t>& range = boundaries[i];
        const size_t start = range.first;
        const size_t end = start + range.second;

        for (size_t j = start; j < end; j++)
            new_board.push_back(board[j]);

        if (i + 1 < n_subgames)
            new_board.push_back(EMPTY);
    }

    const bool do_mirror = strip::_should_mirror(new_board);
    if (do_mirror)
    {
        vector<int> new_board_mirrored = vector_reversed(new_board);
        _set_board(new_board_mirrored);
        return;
    }

    _set_board(new_board);
}

void clobber_1xn::_undo_normalize_impl()
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

game* clobber_1xn::inverse() const
{
    return new clobber_1xn(inverse_board());
}

string clobber_1xn::xoxo(int n)
{
    string result;
    for (int i = 0; i < n; ++i)
        result += "XO";
    return result;
}

//---------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const clobber_1xn& g)
{
    out << g.board_as_string();
    return out;
}

//---------------------------------------------------------------------------

class clobber_1xn_move_generator : public move_generator
{
public:
    clobber_1xn_move_generator(const clobber_1xn& game, bw to_play);
    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    int _at(int p) const { return _game.at(p); }

    bool _is_move(int p, int dir) const;
    bool _has_move(int p) const;
    void _find_next_move();

    const clobber_1xn& _game;
    int _current; // current stone location to test
    int _dir;     // +-1
};

inline clobber_1xn_move_generator::clobber_1xn_move_generator(
    const clobber_1xn& game, bw to_play)
    : move_generator(to_play), _game(game), _current(0), _dir(1)
{
    if (_game.size() > 0             //
        && !_is_move(_current, _dir) //
        )                            //
        _find_next_move();
}

void clobber_1xn_move_generator::operator++()
{
    _find_next_move();
}

inline bool clobber_1xn_move_generator::_is_move(int p, int dir) const
{
    return _at(p) == to_play() && _game.checked_is_color(p + dir, opponent());
}

bool clobber_1xn_move_generator::_has_move(int p) const
{
    assert(_at(p) == to_play());
    return _is_move(p, 1) || _is_move(p, -1);
}

void clobber_1xn_move_generator::_find_next_move()
{
    const int num = _game.size();

    // try same from, other dir first.
    if (_dir == 1                     //
        && _current < num             //
        && _at(_current) == to_play() //
        && _is_move(_current, -1)     //
        )                             //
        _dir = -1;

    else // advance
    {
        ++_current;
        while (_current < num                 //
               && (_at(_current) != to_play() //
                   || !_has_move(_current)    //
                   )                          //
               )                              //
            ++_current;
        if (_current < num)
        {
            if (_is_move(_current, 1))
                _dir = 1;
            else
                _dir = -1;
        }
    }
}

clobber_1xn_move_generator::operator bool() const
{
    return _current < _game.size();
}

move clobber_1xn_move_generator::gen_move() const
{
    assert(operator bool());
    return cgt_move::two_part_move(_current, _current + _dir);
}

//---------------------------------------------------------------------------

move_generator* clobber_1xn::create_move_generator(bw to_play) const
{
    return new clobber_1xn_move_generator(*this, to_play);
}

//---------------------------------------------------------------------------
