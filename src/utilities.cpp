#include "utilities.h"
#include <sstream>


using std::vector, std::string, std::stringstream;

vector<string> split_string(const string& str)
{
    vector<string> strs;

    stringstream stream(str);

    string next_string;
    while (stream >> next_string)
    {
        strs.push_back(next_string);
    }

    return strs;
}

bool is_int(const string& str)
{
    const int N = str.size();

    if (N == 0)
    {
        return false;
    }

    for (int i = 0; i < N; i++)
    {
        const char& c = str[i];

        if (!isdigit(c) && (i != 0 || c != '-'))
        {
            return false;
        }
    }

    if (str[0] == '-' && str.size() < 2)
    {
        return false;
    }

    return true;
}

relation relation_from_search_results(bool le_known, bool is_le, bool ge_known, bool is_ge)
{
    // must resolve to a relation, can't be unknown

    assert(le_known || ge_known);

    if (le_known && ge_known)
    {
        if (!is_le && !is_ge) // 0 0
            return REL_FUZZY;
        if (!is_le && is_ge) // 0 1
            return REL_GREATER;
        if (is_le && !is_ge) // 1 0
            return REL_LESS;
        if (is_le && is_ge) // 1 1
            return REL_EQUAL;

        assert(false);
    }

    if (le_known && is_le)
    {
        assert(!ge_known);
        return REL_LESS_OR_EQUAL;
    }

    if (ge_known && is_ge)
    {
        assert(!le_known);
        return REL_GREATER_OR_EQUAL;
    }

    assert(false);
}

