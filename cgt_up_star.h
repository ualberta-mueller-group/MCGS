//---------------------------------------------------------------------------
// Simple combinatorial games - multiples of up, down, with/without star
//---------------------------------------------------------------------------
#ifndef cgt_up_star_H
#define cgt_up_star_H

#include "cgt_basics.h"
#include "game.h"

//---------------------------------------------------------------------------

class up_star : public game
{
public:
    up_star(int value, bool star);
    // m encodes the change in value and star for undo
    // m is a two part move where the second part is the star change
    // encoded as 0/1
    void play(const move& m, bw to_play);
    void undo_move();
    move_generator* create_move_generator(bw to_play) const;
    
    int num_ups() const { return _value;}
    bool has_star() const { return _star;}
private:
    int _value;
    bool _star;
};

inline up_star::up_star(int value, bool star) :
    _value(value), _star(star)
{ }
//---------------------------------------------------------------------------
#endif // cgt_up_star_H
