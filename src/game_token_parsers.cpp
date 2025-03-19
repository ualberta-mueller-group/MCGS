#include "game_token_parsers.h"
#include "cgt_up_star.h"
#include "utilities.h"
#include "cgt_dyadic_rational.h"
#include "cgt_switch.h"
#include <algorithm>
#include <string>


using namespace std;

////////////////////////////////////////////////// helper functions
namespace {
vector<string> get_string_tokens(const string& line, const vector<char>& special_chars)
{
    string new_line;
    new_line.reserve(line.size() * 2);

    {
        const size_t N = line.size();
        for (size_t i = 0; i < N; i++)
        {
            const char& c = line[i];
            
            if (find(special_chars.begin(), special_chars.end(), c) != special_chars.end())
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

bool get_star(const vector<string>& string_tokens, size_t& idx, bool& val)
{
    assert(idx < string_tokens.size());
    if (string_tokens[idx] != "*")
    {
        val = false;
        return false;
    }

    val = true;
    idx++;
    return true;
}

bool get_int(const vector<string>& string_tokens, size_t& idx, int& val)
{
    assert(idx < string_tokens.size());
    const string& token = string_tokens[idx];

    if (!is_int(token))
        return false;

    val = stoi(token);
    idx++;
    return true;
}

bool get_fraction(const vector<string>& string_tokens, size_t& idx, vector<fraction>& fracs)
{
    assert(idx < string_tokens.size());

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

    if (!(idx < string_tokens.size()))
        return make_fraction();

    const string& second_token = string_tokens[idx];

    // Only continue if "/" next
    if (!is_slash(second_token))
        return make_fraction();

    assert(is_slash(second_token));
    idx++; // consume "/"

    // Must have 2nd int after "/"
    if (!(idx < string_tokens.size()))
        return false;
    if (!get_int(string_tokens, idx, bottom))
        return false;

    return make_fraction();
}

bool consume_optional_comma(const vector<string>& string_tokens, size_t& idx)
{
    // end of input OK
    if (!(idx < string_tokens.size()))
        return true;

    const string& token = string_tokens[idx];
    
    // non-comma OK
    if (!is_comma(token))
        return true;

    // consume comma and expect something after
    idx++;
    return idx < string_tokens.size();
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

    assert(fracs.size() > 0);
    return true;
}

} // namespace


//////////////////////////////////////////////////
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
        assert(idx < N);

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
