#include "game_token_parsers.h"
#include "cgt_up_star.h"
#include "utilities.h"
#include "cgt_dyadic_rational.h"
#include "cgt_switch.h"


game* up_star_parser::parse_game(const std::string& game_token) const 
{
    std::vector<std::string> strs = split_string(game_token);

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
        const std::string& chunk = strs[i];

        if (chunk == "*")
        {
            star_count++;
            star = !star;
            continue;
        }

        if (is_int(chunk))
        {
            int_count++;
            ups = std::stoi(chunk);
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


game* switch_game_parser::parse_game(const std::string& game_token) const
{
    std::string token;
    token.reserve(game_token.size());

    for (size_t i = 0; i < game_token.size(); i++)
    {
        const char& c = game_token[i];

        if (c == ',')
        {
            token.push_back(' ');
            token.push_back(c);
            token.push_back(' ');
            continue;
        }
        token.push_back(c);
    }

    std::vector<std::string> strs = split_string(token);
    const size_t N = strs.size();
    size_t idx = 0;

    std::vector<dyadic_rational*> rationals;

    auto is_comma = [](const std::string& str) -> bool
    {
        return str.size() == 1 && str.back() == ',';
    };

    auto get_fraction = [&]() -> bool
    {
        if (!(idx + 2 < N && is_comma(strs[idx + 1])))
        {
            return false;
        }

        const std::string& str1 = strs[idx];
        const std::string& str2 = strs[idx + 1];
        const std::string& str3 = strs[idx + 2];

        if (!is_int(str1) || !is_comma(str2) || !is_int(str3))
        {
            return false;
        }

        int top = std::stoi(str1);
        int bottom = std::stoi(str3);

        rationals.push_back(new dyadic_rational(top, bottom));

        idx += 3;
        return true;
    };

    auto get_int = [&]() -> bool
    {
        if (!(idx < N && is_int(strs[idx])))
        {
            return false;
        }

        int top = std::stoi(strs[idx]);
        int bottom = 1;

        rationals.push_back(new dyadic_rational(top, bottom));

        idx += 1;
        return true;
    };

    auto delete_games = [&]() -> void
    {
        for (dyadic_rational* g : rationals)
        {
            delete g;
        }
        rationals.clear();
    };

    while (idx < N)
    {
        if (!get_fraction() && !get_int())
        {
            delete_games();
            return nullptr;
        }
    }

    assert(idx == N);

    if (rationals.size() != 2)
    {
        delete_games();
        return nullptr;
    }

    return new switch_game(rationals[0], rationals[1]);
}
