#pragma once
#include "cgt_up_star.h"
#include "game.h"
#include "utilities.h"
#include <cctype>
#include <cstddef>
#include <string>
#include <sstream>
#include <iostream>

// TODO: It's tempting to rewrite int_parser<T>, int2_parser<T> etc into int_parser<T, N> -- probably a bad idea?

class game_token_parser
{
public:

    virtual ~game_token_parser()
    {}

    virtual game* parse_game(const std::string& game_token) const = 0;
};

//////////////////////////////////////////////////////////// 

template <class T>
class basic_parser : public game_token_parser
{
public:

    game* parse_game(const std::string& game_token) const override
    {
        return new T(game_token);
    }
};

template <class T>
class int_parser : public game_token_parser
{
public:

    game* parse_game(const std::string& game_token) const override
    {
        std::vector<std::string> strs = split_string(game_token);

        if (strs.size() != 1 || !is_int(strs[0]))
        {
            return nullptr;
        }

        return new T(std::stoi(strs[0]));
    }
};

template <class T>
class int2_parser : public game_token_parser
{
public:

    game* parse_game(const std::string& game_token) const override
    {
        std::vector<std::string> strs = split_string(game_token);

        if (strs.size() != 2)
        {
            return nullptr;
        }

        for (const std::string& str : strs)
        {
            if (!is_int(str))
            {
                return nullptr;
            }
        }

        int val1 = std::stoi(strs[0]);
        int val2 = std::stoi(strs[1]);

        return new T(val1, val2);

    }
};

class up_star_parser : public game_token_parser
{
public:
    game* parse_game(const std::string& game_token) const override;
};
