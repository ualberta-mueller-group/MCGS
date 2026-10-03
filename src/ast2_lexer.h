/*
   Converts CGT environment input string into tokens, to be used by the parser.
*/
#pragma once

#include <vector>
#include <cstddef>
#include <optional>
#include <string>
#include <cstdint>

#include "ast2_token.h"

////////////////////////////////////////////////// class ast2_lexer
class ast2_lexer
{
public:
    // Line/column number args refer to first char of `env_string`
    [[nodiscard]] std::vector<ast2_token>&& get_tokens(
        const std::string* env_string, size_t line_no, size_t column_no);

private:
    static std::optional<token_type_enum> _get_token_type_simple_char(char c);

    // Consumes chars and advances _idx/_line_no/_column_no
    void _advance(size_t n_chars);

    // Constructs a token and calls `_advance(...)`
    ast2_token _make_token(token_type_enum token_type, size_t n_chars,
                           int64_t num);
    ast2_token _make_token(token_type_enum token_type, size_t n_chars);

    void _env_string_to_tokens();

    std::optional<ast2_token> _get_token_plusminus();
    std::optional<ast2_token> _get_token_simple_char();
    std::optional<ast2_token> _get_token_int();
    std::optional<ast2_token> _get_token_up_down();
    std::optional<ast2_token> _get_token_bar();
    std::optional<ast2_token> _get_token_identifier();
    std::optional<ast2_token> _get_token_exp_game_contents();

    // String between "cgt:" and ":cgt" in .test file
    const std::string* _env_string;

    // String index of next char to consume
    size_t _idx;
    // 1-indexed line/column of next char to consume
    size_t _line_no;
    size_t _column_no;

    // Resulting tokens
    std::vector<ast2_token> _tokens;
};
