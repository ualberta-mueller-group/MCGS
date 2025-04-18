//---------------------------------------------------------------------------
// Implementation of Clobber on a 1-dimensional strip
//---------------------------------------------------------------------------
#include "clobber_1xn.h"

#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"
#include "strip.h"

using std::string, std::pair, std::vector;

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

split_result clobber_1xn::_split_impl() const
{
    vector<pair<int, int>> chunk_ranges;

    string board = board_as_string();
    const size_t N = board.size();

    // auto add_chunk = [&](int start, int len) -> void
    //{
    //     result->push_back(new clobber_1xn(board.substr(start, len)));
    // };

    int chunk_start = -1;
    bool found_black = false;
    bool found_white = false;

    for (size_t i = 0; i < N; i++)
    {
        const char c = board[i];
        int color = clobber_char_to_color(c);

        if (color != EMPTY)
        {
            // new chunk
            if (chunk_start == -1)
            {
                chunk_start = i;
            }

            if (color == BLACK)
            {
                found_black = true;
            }
            else
            {
                found_white = true;
            }
        }
        else
        {
            // end of chunk
            if (chunk_start != -1 && found_black && found_white)
            {
                chunk_ranges.push_back({chunk_start, i - chunk_start});
            }

            chunk_start = -1;
            found_black = false;
            found_white = false;
        }
    }

    if (chunk_start != -1)
    {
        chunk_ranges.push_back({chunk_start, N - chunk_start});
    }

    if (chunk_ranges.size() == 1)
    {
        return split_result();
    }
    else
    {
        split_result result = split_result(vector<game*>());

        for (const pair<int, int>& range : chunk_ranges)
        {
            result->push_back(
                new clobber_1xn(board.substr(range.first, range.second)));
        }

        return result;
    }
}


game* clobber_1xn::inverse() const
{
    return new clobber_1xn(inverse_board());
}

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
