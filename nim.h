#ifndef nim_H
#define nim_H

#include <vector>
#include "cgt_basics.h"
#include "game.h"

using std::pair;
using std::vector;

// first = subgame, second = how many stones
typedef pair<int,int> nim_move;

class nim : public game
{
public:
    nim(); // empty game
    nim(std::string game_as_string);
    const vector<int>& heaps() const;
    // number of pebbles in _heaps[i]
    int heap(int i) const;
private:
    vector<int> _heaps;
}; // nim

inline nim::nim() : _heaps()
{ }

inline const vector<int>& nim::heaps() const
{
    return _heaps;
}

inline int nim::heap(int i) const
{
    return _heaps[i];
}

std::ostream& operator<<(std::ostream& out, const nim& g);

//---------------------------------------------------------------------------
class nim_move_generator : public move_generator
{
public:
    nim_move_generator(const nim& game);
    void operator++();
    operator bool() const;
    nim_move move() const;
private:
    const nim& _game;
    int _current_heap;
    int _current_number;
};

inline nim_move_generator::nim_move_generator(const nim& game) :
    _game(game), _current_heap(0), _current_number(0)
{ }

#endif // nim_H
