#include "new_random.h"

#include <cstdint>
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <map>

#include "histogram.h"
#include "random.h"
#include "utilities.h"

using namespace std;

//uint8_t out_width(-1);
//out_width -= (uint8_t) max_val;
//out_width += (uint8_t) min_val;



////////////////////////////////////////////////// new_random methods

new_random::new_random(uint64_t seed)
    : _rng(seed)
{
}

int8_t new_random::get_i8()
{
    return _rng();
}

int8_t new_random::get_i8_biased(int8_t min_val, int8_t max_val)
{
    assert(min_val <= max_val);

    if (min_val == INT8_MIN && max_val == INT8_MAX)
        return _rng();

    uint8_t range_width = 1 + ((uint8_t) max_val - (uint8_t) min_val);

    assert(((uint64_t) range_width) ==
           1 + ((uint64_t) max_val) - ((uint64_t) min_val));

    const uint8_t generated = _rng();

    return min_val + (generated % range_width);
}


static int max_iter = 0;

int8_t new_random::get_i8_unbiased(int8_t min_val, int8_t max_val)
{
    assert(min_val <= max_val);

    if (min_val == INT8_MIN && max_val == INT8_MAX)
        return _rng();

    uint8_t range_width = 1 + ((uint8_t) max_val - (uint8_t) min_val);
    uint8_t outer_width = 1 + (UINT8_MAX - range_width);

    assert(((uint64_t) range_width) ==
           1 + ((uint64_t) max_val) - ((uint64_t) min_val));

    assert(
            ((uint64_t) range_width) + ((uint64_t) outer_width)
            == ((uint64_t) UINT8_MAX) + 1
            );


    uint8_t excluded_width = outer_width % range_width;

    const uint8_t last_acceptable_value = UINT8_MAX - excluded_width;

    assert((uint64_t) outer_width + (uint64_t) range_width == 256);

    uint8_t generated;


    int iter = 0;
    do
    {
        iter++;
        generated = _rng();
    } while (generated > last_acceptable_value);

    if (iter > max_iter)
    {
        max_iter = iter;
        cout << "NEW WORST ITER: " << max_iter << " ";
        cout << "for: " << (int) min_val << " " << (int) max_val << " ";
        cout << "rejection window is " << (int) excluded_width << " ";
        cout << "last acceptable value is " << (int) last_acceptable_value << endl;
    }

    return min_val + (generated % range_width);
}


//////////////////////////////////////////////////

void test_new_random()
{
    new_random gen(ms_since_epoch());

    map<int64_t, int64_t> counts;

    const int64_t n_cases = 10000000;

    int64_t case_idx = 0;

    random_generator gen_old(ms_since_epoch());

    //for (; case_idx < n_cases; case_idx++)
        //counts[gen.get_i8_biased(3, 9)]++;
        //counts[gen.get_i8_unbiased(3, 9)]++;
        //counts[gen_old.get_i8(3, 9)]++;

    gen.get_i8_unbiased(2, 4);


    while (true)
    {

        if (case_idx >= n_cases)
            break;

        for (int8_t i = INT8_MIN; i <= INT8_MAX; i++)
        {
            if (i == INT8_MAX)
                break;

            if (case_idx >= n_cases)
                break;

            for (int8_t j = i; j <= INT8_MAX; j++)
            {
                if (case_idx >= n_cases)
                    break;

                case_idx++;
                counts[gen.get_i8_unbiased(i, j)] += 1;

                if (j == INT8_MAX)
                    break;
            }
        }
    }


    int64_t min_count = INT64_MAX;
    int64_t max_count = INT64_MIN;

    for (const std::pair<const int64_t, int64_t>& p : counts)
    {
        //cout << p.first << ": " << p.second << " (";
        //cout << (double) p.second / (double) n_cases << " %)" << endl;

        min_count = min(min_count, p.second);
        max_count = max(max_count, p.second);
    }

    cout << min_count << " " << max_count << " DIFF " << (max_count - min_count)
         << endl;

    cout << "MAX ITERATIONS " << max_iter << endl;
}
