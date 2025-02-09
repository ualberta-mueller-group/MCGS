#include "bounds.h"
#include <utility>
#include <vector>
#include "all_game_headers.h"
#include "cgt_up_star.h"
#include "clobber_1xn.h"
#include "sumgame.h"
#include "utilities.h"
#include <iostream>
#include <unordered_map>
#include <sstream>

using namespace std;


//              0 0 0   ?   0 0 0

/*

   LESS_OR_EQUAL
   FUZZY
   GREATER

   Start by finding the lower bound

   Find Gi such that 

   Gi <= S < Gi+1

   Gi <= S

   Gi+1 > S


*/


const int RADIUS = 8;
const int MIN = -RADIUS;
const int MAX = RADIUS;

const int DIAMETER = 2 * RADIUS + 1;



enum relation
{
    R_UNKNOWN = 0,
    R_LESS,
    R_GREATER,
    R_EQUAL,
    R_FUZZY,
};


const unordered_map<relation, string> rel_map {
    {R_UNKNOWN, "_"},
    {R_LESS, "<"},
    {R_GREATER, ">"},
    {R_EQUAL, "="},
    {R_FUZZY, "?"},

};


typedef pair<int, int> arena;


constexpr arena ARENA_INVALID {-2 * RADIUS, -3 * RADIUS};

ostream& operator<<(ostream& os, const arena& x)
{
    os << "[" << x.first << " " << x.second << "]";
    return os;
}



/*

   -2 -1 0 1 2

   0 1 2 3 4
*/

game* get_scale_game(int virtual_idx)
{
    return new up_star(virtual_idx, true);
}

game* get_inverse_scale_game(int virtual_idx)
{
    return new up_star(-virtual_idx, true);
}

void get_bounds(vector<game*>& games)
{
    int check_count = 0;

    relation grid[DIAMETER];

    for (int i = 0; i < DIAMETER; i++)
    {
        grid[i] = R_UNKNOWN;
    }


    vector<arena> arenas;
    vector<arena> arenas_next;


    auto virtual_to_real_idx = [](const int& virtual_idx) -> int
    {
        assert(virtual_idx >= MIN);
        assert(virtual_idx <= MAX);

        int real_idx = virtual_idx + RADIUS;

        assert(real_idx >= 0);
        assert(real_idx < DIAMETER);

        return real_idx;
    };

    auto valid_arena = [](const arena& ar) -> bool
    {
        return (ar.first <= ar.second) && (ar.first >= MIN) && (ar.second <= MAX);
    };

    auto step = [&games, &check_count, &grid, &virtual_to_real_idx, &valid_arena](arena& ar) -> arena
    {
        arena split_arena = ARENA_INVALID;

        int& low = ar.first;
        int& high = ar.second;

        assert(valid_arena(ar));

        int virtual_i = (high + low) / 2;
        int real_i = virtual_to_real_idx(virtual_i);

        cout << "Checking " << virtual_i << endl;
        check_count++;

        game* inverse_scale_game = get_inverse_scale_game(virtual_i);

        sumgame sum(BLACK);
    
        for (game* g : games)
        {
            sum.add(g);
        }

        sum.add(inverse_scale_game);

        bool black_first = sum.solve();

        sum.set_to_play(WHITE);
        bool white_first = sum.solve();

        // 0 0
        // S - Gi = 0
        // S = Gi
        // Gi = S
        if (!black_first && !white_first)
        {
            grid[real_i] = R_EQUAL; 
        }

        // 0 1
        // S - Gi < 0
        // S < Gi
        // Gi > S
        if (!black_first && white_first)
        {
            grid[real_i] = R_GREATER; 
            high = virtual_i - 1;
        }

        // 1 0
        // S - Gi > 0
        // S > Gi
        // Gi < S
        if (black_first && !white_first)
        {
            grid[real_i] = R_LESS;
            low = virtual_i + 1;
        }

        // 1 1
        // S - Gi ?= 0
        // S ?= Gi
        // Gi ?= S
        if (black_first && white_first)
        {
            grid[real_i] = R_FUZZY;
            split_arena = {virtual_i + 1, high};
            high = virtual_i - 1;
        }

        delete inverse_scale_game;

        return split_arena;
    };


    arenas.push_back({MIN, MAX});


    while (!arenas.empty())
    {
        arenas_next.clear();

        for (const arena& ar : arenas)
        {
            cout << ar << endl;
        }
        cout << endl;

        for (size_t i = 0; i < arenas.size(); i++)
        {
            arena& ar = arenas[i];

            // Do one step of binary search within "ar"
            arena split_arena = step(ar);

            if (valid_arena(ar))
            {
                arenas_next.push_back(ar);
            }

            if (valid_arena(split_arena))
            {
                arenas_next.push_back(split_arena);
            }

        }

        arenas.clear();
        swap(arenas, arenas_next);
    }


    const string sep = "\t";

    for (int i = MIN; i <= MAX; i++)
    {
        cout << i << sep;
    }
    cout << endl;

    for (int i = MIN; i <= MAX; i++)
    {
        game* scale_game = get_scale_game(i);

        stringstream stream;
        stream << *scale_game;
        const size_t skip_chars = string("up_star:").size();
        string str = stream.str().substr(skip_chars);

        cout << str << sep;

        delete scale_game;
    }
    cout << endl;

    for (int i = MIN; i <= MAX; i++)
    {
        int real_i = virtual_to_real_idx(i);

        relation r = grid[real_i];
        auto it = rel_map.find(r);
        assert(it != rel_map.end());
        const string& text = it->second;

        cout << text << sep;
    }
    cout << endl;




    cout << "Did " << check_count << " checks (" << (2 * check_count) << " sumgames)" << endl;
}


void test_bounds()
{

    vector<game*> games;

    games.push_back(new clobber_1xn("XOXO.XO.XOXOXO"));

    get_bounds(games);

    for (game* g : games)
    {
        delete g;
    }

}
