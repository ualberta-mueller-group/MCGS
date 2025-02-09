#include "game.h"

#include <cassert>
#include <limits>
#include <memory>

using std::unique_ptr;

std::ostream& operator<<(std::ostream& os, const split_result& split)
{

    if (!split)
    {
        os << "<NO SPLIT>";
    } else
    {
        size_t N = split->size();
        assert(N < std::numeric_limits<size_t>::max());

        for (size_t i = 0; i < N; i++)
        {
            game* g = (*split)[i];

            os << *g;

            if (i + 1 < N)
            {
                os << " ";
            }
        }
    }

    return os;
}


bool game::has_moves() const
{
    unique_ptr<move_generator> gen_b = unique_ptr<move_generator>(create_move_generator(BLACK));

    if (*gen_b)
    {
        return true;
    }

    unique_ptr<move_generator> gen_w = unique_ptr<move_generator>(create_move_generator(WHITE));

    if (*gen_w) 
    {
        return true;
    }

    return false;
}
