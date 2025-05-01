#include "performance_test_nogo.h"


#include <cassert>
#include <chrono>
#include <iostream>
#include "cgt_move.h"
#include "nogo_1xn.h"
#include "impartial_game_wrapper.h"

using std::cout;
using std::endl;
using std::string;
using std::chrono::high_resolution_clock;

using namespace std;
const int UNKNOWN = -1;

void test_nogo(const string& s, int nim_value)
{
    nogo_1xn c(s);
    impartial_game_wrapper g(&c);
    chrono::time_point start = high_resolution_clock::now();
    const int v = g.search_with_tt(29);
    chrono::time_point end = high_resolution_clock::now();
    chrono::duration<double, std::milli> duration = end - start;
    if (nim_value != UNKNOWN)
        assert(v == nim_value);
    cout << "Nogo " << s.length() << ' ' << s << ' '
         << v << ' '
         << duration.count()
         << endl;
}

void performance_test_nogo()
{
    static int expected[] = // computed with this same program
    { 0, 
      0, 1, 0, 1, 2, 0, 1, 0, 1, 2,       //  1-10
      3, 1, 0, 3, 1, 0, 2, 2, 3, UNKNOWN, // 11-20
      UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, 
      UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, 
      UNKNOWN, UNKNOWN                    // 21-30
      };

    int limit = 30; 
    for (int i = 0; i < limit; ++i)
    {
        string s(i, '.');
        test_nogo(s, expected[i]);
    }
}
