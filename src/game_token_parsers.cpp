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
    token.reserve(game_token.size() + 2 * 2); // space for 2 commas

    {
        const size_t N = game_token.size();
        for (size_t i = 0; i < N; i++)
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
    }

    std::vector<std::string> strs = split_string(token);
    const size_t N = strs.size();
    size_t idx = 0;

    std::vector<fraction> fracs;

    auto is_comma = [](const std::string& str) -> bool
    { return str.size() == 1 && str.back() == ','; };

    auto get_fraction = [&]() -> bool
    {
        // 3 tokens wide, middle is comma
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

        fracs.emplace_back(top, bottom);

        idx += 3;
        return true;
    };

    auto get_int = [&]() -> bool
    {
        // 1 token wide
        if (!(idx < N && is_int(strs[idx])))
        {
            return false;
        }

        int top = std::stoi(strs[idx]);
        int bottom = 1;

        fracs.emplace_back(top, bottom);

        idx += 1;
        return true;
    };

    while (idx < N)
    {
        if (!get_fraction() && !get_int())
        {
            return nullptr;
        }
    }

    if (idx != N)
    {
        assert(false);
        return nullptr;
    }

    if (fracs.size() != 2)
    {
        return nullptr;
    }

    for (fraction& f : fracs)
    {
        f.simplify();
    }

    return new switch_game(fracs[0], fracs[1]);
}
