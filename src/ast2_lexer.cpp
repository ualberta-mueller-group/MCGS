#include "ast2_lexer.h"

#include <cassert>
#include <cstdlib>

#include "integral_conversion.h"
#include "safe_arithmetic.h"
#include "string_to_int.h"
#include "utilities.h"

using namespace std;

////////////////////////////////////////////////// ast2_lexer methods
[[nodiscard]] vector<ast2_token>&& ast2_lexer::get_tokens(
    const string* env_string, size_t line_no, size_t column_no)
{
    assert(env_string != nullptr);

    _tokens.clear();

    _env_string = env_string;
    _idx = 0;
    _line_no = line_no;
    _column_no = column_no;

    _env_string_to_tokens();

    return std::move(_tokens);
}

optional<token_type_enum> ast2_lexer::_get_token_type_simple_char(char c)
{
    switch (c)
    {
        case '+':
            return TOKEN_TYPE_PLUS;
        case '-':
            return TOKEN_TYPE_MINUS;
        case ',':
            return TOKEN_TYPE_COMMA;
        case '/':
            return TOKEN_TYPE_SLASH;
        case '*':
            return TOKEN_TYPE_STAR;
        case '(':
            return TOKEN_TYPE_LBRACK;
        case ')':
            return TOKEN_TYPE_RBRACK;
        case '{':
            return TOKEN_TYPE_LBRACE;
        case '}':
            return TOKEN_TYPE_RBRACE;
        case ':':
            return TOKEN_TYPE_COLON;
    }

    return {};
}

void ast2_lexer::_advance(size_t n_chars)
{
    assert(_idx + n_chars <= _env_string->size());

    for (size_t i = 0; i < n_chars; i++)
    {
        const char c = (*_env_string)[_idx++];

        if (is_newline(c))
        {
            _line_no++;
            _column_no = 1;
        }
        else
            _column_no++;
    }
}

ast2_token ast2_lexer::_make_token(token_type_enum token_type, size_t n_chars,
                                   int64_t num)
{
    assert(_idx + n_chars <= _env_string->size());

    ast2_token tok(token_type, _env_string->substr(_idx, n_chars), num, _line_no,
                   _column_no);

    _advance(n_chars);

    return tok;
}

ast2_token ast2_lexer::_make_token(token_type_enum token_type, size_t n_chars)
{
    return _make_token(token_type, n_chars, 0);
}

#if defined(CALL_GET_TOKEN_FN)
#error Macro already defined!
#else
#define CALL_GET_TOKEN_FN(fn)                                                  \
    assert(!tok);                                                              \
    tok = fn();                                                                \
    if (tok)                                                                   \
    {                                                                          \
        _tokens.emplace_back(*tok);                                            \
        continue;                                                              \
    }                                                                          \
    static_assert(true)
#endif

void ast2_lexer::_env_string_to_tokens()
{
    assert(_idx == 0);
    assert(_env_string != nullptr);

    const size_t env_string_size = _env_string->size();
    while (_idx < env_string_size)
    {
        const char c = (*_env_string)[_idx];

        // Consume whitespace
        if (isspace(c))
        {
            _advance(1);
            continue;
        }

        // Get token
        optional<ast2_token> tok;

        CALL_GET_TOKEN_FN(_get_token_plusminus);
        CALL_GET_TOKEN_FN(_get_token_exp_game_contents);
        CALL_GET_TOKEN_FN(_get_token_simple_char);
        CALL_GET_TOKEN_FN(_get_token_int);
        CALL_GET_TOKEN_FN(_get_token_up_down);
        CALL_GET_TOKEN_FN(_get_token_bar);
        CALL_GET_TOKEN_FN(_get_token_identifier);

        THROW_ASSERT(false,
                     "Lexer error in CGT environment: unmatched text at line " +
                         to_string(_line_no) + " col " + to_string(_column_no) +
                         " beginning with char: '" + string(1, c) + "'");
    }

}

optional<ast2_token> ast2_lexer::_get_token_plusminus()
{
    if (_idx + 1 >= _env_string->size())
        return {};

    const char c1 = (*_env_string)[_idx];
    const char c2 = (*_env_string)[_idx + 1];

    if (!(c1 == '+' && c2 == '-'))
        return {};

    return _make_token(TOKEN_TYPE_PLUSMINUS, 2);
}

