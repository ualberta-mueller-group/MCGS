#include "amazons.h"

#include "cgt_move.h"
#include "utilities.h"
#include "grid_location.h"

class amazons_move_generator: public move_generator
{
public:
    amazons_move_generator(const amazons& g, bw to_play);

    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    bool _increment(bool init);

    bool _increment_queen_start(bool init);
    bool _increment_queen_end(bool init);
    bool _increment_arrow_end(bool init);

    bool _point_is_empty(int point) const;


    const amazons& _game;

    bool _has_move;

    grid_location _queen_start;
    int _move1;

    size_t _queen_dir_idx;
    grid_location _queen_end;
    int _move2;

    size_t _arrow_dir_idx;
    grid_location _arrow_end;
    int _move3;

    static constexpr size_t N_DIRS = GRID_DIRS_ALL.size();
};

using namespace std;

////////////////////////////////////////////////// helper functions
namespace {
//////////////////////////////////////// Move encoding
const unsigned int THREE_PART_MOVE_MAX = get_bit_mask<unsigned int>(10);

inline ::move encode_three_part_move(unsigned int val1, unsigned int val2,
                                     unsigned int val3)
{
    static_assert(size_in_bits<::move>() >= 30);

    assert((val1 <= THREE_PART_MOVE_MAX) && //
           (val2 <= THREE_PART_MOVE_MAX) && //
           (val3 <= THREE_PART_MOVE_MAX)    //
    );

    return (val1) | (val2 << 10) | (val3 << 20);
}

inline void decode_three_part_move(::move m, unsigned int& val1,
                                   unsigned int& val2, unsigned int& val3)
{
    assert((m & get_bit_mask<::move>(30)) == m);

    val1 = (m & THREE_PART_MOVE_MAX);
    val2 = ((m >> 10) & THREE_PART_MOVE_MAX);
    val3 = ((m >> 20) & THREE_PART_MOVE_MAX);
}

} // namespace

////////////////////////////////////////////////// amazons methods
amazons::amazons(int n_rows, int n_cols)
    : grid(n_rows, n_cols)
{
}

amazons::amazons(const std::vector<int>& board, int_pair shape)
    : grid(board, shape)
{
}

amazons::amazons(const std::string& game_as_string)
    : grid(game_as_string)
{
}

void amazons::play(const ::move& m, bw to_play)
{
    game::play(m, to_play);

    unsigned int move1, move2, move3;
    decode_three_part_move(m, move1, move2, move3);

    // TODO check paths?

    // Queen end can't be queen start or arrow end
    assert(move2 != move1 && move2 != move3);

    // Move queen
    assert(at(move1) == to_play && at(move2) == EMPTY);
    replace(move1, EMPTY);
    replace(move2, to_play);

    // Shoot arrow
    assert(at(move3) == EMPTY);
    replace(move3, BORDER);

    // TODO this needs unit tests. And debug mode testing for play()/undo()
    // assert_restore_sumgame should catch it?
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        // Remove old state
        hash.toggle_value(2 + move1, to_play);
        hash.toggle_value(2 + move2, EMPTY);

        if (move1 != move3)
            hash.toggle_value(2 + move3, EMPTY);

        // Add new state
        hash.toggle_value(2 + move2, to_play);
        hash.toggle_value(2 + move3, BORDER);

        if (move1 != move3)
            hash.toggle_value(2 + move1, EMPTY);

        _mark_hash_updated();
    }
}

void amazons::undo_move()
{
    const ::move m_enc = last_move();
    game::undo_move();

    const bw to_play = cgt_move::get_color(m_enc);
    const ::move m_dec = cgt_move::decode(m_enc);

    unsigned int move1, move2, move3;
    decode_three_part_move(m_dec, move1, move2, move3);

    // TODO check paths?

    // Queen end can't be queen start or arrow end
    assert(move2 != move1 && move2 != move3);

    // Remove arrow
    assert(at(move3) == BORDER);
    replace(move3, EMPTY);

    // Move queen back
    assert(at(move1) == EMPTY && at(move2) == to_play);
    replace(move1, to_play);
    replace(move2, EMPTY);

    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        // Remove old state
        hash.toggle_value(2 + move2, to_play);
        hash.toggle_value(2 + move3, BORDER);

        if (move1 != move3)
            hash.toggle_value(2 + move1, EMPTY);

        // Add new state
        hash.toggle_value(2 + move1, to_play);
        hash.toggle_value(2 + move2, EMPTY);

        if (move1 != move3)
            hash.toggle_value(2 + move3, EMPTY);

        _mark_hash_updated();
    }
}

