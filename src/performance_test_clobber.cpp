#include "performance_test_clobber.h"


#include <cassert>
#include <chrono>
#include <iostream>
#include "cgt_move.h"
#include "clobber_1xn.h"
#include "impartial_game_wrapper.h"

using std::cout;
using std::endl;
using std::string;
using std::chrono::high_resolution_clock;

using namespace std;
const int UNKNOWN = -1;

void test_nim_value(const string& s, int nim_value)
{
    clobber_1xn c(s);
    impartial_game_wrapper g(&c);
    chrono::time_point start = high_resolution_clock::now();
    const int v = g.search_with_tt(28);
    chrono::time_point end = high_resolution_clock::now();
    chrono::duration<double, std::milli> duration = end - start;
    if (nim_value != UNKNOWN)
        assert(v == nim_value);
    cout << "(XO)^" << s.length()/2 << ' '
         << s << ' '
         << v << ' '
         << duration.count()
         << endl;
}

void performance_test_clobber()
{
        static int Dai_Chen_result[] =  // -1 means they could not solve it
    { 0, 
      1, 3, 0, 2, 0, 2,  0, 3,  1,  4,           //  1-10
      6, 1, 0, 1, 3, 7, UNKNOWN, 3,  1,  0,      // 11-20
     UNKNOWN, 4, 0, 1, 3, 0,  4, 0,  2,  0,      // 21-30
      3, 1, 4, 6, 1, 0,  1, 3, UNKNOWN, UNKNOWN, // 31-40
      3, 1                                       // 41-42
     };
    
    // Fast up to 6, 7 takes a few seconds, 
    // 8 not solved in 5 minutes without a cache/table
    int limit = 20; 
    for (int i = 0; i < limit; ++i)
        test_nim_value(clobber_1xn::xoxo(i), Dai_Chen_result[i]);

}
