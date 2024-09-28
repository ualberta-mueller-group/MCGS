#include "nim.h"

#include <iostream>
#include <string>
#include <sstream>
#include "cgt_basics.h"

namespace {
vector<int> string_to_heaps(std::string game_as_string)
{
    std::stringstream stream(game_as_string);
    vector<int> result;
    int n; 
    while(stream >> n)
    {
        if (n > 0)
            result.push_back(n);
    }
    return result;
}

} // namespace

nim::nim(std::string game_as_string) :
    game(),
    _heaps(string_to_heaps(game_as_string))
{ }


void nim::add_heap(int heap)
{
    _heaps.push_back(heap);
}

void nim::play(const move& m, bw to_play)
{
    game::play(m, to_play);
    const int heap = nim_heap(m);
    const int number = nim_number(m);
    assert(heap >= 0);
    assert(heap < num_heaps());
    assert(number > 0);
    assert(number <= _heaps[heap]);
    _heaps[heap] -= number;
}

 
void nim::undo_move()
{
    // ignore to_play color
    const move m = cgt_move::decode(move_stack().back());
    const int heap = nim_heap(m);
    const int number = nim_number(m);
    _heaps[heap] += number;
    game::undo_move();
}

int nim::nim_value() const
{
    int sum = 0;
    for (int heap: _heaps)
    {
        sum ^= heap;
    }
    return sum;
}

bool nim::static_solve() const
{
    return nim_value() != 0;
}
//---------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const nim& g)
{
    for(auto n: g.heaps())
        out << n << ' ';
    return out;
}
//---------------------------------------------------------------------------

class nim_move_generator : public move_generator
{
public:
    nim_move_generator(const nim& game);
    void operator++();
    operator bool() const;
    move gen_move() const;
private:
    void skip_zeroes();
    const nim& _game;
    int _current_heap;
    int _current_number;
};

nim_move_generator::nim_move_generator(const nim& game) :
    move_generator(BLACK),
    _game(game),
    _current_heap(0),
    _current_number(1)
{
    skip_zeroes();
}

void nim_move_generator::operator++()
{
    const int size = _game.heap_size(_current_heap);
    if (_current_number < size)
        ++_current_number;
    else
    {
        _current_number = 1;
        ++_current_heap;
        skip_zeroes();
    }
}

void nim_move_generator::skip_zeroes()
{
    assert(_current_number == 1);
    const int num = _game.num_heaps();
    while (   _current_heap < num
           && _game.heap_size(_current_heap) == 0
          )
        ++_current_heap;
}

nim_move_generator::operator bool() const
{
    return _current_heap < _game.num_heaps();
}

move nim_move_generator::gen_move() const
{
    return nim_move(_current_heap, _current_number);
}
//---------------------------------------------------------------------------

move_generator* nim::create_move_generator(bw ignore_to_play) const
{
    return new nim_move_generator(*this);
}
//---------------------------------------------------------------------------
