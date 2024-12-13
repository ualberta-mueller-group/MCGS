#include "game.h"

#include <cassert>
#include <limits>

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
