#include "nim.h"

#include <string>
#include <sstream>

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
    _heaps(string_to_heaps(game_as_string))
{ }

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
        _current_number = 0;
        ++_current_heap;
    }
}

nim_move_generator::operator bool() const
{
    return _current_heap < _game.heaps().size();
}

nim_move nim_move_generator::move() const
{
    return std::make_pair(_current_heap, _current_number);
}
