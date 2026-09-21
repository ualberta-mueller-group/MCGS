#include "ast2_token.h"

#include <cassert>
#include <iostream>

using namespace std;

////////////////////////////////////////////////// enum token_type_enum
string token_type_to_string(token_type_enum type)
{
    switch (type)
    {
        case TOKEN_TYPE_PLUSMINUS:
            return "PLUSMINUS";
        case TOKEN_TYPE_PLUS:
            return "PLUS";
        case TOKEN_TYPE_MINUS:
            return "MINUS";
        case TOKEN_TYPE_STAR:
            return "STAR";
        case TOKEN_TYPE_SLASH:
            return "SLASH";
        case TOKEN_TYPE_COMMA:
            return "COMMA";
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
    os << "\"" << tok.str << "\"";
    os << " ";

    os << "L:" << tok.line_no;
    os << " ";

    os << "C:" << tok.column_no;
    os << " ";

    os << "Type:" << token_type_to_string(tok.type);
    os << " ";

    os << "#:" << tok.num;

    return os;
}

////////////////////////////////////////////////// struct ast2_token_scope
ast2_token_scope::ast2_token_scope(size_t start, size_t end)
    : start(start), end(end)
{
    assert(start <= end);
}

ast2_token_scope ast2_token_scope::cut(size_t new_start, size_t new_end) const
{
    assert(new_start >= start);
    assert(new_end <= end);
    return ast2_token_scope(new_start, new_end);
}
