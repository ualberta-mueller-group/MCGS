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
    game(LEFT),
    _heaps(string_to_heaps(game_as_string)),
    _move_stack()
{ }

void nim::play(const move& m)
{
    const int heap = nim_heap(m);
    const int number = nim_number(m);
    assert(heap >= 0);
    assert(heap < _heaps.size());
    assert(number > 0);
    assert(number <= _heaps[heap]);
    _heaps[heap] -= number;
    _move_stack.push_back(m);
}

 
void nim::undo_move()
{
    const move m = _move_stack.back();
    _move_stack.pop_back();
    const int heap = nim_heap(m);
    const int number = nim_number(m);
    _heaps[heap] += number;
}

move_generator* nim::create_mg() const
{
    return new nim_move_generator(*this);
}

std::ostream& operator<<(std::ostream& out, const nim& g)
{
    for(auto n: g.heaps())
        out << n << ' ';
    return out;
}

//---------------------------------------------------------------------------
void nim_move_generator::operator++()
{
    int heap = _game.heap(_current_heap);
    if (_current_number < heap)
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
    const int num_heaps = _game.heaps().size();
    while ((_current_heap < num_heaps) && (_game.heap(_current_heap) == 0))
        ++_current_heap;
}

nim_move_generator::operator bool() const
{
    return _current_heap < _game.heaps().size();
}

move nim_move_generator::gen_move() const
{
    return nim_move(_current_heap, _current_number);
}
