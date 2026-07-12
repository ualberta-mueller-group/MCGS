#include "parsing_utilities.h"

#include <string>
#include <vector>
#include <cstddef>
#include <optional>
#include <algorithm>
#include <cassert>

#include "cgt_basics.h"
#include "search_utils.h"
#include "utilities.h"
#include "fraction.h"
#include "string_to_int.h"

using namespace std;

vector<string> get_string_tokens(const string& line,
                                 const vector<char>& special_chars)
{
    const size_t N = line.size();

    string new_line;
    new_line.reserve(N * 2);

    for (size_t i = 0; i < N; i++)
    {
        const char& c = line[i];

        if (find(special_chars.begin(), special_chars.end(), c) !=
            special_chars.end())
        {
            new_line.push_back(' ');
            new_line.push_back(c);
            new_line.push_back(' ');
        }
        else
        {
            new_line.push_back(c);
        }
    }

    return split_string(new_line);
}

bool get_star(const vector<string>& string_tokens, size_t& idx, bool& val)
{
    const size_t N = string_tokens.size();
    if (!(idx < N))
        return false;

    if (string_tokens[idx] != "*")
    {
        return false;
    }

    val = true;
    idx++;
    return true;
}

bool get_int(const vector<string>& string_tokens, size_t& idx, int& val)
{
    const size_t N = string_tokens.size();
    if (!(idx < N))
        return false;

    const string& token = string_tokens[idx];

    optional<int> int_optional = str_to_i_opt(token);

    if (!int_optional.has_value())
        return false;

    val = int_optional.value();

    idx++;
    return true;
}

bool get_win_loss(const vector<string>& string_tokens, size_t& idx, bool& win)
{
    const size_t N = string_tokens.size();
    if (!(idx < N))
        return false;

    const string& token = string_tokens[idx];

    if (token == "win")
        win = true;
    else if (token == "loss")
        win = false;
    else
        return false;

    idx++;
    return true;
}

bool get_player(const vector<string>& string_tokens, size_t& idx,
                ebw& player)
{
    const size_t N = string_tokens.size();
    if (!(idx < N))
        return false;

    const string& token = string_tokens[idx];

    if (token == "B")
        player = BLACK;
    else if (token == "W")
        player = WHITE;
    else if (token == "N")
        player = EMPTY;
    else
        return false;

    idx++;
    return true;
}

optional<fraction> get_fraction(const vector<string>& string_tokens, size_t& idx)
{
    const size_t initial_idx = idx;
    const size_t N = string_tokens.size();
    if (!(idx < N))
        return false;

    int top = 0;
    int bottom = 1;

    // must have 1st int
    if (!get_int(string_tokens, idx, top))
        return {};

    if (!(idx < N))
        return fraction(top, bottom);

    const string& second_token = string_tokens[idx];

    // Only continue if "/" next
    if (!is_slash(second_token))
        return fraction(top, bottom);

    assert(is_slash(second_token));
    idx++; // consume "/"

    // Must have 2nd int after "/"
    if (!get_int(string_tokens, idx, bottom))
    {
        idx = initial_idx;
        return {};
    }

    return fraction(top, bottom);
}

bool consume_optional_comma(const vector<string>& string_tokens, size_t& idx)
{
    const size_t N = string_tokens.size();
    if (!(idx < N))
        return true; // end of input OK

    const string& token = string_tokens[idx];

    // non-comma OK
    if (!is_comma(token))
        return true;

    // consume comma and expect something after
    idx++;
    return idx < N;
}

bool consume_mandatory_comma(const vector<string>& string_tokens, size_t& idx)
{
    const size_t N = string_tokens.size();
    if (!(idx < N))
        return false;

    if (!is_comma(string_tokens[idx]))
        return false;

    idx++;
    return true;
}

bool get_fraction_list(const string& line, vector<fraction>& fracs)
{
    assert(fracs.size() == 0);

    vector<string> string_tokens = get_string_tokens(line, {'/', ','});
    const size_t N = string_tokens.size();

    if (N == 0)
        return true;

    size_t i = 0;
    while (i < N)
    {
        // must have fraction
        optional<fraction> f = get_fraction(string_tokens, i);

        if (f.has_value())
            fracs.push_back(*f);
        else
            return false;
        
        if (!consume_optional_comma(string_tokens, i))
            return false;
    }

    assert(i == N);
    assert(fracs.size() > 0);
    return true;
}

