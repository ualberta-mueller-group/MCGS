#include "bounds.h"
#include <limits>
#include <utility>
#include <vector>
#include "all_game_headers.h"
#include "cgt_up_star.h"
#include "clobber_1xn.h"
#include "nogo_1xn.h"
#include "sumgame.h"
#include "utilities.h"
#include <iostream>
#include <unordered_map>
#include <sstream>

using namespace std;

int step_count = 0;
int search_count = 0;

int tie_break_inversions = 0;
int tie_count = 0;
vector<int> inversion_steps;

enum tie_behavior_enum
{
    TB_PREDICT,
    TB_NEG,
    TB_POS,
};

tie_behavior_enum tie_behavior = TB_PREDICT;


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

enum optimization_level_enum
{
    OPT_NONE,

    OPT_1_SIDE,

    OPT_ASSUME_NEG,
    OPT_ASSUME_POS,

    OPT_2_SIDE,
    OPT_2_SIDE_NEG,
    OPT_2_SIDE_POS,
};



enum search_result
{
    RESULT_FALSE = false,
    RESULT_TRUE = true,
    RESULT_UNKNOWN,
};

static_assert(RESULT_UNKNOWN != RESULT_FALSE);
static_assert(RESULT_UNKNOWN != RESULT_TRUE);

//const int RADIUS = 16000;
const int RADIUS = 16000;
optimization_level_enum OPTIMIZATION_LEVEL = OPT_NONE;

const int MIN = -RADIUS;
const int MAX = RADIUS;

const int DIAMETER = 2 * RADIUS + 1;



enum relation
{
    R_UNKNOWN = 0,
    R_LESS,
    R_LE,
    R_GREATER,
    R_GE,
    R_EQUAL,
    R_FUZZY,
};


const unordered_map<relation, string> REL_MAP {
    {R_UNKNOWN, "_"},
    {R_LESS, "<"},
    {R_LE, "<="},
    {R_GREATER, ">"},
    {R_GE, ">="},
    {R_EQUAL, "="},
    {R_FUZZY, "?"},

};

const vector<string> GAME_PREFIXES {
    "up_star:",
    "dyadic_rational:",
};

void remove_game_prefixes(string& str)
{
    for (const string& prefix : GAME_PREFIXES)
    {
        size_t idx = str.find(prefix);

        if (idx != string::npos)
        {
            assert(idx == 0);
            str = str.substr(prefix.size());
            break;
        }

    }

}



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

/*
game* get_scale_game(int virtual_idx)
{
    return new dyadic_rational(virtual_idx, 8);
}

game* get_inverse_scale_game(int virtual_idx)
{
    return new dyadic_rational(-virtual_idx, 8);
}
*/


const bool silent = true;


