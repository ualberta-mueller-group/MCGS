#include "game_token_parsers.h"
#include "cgt_up_star.h"
#include "cgt_dyadic_rational.h"
#include "cgt_switch.h"
#include <string>
#include <vector>
#include <cassert>
#include <cstddef>
#include "impartial_game_wrapper.h"

#include "parsing_utilities.h"
using namespace std;


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

// impartial games
game* impartial_game_token_parser_wrapper::parse_game(const string& game_token) const
{
    game* g = _parser->parse_game(game_token);

    if (g == nullptr)
        return nullptr;

    return new impartial_game_wrapper(g, true);
}

