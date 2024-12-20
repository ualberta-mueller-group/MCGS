#pragma once
#include "cgt_up_star.h"
#include "game.h"
#include <cctype>
#include <cstddef>
#include <string>
#include <sstream>
#include <iostream>

// TODO: Rewrite these by splitting strings on whitespace, instead of using stringstreams?

// quick hacks
inline std::vector<std::string> split(const std::string& str)
{
    std::vector<std::string> strs;


    std::stringstream stream(str);

    std::string next_string;
    while (stream >> next_string)
    {
        strs.push_back(next_string);
    }

    return strs;
}

inline bool is_int(const std::string& str)
{
    const int N = str.size();

    if (N == 0)
    {
        return false;
    }

    for (int i = 0; i < N; i++)
    {
        const char& c = str[i];

        if (!isdigit(c) && i != 0 && c != '-')
        {
            return false;
        }
    }

    return true;
}

class game_parser
{
public:

    virtual ~game_parser()
    {}

    virtual game* parse_game(const std::string& game_token) const = 0;
};

template <class T>
class basic_parser : public game_parser
{
public:

    game* parse_game(const std::string& game_token) const override
    {
        return new T(game_token);
    }
};

template <class T>
class int_parser : public game_parser
{
public:

    game* parse_game(const std::string& game_token) const override
    {
        std::stringstream stream(game_token);

        // Get one int
        int val = 0;
        stream >> val;

        // Failure if IO error or not EOF
        if (stream.bad() || !stream.eof())
        {
            return nullptr;
        }

        return new T(val);
    }
};

template <class T>
class int2_parser : public game_parser
{
public:

    game* parse_game(const std::string& game_token) const override
    {
        std::stringstream stream(game_token);

        // Get first int
        int val1 = 0;
        stream >> val1;

        // Failure if IO error OR EOF
        if (stream.bad() || stream.eof())
        {
            return nullptr;
        }

        // Get second int
        int val2 = 0;
        stream >> val2;

        // Failure if IO error or not EOF
        if (stream.bad() || !stream.eof())
        {
            return nullptr;
        }

        return new T(val1, val2);
    }
};

class up_star_parser : public game_parser
{
public:

    game* parse_game(const std::string& game_token) const override
    {
        std::vector<std::string> strs = split(game_token);

        if (strs.size() > 2)
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

            return nullptr;
        }

        if (star_count > 1 || int_count > 1)
        {
            return nullptr;
        }

        return new up_star(ups, star);
    }
};
