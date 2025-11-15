#include "config_map.h"

#include <cctype>
#include <cstdlib>

#include "parsing_utilities.h"
#include "throw_assert.h"
#include "utilities.h"
#include "string_to_int.h"

using namespace std;


config_map::config_map(const string& config_string)
{
    size_t idx = 0;
    const size_t SIZE = config_string.size();

    string current_line;

    auto consume_whitespace = [&]() -> void
    {
        while (idx < SIZE)
        {
            const char c = config_string[idx];

            if (isspace(c))
                idx++;
            else
                break;
        }
    };

    auto get_line = [&]() -> bool
    {
        current_line.clear();

        bool found_semicolon = false;

        while (idx < SIZE)
        {
            const char c = config_string[idx];
            idx++;

            if (c == ';')
            {
                found_semicolon = true;
                break;
            }

            current_line.push_back(c);
        }

        return found_semicolon;
    };

    auto get_pair = [&]() -> bool
    {
        // Find '='
        bool found_eq = false;
        size_t eq_idx = 0;

        for (size_t i = 0; i < current_line.size(); i++)
        {
            if (current_line[i] == '=')
            {
                if (found_eq)
                    return false;

                found_eq = true;
                eq_idx = i;
            }
        }

        if (!found_eq ||                      //
            eq_idx == 0 ||                    //
            eq_idx == current_line.size() - 1 //
        )
            return false;

        // Split on equal
        string key = current_line.substr(0, eq_idx);
        string val = current_line.substr(eq_idx + 1);

        while (!key.empty() && isspace(key.back()))
            key.pop_back();

        while (!val.empty() && isspace(val.back()))
            val.pop_back();

        if (key.empty() || val.empty())
            return false;

        auto inserted = _key_value_map.insert({key, {val, false}});
        return inserted.second;
    };

    while (idx < SIZE)
    {
        consume_whitespace();

        if (!(idx < SIZE))
            break;

        const bool found_line = get_line();
        THROW_ASSERT(found_line);

        const bool found_pair = get_pair();
        THROW_ASSERT(found_pair);
    }
}

void config_map::check_unused_keys() const
{
    string unused_key_string;
    size_t unused_key_count = 0;

    for (const pair<const string, value_t>& p : _key_value_map)
    {
        if (!p.second.used)
        {
            if (unused_key_count > 0)
                unused_key_string += ", ";

            unused_key_string += "\"";
            unused_key_string += p.first;
            unused_key_string += "\"";

            unused_key_count++;
        }
    }

    THROW_ASSERT(unused_key_count == 0,
                 "config_map has unused keys: " + unused_key_string);
}

optional<vector<int>> config_map::get_int_vec(const string& key) const
{
    optional<vector<int>> result(vector<int>({}));

    const string* val_string = _get_value_string(key);
    if (val_string == nullptr)
        return {};

    // TODO use more general int_list function (add to parsing_utilities.h) ???
    vector<string> tokens = get_string_tokens(*val_string, {','});
    const size_t N = tokens.size();

    int x;

    size_t i = 0;
    while (i < N)
    {
        // must have int
        if (!::get_int(tokens, i, x))
            return {};

        result->push_back(x);

        if (!consume_optional_comma(tokens, i))
            return {};
    }

    assert(i == N);
    assert(result->size() > 0);
    return result;
}

optional<int_pair> config_map::get_dims(const string& key) const
{
    optional<vector<int>> vec = get_int_vec(key);

    if (!vec.has_value())
        return {};

    for (const int& val : *vec)
        if (val < 0)
            return {};

    if (vec->size() == 1)
        return int_pair(1, vec->back());
    if (vec->size() == 2)
        return int_pair((*vec)[0], (*vec)[1]);

    return {};
}

// TODO make non-optional versions to make error handling easier
optional<int> config_map::get_int(const string& key) const
{
    const string* val_string = _get_value_string(key);

    if (val_string == nullptr)
        return {};

    return str_to_i_opt(*val_string);
}

ostream& operator<<(ostream& os, const config_map& config)
{
    for (const pair<const string, config_map::value_t>& p :
         config._key_value_map)
    {
        os << "\"" << p.first << "\"";
        os << " USED: " << p.second.used;
        os << " -> ";
        os << "\"" << p.second.value_string << "\"" << endl;
    }

    return os;
}

const string* config_map::_get_value_string(const string& key) const
{
    auto it = _key_value_map.find(key);

    if (it == _key_value_map.end())
        return nullptr;

    const value_t& val_struct = it->second;
    val_struct.used = true;

    return &(val_struct.value_string);
}

