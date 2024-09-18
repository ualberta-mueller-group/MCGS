#ifndef nim_H
#define nim_H

#include <vector>
#include "cgt_basics.h"
#include "game.h"

using std::vector;

class nim : public game
{
public:
    nim(); // empty game
    nim(std::string game_as_string);
private:
    vector<int> _heaps;
}; // nim

inline nim() : _heaps()
{ }

#endif // nim_H