optional<ast2_token> ast2_lexer::_get_token_simple_char()
{
    if (_idx >= _env_string->size())
        return {};

    const char c = (*_env_string)[_idx];
    const optional<token_type_enum> type_opt = _get_token_type_simple_char(c);

    if (!type_opt)
        return {};

    return _make_token(*type_opt, 1);
}

optional<ast2_token> ast2_lexer::_get_token_int()
{
    const size_t env_string_size = _env_string->size();
    if (_idx >= env_string_size)
        return {};

    size_t n_digits = 0;
    for (size_t i = _idx; i < env_string_size; i++)
    {
        const char c = (*_env_string)[i];

        if (isdigit(c))
            n_digits++;
        else
            break;
    }

    if (n_digits == 0)
        return {};

    ast2_token tok = _make_token(TOKEN_TYPE_INT, n_digits);
    tok.num = str_to_ll(tok.str);

    THROW_ASSERT(negate_is_safe(tok.num));

    return tok;
}

optional<ast2_token> ast2_lexer::_get_token_up_down()
{
    const size_t env_string_size = _env_string->size();
    if (_idx >= env_string_size)
        return {};

    const char c_first = (*_env_string)[_idx];
    if (!(c_first == '^' || c_first == 'v'))
        return {};

    size_t arrow_count = 0;
    for (size_t i = _idx; i < env_string_size; i++)
    {
        const char c = (*_env_string)[i];

        if (c == c_first)
            arrow_count++;
        else
            break;
    }

    assert(arrow_count > 0);

    int64_t num_int = integral_cast_checked<int64_t>(arrow_count);
    THROW_ASSERT(negate_is_safe(num_int));
    if (c_first == 'v')
        num_int = -num_int;

    return _make_token(TOKEN_TYPE_UP_DOWN, arrow_count, num_int);
}

optional<ast2_token> ast2_lexer::_get_token_bar()
{
    const size_t env_string_size = _env_string->size();
    if (_idx >= env_string_size)
        return {};

    size_t bar_count = 0;
    for (size_t i = _idx; i < env_string_size; i++)
    {
        const char c = (*_env_string)[i];
        if (c == '|')
            bar_count++;
        else
            break;
    }

    if (bar_count == 0)
        return {};

    return _make_token(TOKEN_TYPE_BAR, bar_count,
                       integral_cast_checked<int64_t>(bar_count));
}

optional<ast2_token> ast2_lexer::_get_token_identifier()
{
    const size_t env_string_size = _env_string->size();

    if (_idx >= env_string_size || isdigit((*_env_string)[_idx]))
        return {};

    size_t ident_size = 0;
    for (size_t i = _idx; i < env_string_size; i++)
    {
        const char c = (*_env_string)[i];
        if (c == '_' || isalpha(c) || isdigit(c))
            ident_size++;
        else
            break;
    }

    if (ident_size == 0)
        return {};

    return _make_token(TOKEN_TYPE_IDENT, ident_size);
}

optional<ast2_token> ast2_lexer::_get_token_exp_game_contents()
{
    const size_t env_string_size = _env_string->size();
    if (_idx >= env_string_size || (*_env_string)[_idx] != '(')
        return {};

    const size_t tokens_size = _tokens.size();
    if (!(tokens_size >= 2 &&                                  //
          _tokens[tokens_size - 2].type == TOKEN_TYPE_IDENT && //
          _tokens[tokens_size - 1].type == TOKEN_TYPE_COLON)   //
    )
        return {};

    size_t size_including_brackets = 0;
    int bracket_stack_size = 0;

    for (size_t i = _idx; i < env_string_size; i++)
    {
        const char c = (*_env_string)[i];

        if (c == '(')
            bracket_stack_size++;
        if (c == ')')
            bracket_stack_size--;

        size_including_brackets++;

        if (bracket_stack_size == 0)
            break;
    }

    if (size_including_brackets < 2 || bracket_stack_size != 0)
        return {};

    assert(bracket_stack_size == 0 &&                                //
           (*_env_string)[_idx] == '(' &&                            //
           (*_env_string)[_idx + size_including_brackets - 1] == ')' //
    );

    _advance(1);
    ast2_token tok =
        _make_token(TOKEN_TYPE_EXP_GAME_CONTENTS, size_including_brackets - 2);
    _advance(1);

    return tok;
}

