#include "gen_toads.h"

#include <vector>
#include <string>
#include <ostream>
#include <cassert>
#include <utility>

#include "cgt_basics.h"
#include "cgt_move_new.h"
#include "throw_assert.h"
#include "game.h"
#include "strip.h"
#include "utilities.h"

using namespace std;

//////////////////////////////////////////////////
// class gen_toads_move_generator

enum toads_move_enum
{
    TOADS_MOVE_UNKNOWN = 0,
    TOADS_MOVE_SLIDE,
    TOADS_MOVE_JUMP,
};

class gen_toads_move_generator: public move_generator
{
public:
    gen_toads_move_generator(const gen_toads& g, bw to_play);

    void operator++() override;
    operator bool() const override;
    ::move gen_move() const override;

private:
    void _increment(bool init);
    bool _increment_start(bool init);
    bool _increment_distance(bool init);

    bool _idx_in_range(int idx) const;
    bool _distance_in_slide_bounds() const;
    bool _distance_in_jump_bounds() const;

    const gen_toads& _g;
    const int _dir;

    int _slide_start;
    int _min_slide_signed;
    int _max_slide_signed;

    int _jump_start;
    int _min_jump_signed;
    int _max_jump_signed;

    int _start_idx;

    toads_move_enum _move_kind;
    int _move_distance;
};

////////////////////////////////////////////////// helper functions
namespace {

bool only_legal_colors(const std::vector<int>& board)
{
    for (const int& x : board)
        if (!is_empty_black_white(x))
            return false;
    return true;
}

} // namespace

////////////////////////////////////////////////// gen_toads methods

gen_toads::gen_toads(const vector<int>& params, const vector<int>& board)
    : strip(board)
{
    THROW_ASSERT(only_legal_colors(board_const()));
    _init_params(params);
}

gen_toads::gen_toads(const vector<int>& params, const string& game_as_string)
    : strip(game_as_string)
{
    THROW_ASSERT(only_legal_colors(board_const()));
    _init_params(params);
}

void gen_toads::play(const ::move& m, bw to_play)
{
    game::play(m, to_play);

    assert(is_black_white(to_play));

    const int distance_signed = cgt_move_new::move2_get_part_1(m);
    const int start_idx = cgt_move_new::move2_get_part_2(m);

    assert(LOGICAL_IMPLIES(distance_signed > 0, to_play == BLACK) && //
           LOGICAL_IMPLIES(distance_signed < 0, to_play == WHITE)    //
    );

    const int end_idx = start_idx + distance_signed;

    assert((0 <= start_idx && start_idx <= size()) && //
           (0 <= end_idx && end_idx <= size())        //
    );

    assert(at(start_idx) == to_play);
    assert(at(end_idx) == EMPTY);

    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        // Remove old state
        hash.toggle_value(4 + start_idx, to_play);
        hash.toggle_value(4 + end_idx, EMPTY);

        // Add new state
        hash.toggle_value(4 + start_idx, EMPTY);
        hash.toggle_value(4 + end_idx, to_play);

        _mark_hash_updated();
    }

    replace(start_idx, EMPTY);
    replace(end_idx, to_play);
}

void gen_toads::undo_move()
{
    const ::move m_enc = last_move();
    game::undo_move();

    const bw to_play = cgt_move_new::get_color(m_enc);

    const int distance_signed = cgt_move_new::move2_get_part_1(m_enc);
    const int start_idx = cgt_move_new::move2_get_part_2(m_enc);

    assert(LOGICAL_IMPLIES(distance_signed > 0, to_play == BLACK) && //
           LOGICAL_IMPLIES(distance_signed < 0, to_play == WHITE)    //
    );

    const int end_idx = start_idx + distance_signed;

    assert((0 <= start_idx && start_idx <= size()) && //
           (0 <= end_idx && end_idx <= size())        //
    );

    assert(at(start_idx) == EMPTY);
    assert(at(end_idx) == to_play);

    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();

        // Remove old state
        hash.toggle_value(4 + start_idx, EMPTY);
        hash.toggle_value(4 + end_idx, to_play);

        // Add new state
        hash.toggle_value(4 + start_idx, to_play);
        hash.toggle_value(4 + end_idx, EMPTY);

        _mark_hash_updated();
    }


    replace(start_idx, to_play);
    replace(end_idx, EMPTY);
}

