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
{ }

nim_move_generator::operator bool() const
{
    return false;
}

//     const nim& _game;
//     int _current_game;
//     int _current_number;
