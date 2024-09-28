#include "nim.h"
#include "nim_test.h"

#include <iostream>
#include <random>

int heap_sum(const nim& game)
{
    int sum = 0;
    for (int h: game.heaps())
        sum += h;
    return sum;
}

void nim_random_test()
{
    const int min_heap = 0;
    const int max_heap = 4;
    const int min_num_heaps = 2;
    const int max_num_heaps = 6;
    const int num_tests = 100;
    const int max_pebbles = 12;
    
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<int> heap_distr(min_heap, max_heap);
    std::uniform_int_distribution<int> num_heap_distr(min_num_heaps, max_num_heaps);

    for (int i=0; i < num_tests; ++i)
    {
        nim game("");
        const int num_heaps = num_heap_distr(generator);
        for (int j=0; j < num_heaps; ++j)
        {
            const int heap = heap_distr(generator);
            game.add_heap(heap);
        }
        if (heap_sum(game) < max_pebbles) // it's a bit slow...
        {
            assert_solve(game);
        }
    }
    std::cout << "Ran " << num_tests << " random nim sums\n";
}
