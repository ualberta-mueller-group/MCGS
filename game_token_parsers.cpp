#include "game_token_parsers.h"
#include "cgt_up_star.h"


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

    for (int i = 0; i < strs.size(); i++)
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
