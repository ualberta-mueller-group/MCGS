#include "clobber.h"
#include "cgt_basics.h"
#include "game.h"
#include "cgt_move.h"
#include <cassert>

// TODO move this into grid in a cleaner way?
namespace {

enum clobber_dir
{
    CLOBBER_UP = 0,
    CLOBBER_RIGHT,
    CLOBBER_DOWN,
    CLOBBER_LEFT,
    CLOBBER_NO_DIR,
};

static constexpr clobber_dir CLOBBER_DIR_INITIAL = CLOBBER_UP;

clobber_dir next_clobber_dir(clobber_dir d)
{
    static const clobber_dir NEXT_DIR[] = {CLOBBER_RIGHT, CLOBBER_DOWN, CLOBBER_LEFT, CLOBBER_NO_DIR, CLOBBER_NO_DIR};

    assert(d >= 0 && d < (sizeof(NEXT_DIR) / sizeof(NEXT_DIR[0])));
    assert(d != CLOBBER_NO_DIR);

    return NEXT_DIR[d];
}

inline int_pair apply_clobber_dir(int_pair coord, clobber_dir dir)
{
    static const int_pair DELTA[] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}, {0, 0}};

    assert(dir >= 0 && dir < (sizeof(DELTA) / sizeof(DELTA[0])));

    const int_pair& d = DELTA[dir];
    coord.first += d.first;
    coord.second += d.second;

    return coord;
}


static_assert(CLOBBER_DIR_INITIAL == 0);

} // namespace

////////////////////////////////////////////////// move generator
class clobber_move_generator: public move_generator
{
public:
    clobber_move_generator(const clobber& game, bw to_play);

    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    void _next_move(bool init);

    bool _increment();
    bool _increment_dir();
    bool _increment_coord();

    int_pair _get_target_coord() const;

    const clobber& _game;
    int_pair _coord;
    clobber_dir _dir;
    bool _has_move;
};


////////////////////////////////////////////////// clobber
clobber::clobber(int n_rows, int n_cols): grid(n_rows, n_cols)
{
}

clobber::clobber(const std::vector<int>& board, int_pair shape): grid(board, shape)
{
}

clobber::clobber(const std::string& game_as_string): grid(game_as_string)
{
}

void clobber::play(const move& m, bw to_play)
{
    game::play(m, to_play);

    const int from = cgt_move::from(m);
    const int to = cgt_move::to(m);

    const bw opp = opponent(to_play);

    assert(checked_is_color(from, to_play));
    assert(checked_is_color(to, opp));

    replace(from, EMPTY);
    replace(to, to_play);
}

void clobber::undo_move()
{
    const move m_enc = last_move();
    game::undo_move();

    bw to_play;
    int to;
    int from = cgt_move::decode3(m_enc, &to, &to_play);

    const bw opp = opponent(to_play);

    assert(checked_is_color(from, EMPTY));
    assert(checked_is_color(to, to_play));

    replace(from, to_play);
    replace(to, opp);
}

bool clobber::is_move(const int_pair& from, const int_pair& to, bw to_play) const
{
    assert(is_black_white(to_play));

    if (!coord_in_bounds(from) || !coord_in_bounds(to))
        return false;

    const bw opp = opponent(to_play);

    return (at(from) == to_play) && (at(to) == opp);
}

split_result clobber::_split_impl() const
{
    // TODO
    return split_result();
}

move_generator* clobber::create_move_generator(bw to_play) const
{
    return new clobber_move_generator(*this, to_play);
}

void clobber::print(std::ostream& str) const
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
    _coord(0, 0),
    _dir(CLOBBER_DIR_INITIAL),
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

move clobber_move_generator::gen_move() const
{
    assert(*this);

    const int from_point = _game.coord_to_point(_coord);
    const int to_point = _game.coord_to_point(_get_target_coord());

    return cgt_move::two_part_move(from_point, to_point);
}

void clobber_move_generator::_next_move(bool init)
{
    assert(init || *this);
    assert(_game.coord_in_bounds(_coord));
    assert(_dir != CLOBBER_NO_DIR);

    _has_move = false;

    if (!init && !_increment())
        return;

    const int player = to_play();

    while (true)
    {
        _has_move = _game.is_move(_coord, _get_target_coord(), player);

        if (_has_move || !_increment())
            break;
    }
}

inline bool clobber_move_generator::_increment()
{
    if (_increment_dir())
        return true;

    return _increment_coord();
}

inline bool clobber_move_generator::_increment_dir()
{
    assert(_dir != CLOBBER_NO_DIR);

    _dir = next_clobber_dir(_dir);

    return _dir != CLOBBER_NO_DIR;
}

inline bool clobber_move_generator::_increment_coord()
{
    assert(_game.coord_in_bounds(_coord));

    _dir = CLOBBER_DIR_INITIAL;

    _coord.second++;

    const int_pair& shape = _game.shape();

    if (_coord.second >= shape.second)
    {
        assert(!_game.coord_in_bounds(_coord));

        _coord.second = 0;
        _coord.first++;

        if (_coord.first >= shape.first)
        {
            assert(!_game.coord_in_bounds(_coord));
            return false;
        }
    }

    assert(_game.coord_in_bounds(_coord));
    return true;
}

inline int_pair clobber_move_generator::_get_target_coord() const
{
    return apply_clobber_dir(_coord, _dir);
}

