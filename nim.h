#ifndef nim_H
#define nim_H

#include <vector>
#include "cgt_basics.h"
#include "game.h"

using std::pair;
using std::vector;

const int MAX_SIZE = (1 << 16);

inline move nim_move(int heap, int number)
{
    assert(sizeof(int) >= 4);
    assert(heap < MAX_SIZE);
    assert(number < MAX_SIZE);
    return heap * MAX_SIZE + number;
}

inline int nim_heap(move m)
{
    return m / MAX_SIZE;
}

inline int nim_number(move m)
{
    return m % MAX_SIZE;
}

class nim : public game
{
public:
    nim(std::string game_as_string);
    const vector<int>& heaps() const;
    // number of pebbles in _heaps[i]
    int heap(int i) const;
    bool find_static_winner(bool& success) const;
    void play(const move& m);
    void undo_move();
    move_generator* create_mg() const;
private:
    vector<int> _heaps;
    vector<move> _move_stack;
}; // nim

inline const vector<int>& nim::heaps() const
{
    return _heaps;
}

inline int nim::heap(int i) const
{
    return _heaps[i];
}

inline bool nim::find_static_winner(bool& success) const
{
    return false;
}


std::ostream& operator<<(std::ostream& out, const nim& g);

//---------------------------------------------------------------------------
class nim_move_generator : public move_generator
{
public:
    nim_move_generator(const nim& game);
    void operator++();
    operator bool() const;
    move gen_move() const;
private:
    void skip_zeroes();
    const nim& _game;
    int _current_heap;
    int _current_number;
};

inline nim_move_generator::nim_move_generator(const nim& game) :
    _game(game), _current_heap(0), _current_number(1)
{
    skip_zeroes();
}

#endif // nim_H
