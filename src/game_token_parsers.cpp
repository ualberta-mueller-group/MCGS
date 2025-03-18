#include "game_token_parsers.h"
#include "cgt_up_star.h"
#include "utilities.h"
#include "cgt_dyadic_rational.h"
#include "cgt_switch.h"
#include <string>


using namespace std;

////////////////////////////////////////////////// helper functions
namespace {
inline bool is_comma(const string& str)
{
    return str == ",";
}

inline bool is_slash(const string& str)
{
    return str == "/";
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

bool get_fraction_list(const string& line, vector<fraction>& fracs)
{
    assert(fracs.size() == 0);

    string new_line;
    new_line.reserve(line.capacity() + 1);

    {
        const size_t N = line.size();
        for (size_t i = 0; i < N; i++)
        {
            const char& c = line[i];

            if (c == '/')
                new_line += " / ";
            else if (c == ',')
                new_line += " , ";
            else
                new_line.push_back(c);
        }
    }

    vector<string> string_tokens = split_string(new_line);
    const size_t N = string_tokens.size();
    if (N == 0)
        return true;

    size_t i = 0;
    while (i < N)
    {
        // must have fraction
        if (!get_fraction(string_tokens, i, fracs))
            return false;

        // optional comma
        if (i < N && is_comma(string_tokens[i]))
        {
            i++;

            // must have fraction after comma
            if (!(i < N))
                return false;
        }
    }

    assert(fracs.size() > 0);
    return true;
}

} // namespace


//////////////////////////////////////////////////

game* up_star_parser::parse_game(const string& game_token) const
{
    vector<string> strs = split_string(game_token);

    if (strs.size() > 2 || strs.size() == 0)
    {
        return nullptr;
    }

    int int_count = 0;
    int star_count = 0;

    int ups = 0;
    bool star = false;

    for (size_t i = 0; i < strs.size(); i++)
    {
        const string& chunk = strs[i];

        if (chunk == "*")
        {
            star_count++;
            star = !star;
            continue;
        }

        if (is_int(chunk))
        {
            int_count++;
            ups = stoi(chunk);
            continue;
        }

        // got chunk that isn't star or integer
        return nullptr;
    }

    if (star_count > 1 || int_count > 1)
    {
        return nullptr;
    }

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
