#include "split_test_utils.h"

#include <algorithm>

using std::string;

//////////////////////////////////////// helper functions

void assert_strip_split_result(const strip* g, vector<string> expected, bool non_empty)
{
    split_result result = non_empty ? g->split_non_empty() : g->split();

    assert(result);

    vector<string> got;

    for (game* g2 : *result)
    {
        strip* gs = dynamic_cast<strip*>(g2);
        assert(gs != nullptr);

        got.push_back(gs->board_as_string());

        delete g2;
    }

    sort(expected.begin(), expected.end());
    sort(got.begin(), got.end());

    assert(expected.size() == got.size());
    for (int i = 0; i < got.size(); i++)
    {
        assert(expected[i] == got[i]);
    }
}

void assert_no_split(const game* g, bool non_empty)
{
    if (non_empty)
        assert(!g->split_non_empty().has_value());
    else
        assert(!g->split().has_value());
}



