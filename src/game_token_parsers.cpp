#include "game_token_parsers.h"
#include "cgt_up_star.h"
#include "utilities.h"
#include "cgt_dyadic_rational.h"
#include "cgt_switch.h"
#include <algorithm>
#include <string>
#include <vector>
#include <cassert>
#include <cstddef>

using namespace std;

////////////////////////////////////////////////// helper functions
namespace {

/*
    Calls split_string() on the line, after surrounding special characters with
    spaces
*/
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

inline bool is_comma(const string& str)
{
    return str == ",";
}

inline bool is_slash(const string& str)
{
    return str == "/";
}

////////////////////////////////////////////////// "get_X()" helper functions
/*      These functions will:

   Return true on success, false on failure.
   Increment idx as they consume from string_tokens.
   Not cause memory errors (i.e. when idx is past the range of string_tokens).
*/

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

// also matches ints
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

// succeeds IFF no comma, or comma with input afterward
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

/*
    Also matches empty list

   Spaces or commas can separate list items, but commas cannot be at the
   end of the list

   i.e.
       "1, 1/2, 3/ 4, 3 / 4"
       "1  /  4  4"
       " 3 1 / 4 6 "
       ""
    are all valid
*/
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

} // namespace

////////////////////////////////////////////////// game parsers

// non-empty "list" of at most 1 star and at most 1 int
game* up_star_parser::parse_game(const string& game_token) const
{
    vector<string> string_tokens = get_string_tokens(game_token, {','});
    const size_t N = string_tokens.size();
    size_t idx = 0;

    int n_ints = 0;
    int n_stars = 0;

    int ups = 0;
    bool star = false;

    auto get_up_or_star = [&]() -> bool
    {
        if (!(idx < N))
            return false;

        int int_val;
        if (get_int(string_tokens, idx, int_val))
        {
            n_ints++;
            ups += int_val;
            return true;
        }

        bool star_val;
        if (get_star(string_tokens, idx, star_val))
        {
            n_stars++;
            star ^= star_val;
            return true;
        }

        return false;
    };

    while (idx < N)
    {
        if (!get_up_or_star())
            return nullptr;
        if (!consume_optional_comma(string_tokens, idx))
            return nullptr;
    }

    assert(idx == N);
    assert(n_ints > 0 || n_stars > 0);

    if (n_ints > 1 || n_stars > 1)
        return nullptr;

    return new up_star(ups, star);
}

// "list" of 2 fractions
game* switch_game_parser::parse_game(const string& game_token) const
{
    vector<fraction> fracs;
    bool success = get_fraction_list(game_token, fracs);

    if (!success || fracs.size() != 2)
        return nullptr;

    assert(fracs.size() == 2);
    fraction& f1 = fracs[0];
    fraction& f2 = fracs[1];

    return new switch_game(f1, f2);
}

// "list" of 1 fraction
game* dyadic_rational_parser::parse_game(const string& game_token) const
{
    vector<fraction> fracs;
    bool success = get_fraction_list(game_token, fracs);

    if (!success || fracs.size() != 1)
        return nullptr;

    assert(fracs.size() == 1);
    fraction& f = fracs[0];

    return new dyadic_rational(f);
}
