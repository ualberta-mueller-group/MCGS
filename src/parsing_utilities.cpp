#include "parsing_utilities.h"

#include <string>
#include <vector>
#include <cstddef>
#include <algorithm>
#include <cassert>

#include "cgt_basics.h"
#include "search_utils.h"
#include "utilities.h"
#include "fraction.h"

using std::string, std::vector;

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

    if (!is_int(token))
        return false;

    val = stoi(token);
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

bool get_player(const std::vector<std::string>& string_tokens, size_t& idx,
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

bool get_fraction(const vector<string>& string_tokens, size_t& idx,
                  vector<fraction>& fracs)
{
    const size_t N = string_tokens.size();
    if (!(idx < N))
        return false;

    int top = 0;
    int bottom = 1;

    auto make_fraction = [&]() -> bool
    {
        fracs.emplace_back(top, bottom);
        return true;
    };

    // must have 1st int
    if (!get_int(string_tokens, idx, top))
        return false;

    if (!(idx < N))
        return make_fraction();

    const string& second_token = string_tokens[idx];

    // Only continue if "/" next
    if (!is_slash(second_token))
        return make_fraction();

    assert(is_slash(second_token));
    idx++; // consume "/"

    // Must have 2nd int after "/"
    if (!(idx < N))
        return false;
    if (!get_int(string_tokens, idx, bottom))
        return false;

    return make_fraction();
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
        if (!get_fraction(string_tokens, i, fracs))
            return false;

        if (!consume_optional_comma(string_tokens, i))
            return false;
    }

    assert(i == N);
    assert(fracs.size() > 0);
    return true;
}

bool get_run_command(const std::vector<std::string>& string_tokens, size_t& idx,
                     std::vector<run_command_t>& run_commands)
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

bool get_run_command_list(const std::string& line,
                          std::vector<run_command_t>& commands)
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
