#pragma once
#include "game.h"
#include "utilities.h"
#include <cctype>
#include <string>

/*
    New games must have a corresponding call to file_parser::add_game_parser()
   in file_parser::init_game_parsers()

   It's tempting to rewrite int_parser<T>, int2_parser<T> etc into
   int_parser<T, N> -- probably a bad idea?
*/

class game_token_parser
{
public:
    virtual ~game_token_parser() {}

    /*
        Input: string representing a game


        Returns a new game owned by the caller, OR nullptr if an error
            occurred
    */
    virtual game* parse_game(const std::string& game_token) const = 0;
};

//////////////////////////////////////// implementations

// forwards the string as-is to the game constructor
template <class T>
class basic_parser : public game_token_parser
{
public:
    game* parse_game(const std::string& game_token) const override
    {
        return new T(game_token);
    }
};

// takes a string of 1 and only 1 int, passes it to the game constructor
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

// takes a string of 2 and only 2 ints, passes them to the game constructor
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

////////////////////////////////////////////////// special cases
class up_star_parser : public game_token_parser
{
public:
    game* parse_game(const std::string& game_token) const override;
};

class switch_game_parser : public game_token_parser
{
public:
    game* parse_game(const std::string& game_token) const override;
};

class dyadic_rational_parser : public game_token_parser
{
public:
    game* parse_game(const std::string& game_token) const override;
};
