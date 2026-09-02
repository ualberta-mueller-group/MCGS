#include "ast2_token.h"

#include <cassert>
#include <iostream>

using namespace std;

////////////////////////////////////////////////// enum token_type_enum
string token_type_to_string(token_type_enum type)
{
    switch (type)
    {
        case TOKEN_TYPE_PLUS:
            return "PLUS";
        case TOKEN_TYPE_MINUS:
            return "MINUS";
        case TOKEN_TYPE_COMMA:
            return "COMMA";
        case TOKEN_TYPE_SLASH:
            return "SLASH";
        case TOKEN_TYPE_STAR:
            return "STAR";
        case TOKEN_TYPE_BAR:
            return "BAR";
        case TOKEN_TYPE_LBRACK:
            return "LBRACK";
        case TOKEN_TYPE_RBRACK:
            return "RBRACK";
        case TOKEN_TYPE_LBRACE:
            return "LBRACE";
        case TOKEN_TYPE_RBRACE:
            return "RBRACE";
        case TOKEN_TYPE_INT:
            return "INT";
        case TOKEN_TYPE_UP_DOWN:
            return "UP_DOWN";
        case TOKEN_TYPE_COLON:
            return "COLON";
        case TOKEN_TYPE_IDENT:
            return "IDENTIFIER";
        case TOKEN_TYPE_EXP_GAME_CONTENTS:
            return "EXPLICIT_GAME_CONTENTS";
    }

    assert(false);
}

////////////////////////////////////////////////// struct ast2_token
ostream& operator<<(ostream& os, const ast2_token& tok)
{
    os << "Type:" << token_type_to_string(tok.type);
    os << " ";
    os << "\"" << tok.str << "\"";
    os << " ";
    os << "#:" << tok.num;

    return os;
}

////////////////////////////////////////////////// struct ast2_token_scope
ast2_token_scope::ast2_token_scope(const vector<ast2_token>* tokens)
    : tokens(tokens), start(0), end(tokens->size())
{
    // TODO initialization will already have dereferenced `tokens` by this point
    assert(tokens != nullptr);
}

ast2_token_scope ast2_token_scope::cut(size_t new_start, size_t new_end) const
{
    assert(new_start >= start);
    assert(new_end <= end);
    return ast2_token_scope(tokens, new_start, new_end);
}

bool ast2_token_scope::idx_inside_scope(size_t idx) const
{
    return (start <= idx && idx < end);
}

bool ast2_token_scope::idx_outside_scope(size_t idx) const
{
    return !idx_inside_scope(idx);
}

ast2_token_scope::ast2_token_scope(const vector<ast2_token>* tokens, size_t new_start,
                         size_t new_end)
    : tokens(tokens), start(new_start), end(new_end)
{
}


