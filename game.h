#ifndef game_H
#define game_H

#include <string>
#include <ostream>

class game
{
public:
//     game() = 0; // empty game
//     game(std::string game_as_string) = 0;
    bool solve();
}; // game


std::ostream& operator<<(std::ostream& out, const game& g);

//---------------------------------------------------------------------------
class move_generator
{
public:
//     move_generator(const game& g, int color) = 0;
    virtual void operator++() = 0;
    virtual operator bool() const = 0;

private:
}; // move_generator

#endif // game_H
