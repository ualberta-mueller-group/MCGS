// NOTE: This test uses n^2 space
#pragma once
#include "game.h"
#include <vector>

/*
    Given a vector of games, sort them according to game::order(), then check
    that their game::order() relations satisfy all of the following conditions:

    For all pairs of games g_i and g_j:
    1: relations are "conclusive", i.e. g_i < g_j, or g_i == g_j, or g_i > g_j
        (no <=, >=, UNKNOWN, etc).
    2: relations are consistent:
        2.1: g_i == g_i
        2.2: g_i == g_j implies g_j == g_i
        2.3: g_i < g_j implies g_j > g_i

    3: Equality is transitive (informal "proof"):
        Suppose for games g1, g2, and g3, that g1 == g2, g2 == g3,
        but !(g1 == g3). Then the equivalence masks for g1 and g2 are:
        mask1 = 110
        mask2 = 111
        Then the loop comparing equivalence masks will fail an assert, because
        the two masks partially overlap.

        Therefore if the test passes, g1 == g3, so equality is transitive.

    4: < and > are transitive (informal "proof"):
        WLOG, consider just "<".

        [Fact 1]
        If g1 < g2, then g1 comes before g2 in the sorted vector. If not, then
        g2 comes before g1, and do_step() will fail an assert when it checks
        that g2 < g1 (not possible due to consistency).

        Now suppose for games g1, g2, and g3, that g1 < g2, g2 < g3, but
        !(g1 < g3).

        g1 < g2 --> g1 comes before g2 (by fact 1)
        g2 < g3 --> g2 comes before g3 (by fact 1)
        [Fact 2]
        Therefore g1 comes before g3

        Then, because relations are "conclusive" !(g1 < g3) means either
        g1 == g3, or g1 > g3.

        Case 1 (g1 == g3):
            Then do_step() will fail an assert when g1 is the outer loop's game,
            because g1 has to pass g2 to reach g3, causing the expect_not_equal
            test to fail.

        Case 2 (g1 > g3):
            Then g3 < g1 (by consistency), and by fact 1, g3 comes before g1.
            This contradicts fact 2.

        Therefore if the test passes, g1 < g3, meaning "<" is transitive
        (WLOG, ">" is transitive too).
*/

void order_test_impl(std::vector<game*>& games);