bool get_int_list(const string& line, vector<int>& ints)
{
    assert(ints.size() == 0);

    vector<string> string_tokens = get_string_tokens(line, {','});
    const size_t N = string_tokens.size();

    if (N == 0)
        return true;

    int x;

    size_t i = 0;
    while (i < N)
    {
        // must have int
        if (!get_int(string_tokens, i, x))
            return false;

        ints.push_back(x);

        if (!consume_optional_comma(string_tokens, i))
            return false;
    }

    assert(i == N);
    assert(ints.size() > 0);
    return true;
}

bool get_run_command(const vector<string>& string_tokens, size_t& idx,
                     vector<run_command_t>& run_commands)
{
    const size_t N = string_tokens.size();
    if (!(idx < N))
        return false;

    run_command_t rc;
    assert(rc.expected_value.type() == SEARCH_VALUE_TYPE_NONE);

    // Must have player
    if (!get_player(string_tokens, idx, rc.player))
        return false;

    if (!(idx < N))
    {
        run_commands.push_back(rc);
        return true;
    }

    // Optional expected outcome
    if (is_black_white(rc.player))
    {
        bool win;
        if (get_win_loss(string_tokens, idx, win))
            rc.expected_value.set_win(win);
    }
    else
    {
        assert(rc.player == EMPTY);

        int nim_value;
        bool got_nimber = get_int(string_tokens, idx, nim_value);

        if (got_nimber)
        {
            if (nim_value < 0)
                return false;

            rc.expected_value.set_nimber(nim_value);
        }
    }

    run_commands.push_back(rc);
    return true;
}

bool get_run_command_list(const string& line,
                          vector<run_command_t>& commands)
{
    assert(commands.empty());

    vector<string> string_tokens = get_string_tokens(line, {','});
    const size_t N = string_tokens.size();

    if (N == 0)
        return true;

    size_t i = 0;
    while (i < N)
    {
        // must have command
        if (!get_run_command(string_tokens, i, commands))
            return false;

        // Must have either: no input remaining, OR comma AND more input
        if (i < N)
        {
            bool got_comma = consume_mandatory_comma(string_tokens, i);
            if (!got_comma || (got_comma && i >= N))
                return false;
        }
    }

    assert(i == N);
    assert(commands.size() > 0);
    return true;
}

optional<ThPoint> get_thpoint(const vector<string>& string_tokens, size_t& idx)
{
    const size_t initial_idx = idx;
    const size_t N = string_tokens.size();

    string thpoint_string;

    bool ok = true;
    bool has_left_bracket = false;
    bool has_right_bracket = false;

    while (idx < N)
    {
        if (!ok || has_right_bracket)
            break;

        const string& tok = string_tokens[idx];
        idx++;

        const size_t tok_size = tok.size();
        for (size_t j = 0; j < tok_size; j++)
        {
            const char c = tok[j];

            // Handle '('
            if (!has_left_bracket)
            {
                if (c != '(')
                {
                    ok = false;
                    break;
                }

                has_left_bracket = true;
                continue;
            }

            // Handle ')'
            if (c == ')')
            {
                assert(!has_right_bracket);

                if (j + 1 != tok_size)
                {
                    ok = false;
                    break;
                }

                has_right_bracket = true;
                break;
            }

            thpoint_string.push_back(c);
        }
    }

    if (!ok || !has_left_bracket || !has_right_bracket)
    {
        idx = initial_idx;
        return {};
    }

    const vector<string> new_tokens = get_string_tokens(thpoint_string, {',', '/'});
    size_t new_idx = 0;

    optional<fraction> f1 = get_fraction(new_tokens, new_idx);
    const bool got_comma = consume_mandatory_comma(new_tokens, new_idx);
    optional<fraction> f2 = get_fraction(new_tokens, new_idx);

    if (!f1.has_value() || !got_comma || !f2.has_value())
    {
        idx = initial_idx;
        return {};
    }

    const ThValue val1(f1->top(), f1->bottom());
    const ThValue val2(f2->top(), f2->bottom());

    return ThPoint(val1, val2);
}

bool get_scaffold(const vector<string>& string_tokens, size_t& idx,
                  vector<ThPoint>& scaffold_points)
{
    assert(scaffold_points.empty());

    while (1)
    {
        const optional<ThPoint> point = get_thpoint(string_tokens, idx);

        if (!point.has_value())
            break;

        scaffold_points.push_back(*point);
    }

    return !scaffold_points.empty();
}
