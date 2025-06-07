//---------------------------------------------------------------------------
// Simple combinatorial games - nimbers
//---------------------------------------------------------------------------
#include "sum_of_nimbers.h"

#include "cgt_nimber.h"
#include <vector>
#include <cassert>
#include "sumgame.h"

std::vector<int> get_nim_heaps(const sumgame& s)
{
    std::vector<int> heaps;
    for (auto nim : s.subgames())
    {
        const nimber* n = dynamic_cast<nimber*>(nim);
        assert(n);
        heaps.push_back(n->value());
    }
    return heaps;
}

bool static_solve(const sumgame& s)
{
    return nimber::nim_sum(get_nim_heaps(s)) != 0;
}