move_generator* gen_toads::create_move_generator(bw to_play) const
{
    return new gen_toads_move_generator(*this, to_play);
}

void gen_toads::print(ostream& str) const
{
    str << "gen_toads<";
    str << get_min_slide() << ", ";
    str << get_max_slide() << ", ";
    str << get_max_jump() << ", ";
    str << get_friendly_jump() << ">:";
    str << board_as_string();
}

void gen_toads::print_move(std::ostream& str, const ::move& m) const
{
    int move_delta, start_idx;
    cgt_move_new::move2_unpack(m, move_delta, start_idx);

    const int end_idx = start_idx + move_delta;

    str << (1 + start_idx) << '-' << (1 + end_idx);
}

game* gen_toads::inverse() const
{
    return new gen_toads(
        {
            get_min_slide(),
            get_max_slide(),
            get_max_jump(),
            get_friendly_jump(),
        },
        inverse_mirror_board());
}

//split_result gen_toads::_split_impl() const
//{
//    assert(false);
//}

void gen_toads::_init_hash(local_hash& hash) const
{
    hash.toggle_value(0, get_min_slide());
    hash.toggle_value(1, get_max_slide());
    hash.toggle_value(2, get_max_jump());
    hash.toggle_value(3, get_friendly_jump());

    const int len = size();
    for (int i = 0; i < len; i++)
        hash.toggle_value(4 + i, at(i));
}

void gen_toads::_normalize_impl()
{
    if (_hash_updatable())
        _mark_hash_updated();
}

void gen_toads::_undo_normalize_impl()
{
    if (_hash_updatable())
        _mark_hash_updated();
}

#define ORDER_COMPARE_MACRO(expr1, expr2) \
{ \
    const auto& val1 = expr1; \
    const auto& val2 = expr2; \
    if (val1 != val2) \
        return val1 < val2 ? REL_LESS : REL_GREATER; \
} \
static_assert(true)

relation gen_toads::_order_impl(const game* rhs) const
{
    const gen_toads* other = reinterpret_cast<const gen_toads*>(rhs);
    assert(dynamic_cast<const gen_toads*>(rhs) == other);

    ORDER_COMPARE_MACRO(get_min_slide(), other->get_min_slide());
    ORDER_COMPARE_MACRO(get_max_slide(), other->get_max_slide());
    ORDER_COMPARE_MACRO(get_max_jump(), other->get_max_jump());
    ORDER_COMPARE_MACRO(get_friendly_jump(), other->get_friendly_jump());

    const int len1 = size();
    const int len2 = other->size();
    ORDER_COMPARE_MACRO(len1, len2);

    assert(len1 == len2);

    for (int i = 0; i < len1; i++)
    {
        ORDER_COMPARE_MACRO(at(i), other->at(i));
    }

    return REL_EQUAL;
}

#undef ORDER_COMPARE_MACRO

void gen_toads::_init_params(const vector<int>& params)
{
    THROW_ASSERT(params.size() == 4);
    _min_slide = params[0];
    _max_slide = params[1];
    _max_jump = params[2];
    _friendly_jump = static_cast<bool>(params[3]);

    THROW_ASSERT(_min_slide > 0 &&              //
                 _min_slide <= _max_slide &&    //
                 _max_jump >= 0 &&              //
                 params[3] == (params[3] & 0x1) //
    );
}

//////////////////////////////////////////////////
// gen_toads_move_generator methods