move_generator* amazons::create_move_generator(bw to_play) const
{
    return new amazons_move_generator(*this, to_play);
}

game* amazons::inverse() const
{
    return new amazons(inverse_board(), shape());
}

////////////////////////////////////////////////// split
#ifdef AMAZONS_SPLIT
split_result amazons::_split_impl() const
{
    assert(false);
}
#endif

//////////////////////////////////////////////////

void test_amazons_stuff()
{
    amazons g("X.|..");

    unique_ptr<move_generator> gen(g.create_move_generator(BLACK));

    cout << g << endl;

    while (*gen)
    {
        const ::move m = gen->gen_move();
        ++(*gen);

        unsigned int move1, move2, move3;
        decode_three_part_move(m, move1, move2, move3);

        cout << move1 << " " << move2 << " " << move3 << endl;
    }
}

//////////////////////////////////////////////////
// amazons_move_generator methods

amazons_move_generator::amazons_move_generator(const amazons& g, bw to_play)
    : move_generator(to_play),
      _game(g),
      _has_move(false),
      _queen_start(g.shape()),
      _queen_end(g.shape()),
      _arrow_end(g.shape())
{
    _increment(true);
}

void amazons_move_generator::operator++()
{
    assert(*this);
    _increment(false);
}

amazons_move_generator::operator bool() const
{
    return _has_move;
}

::move amazons_move_generator::gen_move() const
{
    assert(*this);
    return encode_three_part_move(_move1, _move2, _move3);
}

bool amazons_move_generator::_increment(bool init)
{
    assert(init || *this);

    if (init)
    {
        _has_move = false;

        if (_game.size() == 0)
            return false;
    }

    bool has_queen_start = _has_move;
    bool has_queen_end = _has_move;
    bool has_arrow_end = _has_move;

    while (true)
    {
        // Try to increment arrow
        if (has_queen_end && _increment_arrow_end(!has_arrow_end))
        {
            _has_move = true;
            return true;
        }

        has_arrow_end = false;

        // Try to increment queen end
        if (has_queen_start && _increment_queen_end(!has_queen_end))
        {
            has_queen_end = true;
            continue;
        }

        has_queen_end = false;

        // Try to increment queen
        if (_increment_queen_start(!has_queen_start))
        {
            has_queen_start = true;
            continue;
        }

        _has_move = false;
        return false;
    }
}

bool amazons_move_generator::_increment_queen_start(bool init)
{
    if (init)
        assert(_queen_start.get_coord() == int_pair(0, 0));
    else
        _queen_start.increment_position();

    while (_queen_start.valid())
    {
        _move1 = _queen_start.get_point();

        if (_game.at(_move1) == to_play())
            return true;

        _queen_start.increment_position();
    }

    return false;
}

bool amazons_move_generator::_increment_queen_end(bool init)
{
    if (init)
    {
        _queen_dir_idx = 0;
        _queen_end.set_coord(_queen_start.get_coord());
    }

    assert(_queen_dir_idx < N_DIRS);

    while (true)
    {
        const grid_dir dir = GRID_DIRS_ALL[_queen_dir_idx];

        if (_queen_end.move(dir))
        {
            _move2 = _queen_end.get_point();

            if (_point_is_empty(_move2))
                return true;
        }

        _queen_dir_idx++;
        if (_queen_dir_idx >= N_DIRS)
            return false;

        _queen_end.set_coord(_queen_start.get_coord());
    }

    return false;
}

bool amazons_move_generator::_increment_arrow_end(bool init)
{
    if (init)
    {
        _arrow_dir_idx = 0;
        _arrow_end.set_coord(_queen_end.get_coord());
    }

    assert(_arrow_dir_idx < N_DIRS);

    while (true)
    {
        // Try to move in current direction
        const grid_dir dir = GRID_DIRS_ALL[_arrow_dir_idx];

        if (_arrow_end.move(dir))
        {
            _move3 = _arrow_end.get_point();

            if (_point_is_empty(_move3))
                return true;
        }

        // Otherwise increment direction
        _arrow_dir_idx++;
        if (_arrow_dir_idx >= N_DIRS)
            return false;

        _arrow_end.set_coord(_queen_end.get_coord());
    }

    return false;
}

inline bool amazons_move_generator::_point_is_empty(int point) const
{
    if (point == _move1)
        return true;

    return _game.at(point) == EMPTY;
}
