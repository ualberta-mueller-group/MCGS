#include "split_test_utils.h"
#include <algorithm>

using std::string;
using std::vector;

void assert_strip_split_result(const strip* g, vector<string> expected)
{
    split_result result = g->split();

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
    for (size_t i = 0; i < got.size(); i++)
    {
        assert(expected[i] == got[i]);
    }
}

void assert_no_split(const game* g)
{
    assert(!g->split().has_value());
}
