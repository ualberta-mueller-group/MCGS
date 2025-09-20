#include "grid_hash.h"

#include <iostream>
#include "grid.h"
#include "grid_location.h"

using namespace std;

////////////////////////////////////////////////// helper functions
namespace {
void print_grid(const int_pair& shape, const vector<int>& vec)
{
    for (int r = 0; r < shape.first; r++)
    {
        for (int c = 0; c < shape.second; c++)
        {
            cout << vec[r * shape.second + c] << ' ';
        }
        cout << endl;
    }
}

} // namespace

//////////////////////////////////////////////////
void test_grid_hash_stuff()
{
}
