//---------------------------------------------------------------------------
// Simple combinatorial games - nimbers
//---------------------------------------------------------------------------
#pragma once


#include <vector>
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
    game* inverse() const;
    void print(std::ostream& str) const;
    static int nim_sum(const std::vector<int>& values); // uses Nim formula
    static int nim_sum(const std::vector<nimber*>& values); // uses Nim formula

private:
    int _value;
};

inline game* nimber::inverse() const
{ return new nimber(_value); }

inline nimber::nimber(int value) : _value(value)
{ }
//---------------------------------------------------------------------------
