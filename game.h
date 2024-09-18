#ifndef game_H
#define game_H

#include <string>
#include <ostream>

class game
{
public:
    game() = 0; // empty game
    game(std::string game_as_string) = 0;
    bool solve();
}; // game


std::ostream& operator<<(std::ostream& out, const game& g);

class game_move_generator
{
public:
    game_move_generator(const game& g, int color) = 0;
    void operator++() = 0;
    operator bool() const = 0;

private:
    const game& _g;
    int _color;
    int _current;
}; // game_move_generator

#endif // game_H