gen_toads_move_generator::gen_toads_move_generator(const gen_toads& g, bw to_play)
    : move_generator(to_play),
      _g(g),
      _dir(to_play == BLACK ? 1 : -1)
{
    assert(is_black_white(to_play));

    // Slide constants
    _min_slide_signed = _g.get_min_slide();
    _max_slide_signed = _g.get_max_slide();
    _slide_start = _min_slide_signed;

    // Jump constants
    _min_jump_signed = 1;
    _max_jump_signed = _g.get_max_jump();
    if (_max_jump_signed != 0)
        _max_jump_signed++;
    _jump_start = _min_jump_signed;


    // Adjust sign for direction
    if (_dir == -1)
    {
        _min_slide_signed = -_min_slide_signed;
        _max_slide_signed = -_max_slide_signed;
        swap(_min_slide_signed, _max_slide_signed);

        _min_jump_signed = -_min_jump_signed;
        _max_jump_signed = -_max_jump_signed;
        swap(_min_jump_signed, _max_jump_signed);

        _slide_start = _max_slide_signed;
        _jump_start = _max_jump_signed;
    }
    else
        assert(_dir == 1);

    assert((_min_slide_signed <= _max_slide_signed) && //
           logical_iff(_g.get_max_jump() != 0,
                       (_min_jump_signed <= _max_jump_signed)) //
    );

    _increment(true);
}

void gen_toads_move_generator::operator++()
{
    assert(*this);
    _increment(false);
}

gen_toads_move_generator::operator bool() const
{
    return _start_idx < _g.size();
}

::move gen_toads_move_generator::gen_move() const
{
    assert(*this);
    // _move_distance is signed
    return cgt_move_new::move2_create(_move_distance, _start_idx);
}

void gen_toads_move_generator::_increment(bool init)
{
    assert(init || *this);

    bool has_start = !init;
    bool has_distance = !init;

    while (true)
    {
        // Try to increment distance
        if (has_start && _increment_distance(!has_distance))
            return;

        has_distance = false;

        // Try to increment start stone
        if (_increment_start(!has_start))
        {
            has_start = true;
            continue;
        }

        assert(_start_idx == _g.size() && !(*this));
        return;
    }
}

bool gen_toads_move_generator::_increment_start(bool init)
{
    if (init)
        _start_idx = 0;
    else
        _start_idx++;

    const int len = _g.size();
    const bw color = to_play();
    assert(is_black_white(color));

    while (_start_idx < len)
    {
        if (_g.at(_start_idx) == color)
            return true;

        _start_idx++;
    }

    return false;
}

bool gen_toads_move_generator::_increment_distance(bool init)
{
    if (init)
    {
        _move_kind = TOADS_MOVE_UNKNOWN;
        _move_distance = 0;
    }
    else
        _move_distance += _dir;

    if (_move_kind == TOADS_MOVE_JUMP)
        return false;

    if (_move_kind == TOADS_MOVE_UNKNOWN)
    {
        const int idx_adjacent = _start_idx + _dir;

        if (!_idx_in_range(idx_adjacent))
            return false;

        const int val_adjacent = _g.at(idx_adjacent);
        assert(is_empty_black_white(val_adjacent));

        if (val_adjacent == EMPTY)
        {
            _move_kind = TOADS_MOVE_SLIDE;
            _move_distance = _slide_start;
        }
        else
        {
            _move_kind = TOADS_MOVE_JUMP;
            _move_distance = _jump_start;
        }
    }

    const bw player = to_play();

    int idx = _start_idx + _move_distance;

    if (_move_kind == TOADS_MOVE_SLIDE)
    {
        if (!_distance_in_slide_bounds() || !_idx_in_range(idx))
            return false;

        return _g.at(idx) == EMPTY;
    }
    else
    {
        assert(_move_kind == TOADS_MOVE_JUMP);

        while (_distance_in_jump_bounds())
        {
            const int idx = _start_idx + _move_distance;

            if (!_idx_in_range(idx))
                return false;

            const int val = _g.at(idx);
            assert(is_empty_black_white(val));

            if (val == EMPTY)
                return true;

            if (val == player && !_g.get_friendly_jump())
                return false;

            _move_distance += _dir;
        }
    }

    return false;
}

inline bool gen_toads_move_generator::_idx_in_range(int idx) const
{
    return (0 <= idx) && (idx < _g.size());
}

inline bool gen_toads_move_generator::_distance_in_slide_bounds() const
{
    return (_min_slide_signed <= _move_distance) &&
           (_move_distance <= _max_slide_signed);
}

inline bool gen_toads_move_generator::_distance_in_jump_bounds() const
{
    return (_min_jump_signed <= _move_distance) &&
           (_move_distance <= _max_jump_signed);
}
