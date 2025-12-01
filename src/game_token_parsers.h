/*
    Components used by file_parser to parse specific games
*/
#pragma once

#include <string>
#include <cassert>
#include <vector>
#include <memory>
#include <cstddef>
#include <iostream>
#include <optional>

#include "game.h"
#include "parsing_utilities.h"
#include "utilities.h"
#include "string_to_int.h"

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

// forwards the string as-is to the game constructor, then calls is_legal()
template <class T>
class basic_parser_with_check : public game_token_parser
{
public:
    game* parse_game(const std::string& game_token) const override
    {
        T* g = new T(game_token);

        if (!g->is_legal())
        {
            std::cerr << "ERROR: game not legal!" << std::endl;

            delete g;
            return nullptr;
        }

        return g;
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

        if (strs.size() != 1)
            return nullptr;

        std::optional<int> int_optional = str_to_i_opt(strs[0]);

        if (!int_optional.has_value())
            return nullptr;

        return new T(int_optional.value());
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
            return nullptr;

        std::optional<int> val1_optional = str_to_i_opt(strs[0]);
        std::optional<int> val2_optional = str_to_i_opt(strs[1]);

        if (!val1_optional.has_value() || !val2_optional.has_value())
            return nullptr;

        return new T(val1_optional.value(), val2_optional.value());
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

////////////////////////////////////////////////// impartial games
class impartial_game_token_parser_wrapper : public game_token_parser
{
public:
    impartial_game_token_parser_wrapper(
        std::shared_ptr<game_token_parser>& parser)
        : _parser(parser)
    {
        assert(parser.get() != nullptr);
    }

    virtual ~impartial_game_token_parser_wrapper() {}

    game* parse_game(const std::string& game_token) const override;

private:
    std::shared_ptr<game_token_parser> _parser;
};

////////////////////////////////////////////////// parameterized ("gen_") games
/*
   TODO document this. Or better, put parameters into section titles?
*/
template <class T>
class basic_parameterized_game_parser : public game_token_parser
{
public:
    game* parse_game(const std::string& game_token) const override
    {
        std::vector<std::string> string_tokens =
            get_string_tokens(game_token, {':', ','});

        std::vector<int> param_vec;
        param_vec.reserve(string_tokens.size());

        const size_t N = string_tokens.size();
        size_t idx = 0;
        bool found_colon = false;

        int param;
        while (idx < N)
        {
            // Mandatory param
            if (!get_int(string_tokens, idx, param))
                return nullptr;

            param_vec.push_back(param);

            // Check for colon
            if (idx < N && string_tokens[idx] == ":")
            {
                found_colon = true;
                idx++;
                break;
            }

            // Otherwise mandatory comma
            if (!consume_mandatory_comma(string_tokens, idx))
                return nullptr;
        }

        // Should have idx + 1 == N
        if (idx + 1 != N || !found_colon)
            return nullptr;

        const std::string& game_string = string_tokens[idx];
        return new T(param_vec, game_string);
    }
};
