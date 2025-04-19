//---------------------------------------------------------------------------
// Impartial game - kayles
//---------------------------------------------------------------------------
#include "kayles.h"

#include <ostream>
#include <vector>
using std::vector;

int kayles::_cache[100];

move kayles::encode(int take, int smaller, int larger)
{
    int first = 2 * smaller + take - 1;
    return cgt_move::two_part_move(first, larger);
}

void kayles::decode(move m, int& take, 
                    int& smaller, int& larger)
{
    larger = cgt_move::second(m);
    assert(larger >= 0);
    int f = cgt_move::first(m);
    smaller = f / 2;
    assert(smaller >= 0);
    assert(larger >= smaller);
    take = 1 + f % 2;
}

void kayles::_init_hash(local_hash& hash)
{
    hash.toggle_value(0, _value);
    assert(_smaller_part == 0); // not sure.
}

void kayles::play(const move& m)
{
    int take;
    int smaller;
    int larger;
    decode(m, take, smaller, larger);
    _value = larger;
    _smaller_part = smaller;
    impartial_game::play(m);
}

void kayles::undo_move()
{
    const move m = cgt_move::decode(last_move());
    int take;
    int smaller;
    int larger;
    decode(m, take, smaller, larger);
    _value = larger + smaller + take;
    _smaller_part = 0;
    game::undo_move();
}

void kayles::print(std::ostream& str) const
{
    str << "kayles: " << _value;
}

void kayles::print_move(move m, std::ostream& str)
{
    int take;
    int smaller;
    int larger;
    decode(m, take, smaller, larger);
    str << "kayles move: take " << take 
    << ", smaller " << smaller << ", larger " << larger << '\n';
}

game* kayles::inverse() const
{
    return new kayles(_value);
}

relation kayles::_order_impl(const game* rhs) const
{
    const kayles* other = reinterpret_cast<const kayles*>(rhs);
    assert(dynamic_cast<const kayles*>(rhs) == other);

    const int val1 = value();
    const int val2 = other->value();

    if (val1 != val2)
        return val1 < val2 ? REL_LESS : REL_GREATER;

    return REL_EQUAL;
}
split_result kayles::_split_impl() const
{
    assert(_value >= _smaller_part);
    split_result result = split_result(vector<game*>());
    if (_value > 0) // At least one subgame left
    {
        result->push_back(new kayles(_value));
        if (_smaller_part > 0) // two subgames
            result->push_back(new kayles(_smaller_part));
    }
    return result;
}

// Values from https://en.wikipedia.org/wiki/Kayles
int kayles::static_result(int n)
{
    static int small_values[] =
    {/* 0+ */ 0, 1, 2, 3, 1, 4, 3, 2, 1, 4, 2, 6,
    /* 12+ */ 4, 1, 2, 7, 1, 4, 3, 2, 1, 4, 6, 7,
    /* 24+ */ 4, 1, 2, 8, 5, 4, 7, 2, 1, 8, 6, 7,
    /* 36+ */ 4, 1, 2, 3, 1, 4, 7, 2, 1, 8, 2, 7,
    /* 48+ */ 4, 1, 2, 8, 1, 4, 7, 2, 1, 4, 2, 7,
    /* 60+ */ 4, 1, 2, 8, 1, 4, 7, 2, 1, 8, 6};

    static int periodic_values[] =
    {4, 1, 2, 8, 1, 4, 7, 2, 1, 8, 2, 7};

    assert(n >= 0);
    if (n <= 70)
        return small_values[n];
    else
        return periodic_values[n % 12];
}

void kayles::set_solved(int nim_value)
{
    assert(_smaller_part == 0);
    if (! is_solved())
    {
        kayles::store(_value, nim_value);
        impartial_game::set_solved(nim_value);
    }
}

//---------------------------------------------------------------------------
class kayles_move_generator : public move_generator
{
public:
    kayles_move_generator(const kayles& game);
    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    const kayles& _game;
    int _smaller_number;
    int _take; // Take 1 or 2 pebbles. 0 is a flag meaning no more moves
};

kayles_move_generator::kayles_move_generator(const kayles& game)
    : move_generator(BLACK), _game(game), _smaller_number(0),
    _take(1)
{
    if (_game.value() < 1)
        _take = 0;
}

void kayles_move_generator::operator++()
{
    ++_smaller_number;
    if (_take + 2 * _smaller_number > _game.value())
    {
        if (_take == 1 && (_game.value() > 1))
        {
            _take = 2;
            _smaller_number = 0;
        }
        else
            _take = 0;
    }
}

kayles_move_generator::operator bool() const
{
    return _take > 0;
}

move kayles_move_generator::gen_move() const
{
    int x = _smaller_number;
    int y = _game.value() - x - _take;
    assert(x <= y);
    return kayles::encode(_take, x, y);
}

//---------------------------------------------------------------------------
move_generator* kayles::create_move_generator() const
{
    return new kayles_move_generator(*this);
}
