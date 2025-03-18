#include "utilities.h"
#include <sstream>

using std::vector, std::string, std::stringstream;

////////////////////////////////////////
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
    const size_t N = str.size();

    if (N == 0)
    {
        return false;
    }

    for (size_t i = 0; i < N; i++)
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

bool string_starts_with(const std::string& str, const std::string& word)
{
    if (word.size() > str.size())
    {
        return false;
    }

    assert(str.size() >= word.size());

    const size_t word_len = word.size();
    for (size_t i = 0; i < word_len; i++)
    {
        if (str[i] != word[i])
        {
            return false;
        }
    }

    return true;
}

bool string_ends_with(const std::string& str, const std::string& word)
{
    if (word.size() > str.size())
    {
        return false;
    }

    assert(str.size() >= word.size());

    const size_t word_len = word.size();
    const size_t str_len = str.size();

    for (size_t i = 0; i < word_len; i++)
    {
        if (str[str_len - 1 - i] != word[word_len - 1 - i])
        {
            return false;
        }
    }

    return true;
}

////////////////////////////////////////
relation relation_from_search_results(bool le_known, bool is_le, bool ge_known,
                                      bool is_ge)
{
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
        return REL_LESS_OR_EQUAL;
    }

    if (ge_known && is_ge)
    {
        return REL_GREATER_OR_EQUAL;
    }

    return REL_UNKNOWN;
}
