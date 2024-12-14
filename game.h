//---------------------------------------------------------------------------
// Combinatorial game and move generator
//---------------------------------------------------------------------------

#ifndef game_H
#define game_H

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include "cgt_basics.h"
#include "cgt_move.h"
//---------------------------------------------------------------------------

using std::vector;
//---------------------------------------------------------------------------

class move_generator;
class game;
//---------------------------------------------------------------------------

typedef std::optional<std::vector<game*>> split_result;

class game
{
public:
    game();
    virtual ~game() { }
    bool is_active() const;
    void set_active(bool status);
    move last_move() const;
    // Used to verify that game is restored after search
    int moves_hash() const; // TODO do a proper implementation
    
    virtual void play(const move& m, bw to_play);
    virtual void undo_move();

    /*
        List of games to REPLACE current game. Empty list means game is 0.
        No value means split didn't occur. See std::optional

        TODO assert in sumgame::play_sum() and sumgame::undo_move() 
            that list never contains the original game object?
    */
    virtual split_result split() const;

    virtual move_generator* create_move_generator(bw to_play) const = 0;
    virtual void print(std::ostream& str) const = 0;
    virtual game* inverse() const = 0; // caller takes ownership

private:
    vector<move> _move_stack;
    bool _is_active;
}; // game

inline game::game() :
    _move_stack(),
    _is_active(true)
{ }

inline bool game::is_active() const
{
    return _is_active;
}

inline void game::set_active(bool status)
{
    _is_active = status;
}

inline move game::last_move() const
{
    return _move_stack.back();
}

inline void game::play(const move& m, int to_play)
{
    assert(cgt_move::get_color(m) == 0);
    const move mc = cgt_move::encode(m, to_play);
    _move_stack.push_back(mc);
//     std::cout << "move "<< cgt_move::print(m) << "\n";
//     std::cout << "move + color "<< cgt_move::print(mc) << std::endl;
}

inline void game::undo_move()
{
    _move_stack.pop_back();
}

inline split_result game::split() const
{
    return split_result(); // no value
}

inline int game::moves_hash() const
{
    return _move_stack.size();
}

inline std::ostream& operator<<(std::ostream& out, const game& g)
{
    g.print(out);
    return out;
}

//---------------------------------------------------------------------------
class move_generator
{
public:
    move_generator(bw to_play);
    virtual ~move_generator() { }
    int to_play() const { return _to_play; }
    int opponent() const { return ::opponent(_to_play); }

    virtual void operator++() = 0;
    virtual operator bool() const = 0;
    virtual move gen_move() const = 0;
private:
    const bw _to_play;
}; // move_generator

inline move_generator::move_generator(bw to_play) :
    _to_play(to_play)
{ }
//---------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const split_result& split);


#endif // game_H
