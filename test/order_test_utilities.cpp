#include "order_test_utilities.h"

#include "game.h"
#include <vector>
#include <algorithm>
#include <cassert>
#include "bit_array.h"

using namespace std;

namespace {

void assert_relation_is_conclusive(relation rel)
{
    assert(                      //
           rel == REL_LESS    || //
           rel == REL_EQUAL   || //
           rel == REL_GREATER    //
            );                   //
}

// smaller games first
bool sort_key(const game* g1, const game* g2)
{
    relation rel = g1->order(g2);
    assert_relation_is_conclusive(rel);
    return rel == REL_LESS;
}

} // namespace

void order_test_impl(vector<game*>& games)
{
    // First sort the games
    sort(games.begin(), games.end(), sort_key);

    const size_t N = games.size();
    const size_t N2 = N*N;

    // Store all pair-wise comparisons for later
    vector<relation> relations(N2, REL_UNKNOWN);

    auto at = [&relations, &N](size_t i, size_t j) -> relation&
    {
        assert(0 <= i && i < N);
        assert(0 <= j && j < N);
        const size_t idx = i * N + j;
        assert(0 <= idx && idx < relations.size());
        return relations[idx];
    };

    for (size_t i = 0; i < N; i++)
    {
        const game* g1 = games[i];

        for (size_t j = 0; j < N; j++)
        {
            const game* g2 = games[j];
            at(i, j) = g1->order(g2);
            at(j, i) = g2->order(g1);
        }
    }

    /*
       For each index pair i, j, game g_i's bit_array at index j will be
       true IFF g_i == g_j
    */
    vector<bit_array> equivalence_masks;
    for (size_t i = 0; i < N; i++)
        equivalence_masks.emplace_back(N);


    /*
        idx1 < idx2. Compares one pair of games, ensuring their relations are
        conclusive, consistent, and checks over several iterations that there's
        only one contiguous block of equal games.
    */
    auto do_step = [&](size_t idx1, size_t idx2, bool& expect_not_equal) -> void
    {
        assert(idx1 < idx2);

        const relation rel_left = at(idx1, idx2);
        const relation rel_right = at(idx2, idx1);

        assert_relation_is_conclusive(rel_left);
        assert_relation_is_conclusive(rel_right);

        bool is_equal = false;

        if (rel_left == REL_EQUAL || rel_right == REL_EQUAL)
        {
            assert(rel_left == REL_EQUAL && rel_right == REL_EQUAL);
            is_equal = true;
            equivalence_masks[idx1].set(idx2, true);
            equivalence_masks[idx2].set(idx1, true);
        }
        else
        {
            assert(rel_left == REL_LESS && rel_right == REL_GREATER);
            is_equal = false;
            expect_not_equal = true;
        }

        if (expect_not_equal)
            assert(!is_equal);
    };


    for (size_t i = 0; i < N; i++)
    {
        assert(at(i, i) == REL_EQUAL);
        equivalence_masks[i].set(i, true);

        // iterate left
        bool expect_not_equal = false;
        for (size_t j = i; j > 0;) // don't underflow unsigned value...
        {
            j--;
            do_step(j, i, expect_not_equal);
        }

        // iterate right
        expect_not_equal = false;
        for (size_t j = i + 1; j < N; j++)
        {
            do_step(i, j, expect_not_equal);
        }
    }

    // Check that all equivalence classes are consistent...
    assert(equivalence_masks.size() == N);
    for (size_t i = 0; i < N; i++)
    {
        const bit_array& mask_i = equivalence_masks[i];

        for (size_t j = i + 1; j < N; j++)
        {
            const bit_array& mask_j = equivalence_masks[j];
            bit_array_relation rel = mask_i.compare(mask_j);
            assert(rel == BIT_ARRAY_NO_OVERLAP || rel == BIT_ARRAY_ALL_OVERLAP);
        }
    }
}
