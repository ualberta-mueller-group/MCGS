#ifndef game_H
#define game_H

#include <string>
#include <ostream>

typedef int move;

class move_generator;

class game
{
public:
    game(int color);
    bool solve();
    int to_play() const;
    void set_to_play(int color);
    virtual bool find_static_winner(bool& success) const = 0;
    virtual void play(const move& m) = 0;
    virtual void undo_move() = 0;
    virtual move_generator* create_mg() const = 0;

private:
    int _to_play;
}; // game

inline game::game(int color) :
    _to_play(color)
{ }

inline int game::to_play() const
{
    return _to_play;
}

inline void game::set_to_play(int color)
{
    _to_play = color;
}

std::ostream& operator<<(std::ostream& out, const game& g);

//---------------------------------------------------------------------------
class move_generator
{
public:
//     move_generator(const game& g, int color) = 0;
    virtual void operator++() = 0;
    virtual operator bool() const = 0;
    virtual move gen_move() const = 0;
}; // move_generator

#endif // game_H
