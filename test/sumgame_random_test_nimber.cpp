#include "cgt_nimber.h"
#include "sumgame.h"
#include "sum_of_nimbers.h"
#include "test_utilities.h"
#include <random>

namespace {
/*
void add_heap(sumgame& sum, int heap)
{
    sum.add(new nimber(heap));
}
*/

void assert_solve(sumgame& sum)
{
    assert_solve_sum(sum, BLACK, static_solve(sum));
}
} // namespace

void sumgame_random_test_nimber()
{
    const int min_heap = 0;
    const int max_heap = 4;
    const int min_num_heaps = 2;
    const int max_num_heaps = 6;
    const int num_tests = 10;
    const int max_pebbles = 12;

    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<int> heap_distr(min_heap, max_heap);
    std::uniform_int_distribution<int> num_heap_distr(min_num_heaps,
                                                      max_num_heaps);

    for (int i = 0; i < num_tests; ++i)
    {
        sumgame sum(BLACK);
        std::vector<nimber*> nimbers;

        const int num_heaps = num_heap_distr(generator);
        for (int j = 0; j < num_heaps; ++j)
        {
            const int heap = heap_distr(generator);
            if (num_heaps + heap >= max_pebbles)
                break;
            else
            {
                // add_heap(sum, heap);

                nimber* n = new nimber(heap);
                nimbers.push_back(n);

                sum.add(n);
            }
        }
        assert_solve(sum);

        for (nimber* n : nimbers)
        {
            delete n;
        }
    }
    // std::cout << "Ran " << num_tests << " random nim sums\n";
}
