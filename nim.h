//---------------------------------------------------------------------------
// Implementation of the game of nim
// Move representation, state representation, play and undo move
// Nim move generator
//---------------------------------------------------------------------------

#ifndef nim_H
#define nim_H

#include <vector>
#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"

using std::vector;

//---------------------------------------------------------------------------
// encode/decode nim_move as move

inline move nim_move(int heap, int number)
{
    return cgt_move::two_part_move(heap, number);
}

inline int nim_heap(move m)
{
    return cgt_move::first(m);
}

inline int nim_number(move m)
{
    return cgt_move::second(m);
}
//---------------------------------------------------------------------------

// Nim class
// Implementation:
//      heaps with 0 pebbles are stripped out at construction time,
//      but can occur later during play
class nim : public game
{
public:
    nim(std::string game_as_string);
    
    // virtual functions
    void play(const move& m);
    void undo_move();
    move_generator* create_move_generator() const;
    
    // accessor/helper functions
    const vector<int>& heaps() const;
    
    int num_heaps() const;
    int heap_size(int i) const;
    
    bool static_solve() const; // use Nim formula
private:
    vector<int> _heaps;
}; // nim
//---------------------------------------------------------------------------

inline const vector<int>& nim::heaps() const
{
    return _heaps;
}

inline int nim::num_heaps() const
{
    return _heaps.size();
}

inline int nim::heap_size(int i) const
{
    return _heaps[i];
}

//---------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const nim& g);
//---------------------------------------------------------------------------

#endif // nim_H
