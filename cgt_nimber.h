#ifndef cgt_nimber_H
#define cgt_nimber_H

#include "cgt_basics.h"
#include "game.h"
//---------------------------------------------------------------------------

class nimber : public game
{
public:
    nimber(int value);
    void play(const move& m, bw to_play);
    void undo_move();
    move_generator* create_move_generator(bw to_play) const;

    int value() const {return _value;}
private:
    int _value;
};

inline nimber::nimber(int value) : _value(value)
{ }
//---------------------------------------------------------------------------
#endif // cgt_nimber_H
