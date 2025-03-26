#include "hashing_benchmark.h"
#include "clobber_1xn.h"
#include "nogo_1xn.h"
#include "elephants.h"
#include <unordered_set>
#include <chrono>

using namespace std;

namespace {

uint64_t n_zeroes = 0;
uint64_t n_collisions = 0;
uint64_t n_games = 0;
std::unordered_set<uint64_t> hash_set;

void test_hash(uint64_t full_hash)
{
    n_games++;

    uint64_t bottom_half_mask(-1);
    bottom_half_mask >>= 32;

    uint64_t hash = 0;
    hash ^= full_hash & bottom_half_mask;
    hash ^= (full_hash >> 32) & bottom_half_mask;

    if (hash == 0)
    {
        n_zeroes += 1;
        return;
    }

    return;

    auto it = hash_set.insert(hash);

    if (!it.second)
    {
        n_collisions += 1;
    }
}



} // namespace

void __benchmark_hash_function(hash_func_t& fn, const std::string& label)
{
    cout << "Test: " << label << endl;

    n_zeroes = 0;
    n_collisions = 0;
    n_games = 0;
    hash_set.clear();

    chrono::time_point start = chrono::high_resolution_clock::now();

    for (int size = 1; size <= 16; size++)
    {
        cout << "Size: " << size << endl;

        vector<int> board(size);

        for (int& x : board)
            x = 0;

        auto increment = [&]() -> bool
        {
            board.back() += 1;
            bool carry = false;

            for (auto it = board.rbegin(); it != board.rend(); it++)
            {
                int& val = *it;

                if (carry)
                {
                    val += 1;
                    carry = false;
                }

                if (val >= 3)
                {
                    assert((val / 3) < 2);
                    val %= 3;
                    carry = true;
                }

            }

            return !carry;
        };

        do
        {
            clobber_1xn c(board);
            uint64_t hash1 = fn(c);
            test_hash(hash1);

            nogo_1xn n(board);
            uint64_t hash2 = fn(n);
            test_hash(hash2);

            elephants e(board);
            uint64_t hash3 = fn(e);
            test_hash(hash3);

            //cout << c << " " << hash1 << endl;
            //cout << n << " " << hash2 << endl;
            //cout << endl;
        }
        while (increment());
    }

    chrono::time_point end = chrono::high_resolution_clock::now();
    chrono::duration<double, std::milli> duration = end - start;

    cout << "ZEROES: " << n_zeroes << endl;
    cout << "COLLISIONS: " << n_collisions << endl;
    cout << "GAMES: " << n_games << endl;

    cout << "Collision rate: " << 100.0 * (double) n_collisions / (double) n_games << endl;
    cout << "Duration (ms): " << duration.count() << endl;

    double rate = ((double) n_games) / (duration.count() / 1000.0);
    cout << rate << " hashes per second" << endl;
}