void get_bounds(vector<game*>& games)
{
    const int BOUND_UNDEFINED = std::numeric_limits<int>::min();

    int bound_low = BOUND_UNDEFINED;
    int bound_high = BOUND_UNDEFINED;

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

    auto step = [&](arena& ar) -> arena
    {
        step_count++;
        arena split_arena = ARENA_INVALID;

        int& low = ar.first;
        int& high = ar.second;

        assert(valid_arena(ar));

        int virtual_i = (high + low) / 2;
        int real_i = virtual_to_real_idx(virtual_i);

        if (!silent)
        {
            cout << "Checking " << virtual_i << endl;
        }

        game* inverse_scale_game = get_inverse_scale_game(virtual_i);

        sumgame sum(BLACK);
    
        for (game* g : games)
        {
            sum.add(g);
        }

        sum.add(inverse_scale_game);


        // BASIC APPROACH
        if (OPTIMIZATION_LEVEL == OPT_NONE)
        {
            search_count += 2;
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


                assert(bound_high == BOUND_UNDEFINED || virtual_i < bound_high);
                bound_high = virtual_i;

                assert(bound_low == BOUND_UNDEFINED || virtual_i > bound_low);
                bound_low = virtual_i;

                low = virtual_i + 1;
                high = virtual_i - 1;

            }

            // 0 1
            // S - Gi < 0
            // S < Gi
            // Gi > S
            if (!black_first && white_first)
            {
                grid[real_i] = R_GREATER; 
                high = virtual_i - 1;

                assert(bound_high == BOUND_UNDEFINED || virtual_i < bound_high);
                bound_high = virtual_i;
            }

            // 1 0
            // S - Gi > 0
            // S > Gi
            // Gi < S
            if (black_first && !white_first)
            {
                grid[real_i] = R_LESS;
                low = virtual_i + 1;

                assert(bound_low == BOUND_UNDEFINED || virtual_i > bound_low);
                bound_low = virtual_i;
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
        }


        // 1 SIDED OPTIMIZATION
        if (OPTIMIZATION_LEVEL == OPT_1_SIDE)
        {
            search_count++;
            bool black_first = sum.solve();

            // 0 ?
            // S - Gi <= 0
            // S <= Gi
            // Gi >= S
            if (!black_first)
            {
                grid[real_i] = R_GE; 
                high = virtual_i - 1;

                assert(bound_high == BOUND_UNDEFINED || virtual_i < bound_high);
                bound_high = virtual_i;
            } else
            {

                search_count++;
                sum.set_to_play(WHITE);
                bool white_first = sum.solve();

                // 1 0
                // S - Gi > 0
                // S > Gi
                // Gi < S
                if (black_first && !white_first)
                {
                    grid[real_i] = R_LESS;
                    low = virtual_i + 1;

                    assert(bound_low == BOUND_UNDEFINED || virtual_i > bound_low);
                    bound_low = virtual_i;
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

            }
        }

        // 2 SIDED OPTIMIZATION
        if (OPTIMIZATION_LEVEL == OPT_2_SIDE || OPTIMIZATION_LEVEL == OPT_ASSUME_NEG || OPTIMIZATION_LEVEL == OPT_ASSUME_POS)
        {
            static int tie_break_rule = -1;

            if (tie_behavior == TB_NEG)
            {
                tie_break_rule = -1;
            }

            if (tie_behavior == TB_POS)
            {
                tie_break_rule = 1;
            }

            search_result black_first = RESULT_UNKNOWN;
            search_result white_first = RESULT_UNKNOWN;
            
            // What side of the bound range are we on?
            int mid = 0;

            if (bound_low != BOUND_UNDEFINED && bound_high != BOUND_UNDEFINED)
            {
                mid = (bound_low + bound_high) / 2;
            }

            int bound_side = tie_break_rule;
            bool did_tie_break = true;

            if (virtual_i != mid)
            {
                did_tie_break = false;
                bound_side = virtual_i < mid ? -1 : 1;
            } else
            {
                tie_count++;
            }


            if (OPTIMIZATION_LEVEL == OPT_ASSUME_NEG)
            {
                bound_side = -1;
            }

            if (OPTIMIZATION_LEVEL == OPT_ASSUME_POS)
            {
                bound_side = 1;
            }


            assert(bound_side != 0);


            // If we're on the lower side, assume that we'll be increasing the lower bound
            // so check white first
            // Otherwise check black first...


            // First search
            bool conclusive = false;

            int local_search_count = 1;

            if (bound_side < 0)
            {
                sum.set_to_play(WHITE);
                white_first = (search_result) sum.solve();
                search_count++;

                if (white_first == false)
                {
                    conclusive = true;

                    // ? 0
                    // S - Gi >= 0
                    // S >= Gi
                    // Gi <= S
                    grid[real_i] = R_LE;
                    low = virtual_i + 1;
                    assert(bound_low == BOUND_UNDEFINED || virtual_i > bound_low);
                    bound_low = virtual_i;
                }
            } else
            {
                sum.set_to_play(BLACK);
                black_first = (search_result) sum.solve();
                search_count++;

                if (black_first == false)
                {
                    conclusive = true;

                    // 0 ?
                    // S - Gi <= 0
                    // S <= Gi
                    // Gi >= S
                    grid[real_i] = R_GE;
                    high = virtual_i - 1;

                    assert(bound_high == BOUND_UNDEFINED || virtual_i < bound_high);
                    bound_high = virtual_i;
                }
            }


            if (!conclusive)
            {
                local_search_count++;

                if (bound_side < 0)
                {
                    assert(black_first == RESULT_UNKNOWN);
                    assert(white_first != RESULT_UNKNOWN);

                    sum.set_to_play(BLACK);
                    black_first = (search_result) sum.solve();
                    search_count++;

                    if (black_first == false)
                    {
                        conclusive = true;

                        // 0 1
                        // S - Gi <= 0
                        // S <= Gi
                        // Gi >= S
                        grid[real_i] = R_GE;
                        high = virtual_i - 1;

                        assert(bound_high == BOUND_UNDEFINED || virtual_i < bound_high);
                        bound_high = virtual_i;
                    }
                } else
                {
                    assert(black_first != RESULT_UNKNOWN);
                    assert(white_first == RESULT_UNKNOWN);

                    sum.set_to_play(WHITE);
                    white_first = (search_result) sum.solve();
                    search_count++;

                    if (white_first == false)
                    {
                        conclusive = true;

                        // 1 0
                        // S - Gi >= 0
                        // S >= Gi
                        // Gi <= S
                        grid[real_i] = R_LE;
                        low = virtual_i + 1;
                        assert(bound_low == BOUND_UNDEFINED || virtual_i > bound_low);
                        bound_low = virtual_i;
                    }
                }
            }

            // If still inconclusive, we have "1 1"

            if (!conclusive)
            {
                assert(black_first == RESULT_TRUE);
                assert(white_first == RESULT_TRUE);

                grid[real_i] = R_FUZZY;
                split_arena = {virtual_i + 1, high};
                high = virtual_i - 1;
            }

            if (did_tie_break && conclusive && (local_search_count == 2))
            {
                tie_break_rule *= -1;
                tie_break_inversions++;
                inversion_steps.push_back(step_count - 1);
            }
        }



        delete inverse_scale_game;

        return split_arena;
    };


    arenas.push_back({MIN, MAX});


    while (!arenas.empty())
    {
        arenas_next.clear();

        if (!silent)
        {
            for (const arena& ar : arenas)
            {
                cout << ar << endl;
            }
            cout << endl;
        }

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



        // check for fuzzy range extending to MIN or MAX
        if (step_count >= 5 && (bound_low == BOUND_UNDEFINED || bound_high == BOUND_UNDEFINED))
        {

            sumgame sum(BLACK);

            for (game* g : games)
            {
                sum.add(g);
            }

            game* inverse_scale_game = get_inverse_scale_game(bound_low == BOUND_UNDEFINED ? MIN : MAX);

            sum.add(inverse_scale_game);

            sum.set_to_play(BLACK);
            bool black_first = sum.solve();

            if (black_first)
            {
                sum.set_to_play(WHITE);
                bool white_first = sum.solve();

                if (white_first)
                {
                    arenas_next.clear();
                    bound_low = 999999;
                    bound_high = -999999;
                } else
                {
                    assert(false);
                }
            } else
            {
                assert(false);
            }

            delete inverse_scale_game;

        }

        arenas.clear();
        swap(arenas, arenas_next);
    }


    const string sep = "\t";


    const int ROW_COLS = 18;
    const int N_ROWS = (DIAMETER / ROW_COLS) + ((DIAMETER % ROW_COLS) > 0);
    //const int N_ROWS = 0;


    if (!silent)
    {
        for (int n = 0; n < N_ROWS; n++)
        {
            int min = MIN + n * ROW_COLS;

            int max = min + ROW_COLS - 1;
            max = max > MAX ? MAX : max;

            for (int i = min; i <= max; i++)
            {
                cout << i << sep;
            }
            cout << endl;

            for (int i = min; i <= max; i++)
            {
                game* scale_game = get_scale_game(i);

                stringstream stream;
                stream << *scale_game;
                string str = stream.str();
                remove_game_prefixes(str);

                cout << str << sep;

                delete scale_game;
            }
            cout << endl;

            for (int i = min; i <= max; i++)
            {
                int real_i = virtual_to_real_idx(i);

                relation r = grid[real_i];
                auto it = REL_MAP.find(r);
                assert(it != REL_MAP.end());
                const string& text = it->second;

                cout << text << sep;
            }
            cout << endl;
            cout << endl;

        }
    }

    cout << "BOUNDS: [" << bound_low << " " << bound_high << "]" << endl; 


    /*
        Clobber:
        XOXO.XO.XOXOXO.XXOOXO

        radius  NONE    1_SIDE  2_SIDE  NEG     POS
        32      22      17      12      17      17
        16000   54      41      29      41      41



        Clobber:
        XOXOXOXO.XO.XXOO.OXXO

        radius  NONE    1_SIDE  2_SIDE  NEG     POS
        32      22      18      14      18      18
        16000   58      45      32      45      45


        Clobber:
        XXXXXXXO
        radius  NONE    1_SIDE  2_SIDE  NEG     POS
        32      10      9       8       9       9
        16000   30      18      25      26      18







    */
    cout << "Did " << step_count << " steps (" << search_count << " sumgames)" << endl;
}


void test_bounds()
{
     

    vector<game*> games;

    //games.push_back(new clobber_1xn("XOXO.XO.XOXOXO.XXOOXO"));
    //games.push_back(new clobber_1xn("XOXOXOXO.XO.XXOO.OXXO"));
    //games.push_back(new clobber_1xn("XXXXXXXO"));
    //games.push_back(new clobber_1xn("OOOOOOOX"));

    //games.push_back(new clobber_1xn("OOOXX.OXXOOX..XXOO.XOOXO"));
    games.push_back(new clobber_1xn("XXXOO.XOOXXO..OOXX.OXXOX"));

    //games.push_back(new clobber_1xn(""));

    //games.push_back(new clobber_1xn("OOXX.XXOO.XX.XO..XOOX.XXOOXOOO"));
    //games.push_back(new clobber_1xn("XXOO.OOXX.OO.OX..OXXO.OOXXOXXX"));

    //games.push_back(new nogo_1xn("..X."));



    //games.push_back(new clobber_1xn("XXXXXXXXXXXXO"));
    //games.push_back(new up_star(16000, true));

    
    



    vector<optimization_level_enum> opts {
        OPT_NONE,
        OPT_1_SIDE,

        OPT_2_SIDE,
        OPT_2_SIDE_NEG,
        OPT_2_SIDE_POS,

    };

    for (optimization_level_enum opt : opts)
    {
        step_count = 0;
        search_count = 0;

        tie_break_inversions = 0;
        tie_count = 0;
        inversion_steps.clear();

        tie_behavior = TB_PREDICT;

        if (opt == OPT_2_SIDE_NEG)
        {
            opt = OPT_2_SIDE;
            tie_behavior = TB_NEG;
        }

        if (opt == OPT_2_SIDE_POS)
        {
            opt = OPT_2_SIDE;
            tie_behavior = TB_POS;
        }


        OPTIMIZATION_LEVEL = opt;
        get_bounds(games);

        if (opt == OPT_2_SIDE)
        {

            cout << "Ties: " << tie_count << endl;
            cout << "Inversions: " << tie_break_inversions << endl;

            cout << "Inversion steps: ";
            {
                const size_t N = inversion_steps.size();

                for (size_t i = 0; i < N; i++)
                {
                    cout << inversion_steps[i];

                    if (i + 1 < N)
                    {
                        cout << " ";
                    }
                }

                cout << endl;
            }

        }




    }


    for (game* g : games)
    {
        delete g;
    }

}
