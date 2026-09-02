#pragma once
#include <string>
#include <vector>

////////////////////////////////////////////////// enum token_type_enum
enum token_type_enum
{
    TOKEN_TYPE_PLUS,
    TOKEN_TYPE_MINUS,
    TOKEN_TYPE_COMMA,
    TOKEN_TYPE_SLASH,
    TOKEN_TYPE_STAR,
    TOKEN_TYPE_BAR,
    TOKEN_TYPE_LBRACK,
    TOKEN_TYPE_RBRACK,
    TOKEN_TYPE_LBRACE,
    TOKEN_TYPE_RBRACE,
    TOKEN_TYPE_INT,
    TOKEN_TYPE_UP_DOWN,
    TOKEN_TYPE_COLON,
    TOKEN_TYPE_IDENT,
    TOKEN_TYPE_EXP_GAME_CONTENTS,
};

std::string token_type_to_string(token_type_enum type);

////////////////////////////////////////////////// struct ast2_token
struct ast2_token
{
    ast2_token(token_type_enum type, std::string str) : type(type), str(str), num(0) {}

    ast2_token(token_type_enum type, std::string str, int64_t num)
        : type(type), str(str), num(num)
    {
    }

    token_type_enum type;
    std::string str;
    int64_t num;
};

std::ostream& operator<<(std::ostream& os, const ast2_token& tok);

////////////////////////////////////////////////// struct ast2_token_scope
struct ast2_token_scope
{
    ast2_token_scope(const std::vector<ast2_token>* tokens);
    [[nodiscard]] ast2_token_scope cut(size_t new_start, size_t new_end) const;
    bool idx_inside_scope(size_t idx) const;
    bool idx_outside_scope(size_t idx) const;

private:
    ast2_token_scope(const std::vector<ast2_token>* tokens, size_t new_start,
            size_t new_end);

public:
    const std::vector<ast2_token>* tokens;
    const size_t start;
    const size_t end;
};

