#include "cgt_environment.h"
#include "integral_conversion.h"
#include "n_bit_int.h"
#include "safe_arithmetic.h"
#include "search_graph_debug.h"
#include "sign_enum.h"
#include "string_to_int.h"
#include "throw_assert.h"
#include "utilities.h"
#include "generic_graph_printer.h"

#include <cctype>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <type_traits>
#include <unistd.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <memory>
#include <fstream>
#include <optional>
#include <cassert>
#include <variant>

#include "ast2_token.h"
#include "ast2_node.h"

using namespace std;


template <class... Ts>
static bool ptr_variant_non_null(const std::variant<Ts...>& ptr_variant)
{
    return std::visit([](const auto& ptr) -> bool
    {
        return static_cast<bool>(ptr);
    }, ptr_variant);
}

template <class... Ts>
static bool ptr_variant_null(const std::variant<Ts...>& ptr_variant)
{
    return std::visit([](const auto& ptr) -> bool
    {
        return !static_cast<bool>(ptr);
    }, ptr_variant);
}

namespace {

class print_rule
{
public:
    print_rule(const string& name)
    {
        //cout << "+++ " << name << endl;
    }

    ~print_rule()
    {
        //cout << "--- " << name << endl;
    }

    //string name;
};


////////////////////////////////////////////////// Helper functions

class ast2_lexer
{
public:
    [[nodiscard]] vector<ast2_token>&& get_tokens(const string* env_string,
                                                  size_t line_no,
                                                  size_t column_no);

private:
    static optional<token_type_enum> _get_token_type_simple_char(char c);

    void _advance(size_t n_chars);
    ast2_token _make_token(token_type_enum token_type, size_t n_chars, int64_t num);
    ast2_token _make_token(token_type_enum token_type, size_t n_chars);

    void _env_string_to_tokens();

    optional<ast2_token> _get_token_plusminus();
    optional<ast2_token> _get_token_simple_char();
    optional<ast2_token> _get_token_int();
    optional<ast2_token> _get_token_up_down();
    optional<ast2_token> _get_token_bar();
    optional<ast2_token> _get_token_identifier();
    optional<ast2_token> _get_token_exp_game_contents();

    // String between "cgt:" and ":cgt" in .test file
    const string* _env_string;

    // String index of next char to consume
    size_t _idx;
    // 1-indexed line/column of next char to consume
    size_t _line_no;
    size_t _column_no;

    // Resulting tokens
    vector<ast2_token> _tokens;
};

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

//////////////////////////////////////// Parser functions
enum ast2_rule_enum
{
    AST2_RULE_NONE = 0,
    AST2_RULE_INTEGER,
    AST2_RULE_RATIONAL,
    AST2_RULE_UP,
    AST2_RULE_NIMBER,
    AST2_RULE_RATIONAL_UP_NIMBER,
    AST2_RULE_EXPLICIT_GAME,
    AST2_RULE_ATOMIC_GAME,
    AST2_RULE_PLUSMINUS_GAME,
    AST2_RULE_QUALIFIED_GAME,
    AST2_RULE_SUM,
    AST2_RULE_BRACKET_SUM,
    AST2_RULE_OPTION_LIST,
    AST2_RULE_BRACED_OPTION_LIST,
    AST2_RULE_UNBRACED_CGT_GAME,
    AST2_RULE_BRACED_CGT_GAME,
};

string ast2_rule_enum_to_string(ast2_rule_enum rule_type)
{
    switch (rule_type)
    {
        case AST2_RULE_NONE:
            return "NONE";
        case AST2_RULE_INTEGER:
            return "integer";
        case AST2_RULE_RATIONAL:
            return "rational";
        case AST2_RULE_UP:
            return "up";
        case AST2_RULE_NIMBER:
            return "nimber";
        case AST2_RULE_RATIONAL_UP_NIMBER:
            return "rational_up_nimber";
        case AST2_RULE_EXPLICIT_GAME:
            return "explicit_game";
        case AST2_RULE_ATOMIC_GAME:
            return "atomic_game";
        case AST2_RULE_PLUSMINUS_GAME:
            return "plusminus_game";
        case AST2_RULE_QUALIFIED_GAME:
            return "qualified_game";
        case AST2_RULE_SUM:
            return "sum";
        case AST2_RULE_BRACKET_SUM:
            return "bracket_sum";
        case AST2_RULE_OPTION_LIST:
            return "option_list";
        case AST2_RULE_BRACED_OPTION_LIST:
            return "braced_option_list";
        case AST2_RULE_UNBRACED_CGT_GAME:
            return "unbraced_cgt_game";
        case AST2_RULE_BRACED_CGT_GAME:
            return "braced_cgt_game";
    }

    assert(false);
}

class ast2_parser
{
public:
    unique_ptr<ast2_sum> parse(const vector<ast2_token>* tokens);

private:
    const ast2_token* _get_token(size_t idx) const;
    void _mark_consumed(size_t idx, ast2_rule_enum rule_type);

    const ast2_token_scope _get_active_scope() const;
    void _push_scope(ast2_token_scope scope);
    void _pop_scope();

    // Parse basic games
    unique_ptr<ast2_integer> _parse_ast2_integer(bool at_least_0);
    unique_ptr<ast2_rational> _parse_ast2_rational();
    unique_ptr<ast2_up> _parse_ast2_up();
    unique_ptr<ast2_nimber> _parse_ast2_nimber();
    unique_ptr<ast2_rational_up_nimber> _parse_ast2_rational_up_nimber();
    unique_ptr<ast2_explicit_game> _parse_ast2_explicit_game();

    // Parse composite games
    unique_ptr<ast2_atomic_game> _parse_ast2_atomic_game();
    unique_ptr<ast2_plusminus_game> _parse_ast2_plusminus_game();
    unique_ptr<ast2_qualified_game> _parse_ast2_qualified_game();

    // Parse CGT games
    unique_ptr<ast2_sum> _parse_ast2_sum();
    unique_ptr<ast2_bracket_sum> _parse_ast2_bracket_sum();
    unique_ptr<ast2_option_list> _parse_ast2_option_list();
    unique_ptr<ast2_braced_option_list> _parse_ast2_braced_option_list();
    unique_ptr<ast2_unbraced_cgt_game> _parse_ast2_unbraced_cgt_game();
    unique_ptr<ast2_braced_cgt_game> _parse_ast2_braced_cgt_game();

    // Data
    const vector<ast2_token>* _tokens;
    size_t _idx;

    vector<ast2_token_scope> _scope_stack;

    pair<size_t, ast2_rule_enum> _deepest_parse;
};

unique_ptr<ast2_sum> ast2_parser::parse(const vector<ast2_token>* tokens)
{
    assert(tokens != nullptr);

    _tokens = tokens;
    _idx = 0;

    _scope_stack.clear();
    _push_scope(ast2_token_scope(0, tokens->size()));

    _deepest_parse = {0, AST2_RULE_NONE};

    unique_ptr<ast2_sum> result = _parse_ast2_sum();

    assert(_scope_stack.size() == 1);

    // Error check
    const size_t n_consumed = _deepest_parse.first;
    if (n_consumed < tokens->size())
    {
        stringstream str;
        str << "CGT environment parser error! Last token consumed: ";

        if (n_consumed == 0)
            str << "None!";
        else
        {
            str << "Rule: " << ast2_rule_enum_to_string(_deepest_parse.second);
            str << " ";
            str << (*_tokens)[n_consumed - 1];
        }

        THROW_ASSERT(false, str.str());
    }

    return result;
}

const ast2_token* ast2_parser::_get_token(size_t idx) const
{
    const ast2_token_scope& scope = _get_active_scope();

    if (scope.start <= idx && idx < scope.end)
        return &((*_tokens)[idx]);

    return nullptr;
}

void ast2_parser::_mark_consumed(size_t idx, ast2_rule_enum rule_type)
{
    idx += 1;
    if (idx > _deepest_parse.first)
        _deepest_parse = {idx, rule_type};
}

const ast2_token_scope ast2_parser::_get_active_scope() const
{
    assert(!_scope_stack.empty());
    return _scope_stack.back();
}

void ast2_parser::_push_scope(ast2_token_scope scope)
{
    _scope_stack.push_back(scope);
}

void ast2_parser::_pop_scope()
{
    assert(!_scope_stack.empty());
    _scope_stack.pop_back();
}

// Parse basic games
unique_ptr<ast2_integer> ast2_parser::_parse_ast2_integer(bool at_least_0)
{
    print_rule pr("Integer");

    const ast2_token* tok1 = _get_token(_idx);
    if (!tok1)
        return nullptr;

    int64_t abs_value;
    bool is_negative = false;

    if (tok1->type == TOKEN_TYPE_MINUS)
    {
        if (at_least_0)
            return nullptr;

        _mark_consumed(_idx, AST2_RULE_INTEGER);
        is_negative = true;

        const ast2_token* tok2 = _get_token(_idx + 1);

        if (!tok2 || tok2->type != TOKEN_TYPE_INT)
            return nullptr;

        _mark_consumed(_idx + 1, AST2_RULE_INTEGER);
        abs_value = tok2->num;
    }
    else
    {
        if (tok1->type != TOKEN_TYPE_INT)
            return nullptr;

        _mark_consumed(_idx, AST2_RULE_INTEGER);
        abs_value = tok1->num;
    }

    THROW_ASSERT(abs_value >= 0 && negate_is_safe(abs_value));

    if (is_negative)
        abs_value = -abs_value;

    _idx += (1 + is_negative);
    return make_unique<ast2_integer>(abs_value);
}

unique_ptr<ast2_rational> ast2_parser::_parse_ast2_rational()
{
    print_rule pr("Rational");

    unique_ptr<ast2_integer> top = _parse_ast2_integer(false);
    if (!top)
        return nullptr;

    const size_t idx_after_top = _idx;

    const int64_t top_value = top->value;
    int64_t bottom_value = 1;

    const ast2_token* tok_slash = _get_token(_idx);
    if (tok_slash && tok_slash->type == TOKEN_TYPE_SLASH)
    {
        _mark_consumed(_idx, AST2_RULE_RATIONAL);
        _idx++;
        unique_ptr<ast2_integer> bottom = _parse_ast2_integer(false);

        if (bottom)
            bottom_value = bottom->value;
        else
            _idx = idx_after_top;
    }

    return make_unique<ast2_rational>(top_value, bottom_value);
}

unique_ptr<ast2_up> ast2_parser::_parse_ast2_up()
{
    print_rule pr("Up");

    const ast2_token* tok1 = _get_token(_idx);
    if (!tok1 || tok1->type != TOKEN_TYPE_UP_DOWN)
        return nullptr;
    _mark_consumed(_idx, AST2_RULE_UP);
    _idx++;

    const int64_t arrow_value = tok1->num;
    THROW_ASSERT(negate_is_safe(arrow_value));
    assert(arrow_value != 0);

    if (arrow_value != 1 && arrow_value != -1)
        return make_unique<ast2_up>(arrow_value);

    unique_ptr<ast2_integer> int_suffix = _parse_ast2_integer(true);
    if (int_suffix)
    {
        int64_t final_value = int_suffix->value;
        THROW_ASSERT(negate_is_safe(final_value));

        if (arrow_value == -1)
            final_value = -final_value;

        return make_unique<ast2_up>(final_value);
    }

    return make_unique<ast2_up>(arrow_value);
}

unique_ptr<ast2_nimber> ast2_parser::_parse_ast2_nimber()
{
    print_rule pr("Nimber");

    const ast2_token* tok1 = _get_token(_idx);
    if (!tok1 || tok1->type != TOKEN_TYPE_STAR)
        return nullptr;
    _mark_consumed(_idx, AST2_RULE_NIMBER);
    _idx++;

    unique_ptr<ast2_integer> int_suffix = _parse_ast2_integer(true);
    if (int_suffix)
    {
        const int64_t nim_value = int_suffix->value;
        THROW_ASSERT(nim_value >= 0);

        return make_unique<ast2_nimber>(nim_value);
    }

    return make_unique<ast2_nimber>(1);
}

unique_ptr<ast2_rational_up_nimber> ast2_parser::_parse_ast2_rational_up_nimber()
{
    print_rule pr("Rational_up_nimber");

    unique_ptr<ast2_rational> rational = _parse_ast2_rational();
    unique_ptr<ast2_up> up = _parse_ast2_up();
    unique_ptr<ast2_nimber> nimber = _parse_ast2_nimber();

    if (!(rational || up || nimber))
        return nullptr;

    return make_unique<ast2_rational_up_nimber>(
        std::move(rational), std::move(up), std::move(nimber));
}

unique_ptr<ast2_explicit_game> ast2_parser::_parse_ast2_explicit_game()
{
    print_rule pr("Explicit game");

    const ast2_token* tok1 = _get_token(_idx);
    const ast2_token* tok2 = _get_token(_idx + 1);
    const ast2_token* tok3 = _get_token(_idx + 2);

    if (tok1 && tok1->type == TOKEN_TYPE_IDENT)
        _mark_consumed(_idx, AST2_RULE_EXPLICIT_GAME);
    else
        return nullptr;

    if (tok2 && tok2->type == TOKEN_TYPE_COLON)
        _mark_consumed(_idx + 1, AST2_RULE_EXPLICIT_GAME);
    else
        return nullptr;

    if (tok3 && tok3->type == TOKEN_TYPE_EXP_GAME_CONTENTS)
        _mark_consumed(_idx + 2, AST2_RULE_EXPLICIT_GAME);
    else
        return nullptr;


    const string& game_title = tok1->str;
    const string& game_contents = tok3->str;

    _idx += 3;
    return make_unique<ast2_explicit_game>(game_title, game_contents);
}

// Parse composite games
unique_ptr<ast2_atomic_game> ast2_parser::_parse_ast2_atomic_game()
{
    print_rule pr("Atomic game");

    ast2_atomic_game::variant_t ptr_variant;

    ptr_variant = _parse_ast2_rational_up_nimber();
    if (ptr_variant_non_null(ptr_variant))
        return make_unique<ast2_atomic_game>(std::move(ptr_variant));

    ptr_variant = _parse_ast2_bracket_sum();
    if (ptr_variant_non_null(ptr_variant))
        return make_unique<ast2_atomic_game>(std::move(ptr_variant));

    ptr_variant = _parse_ast2_braced_cgt_game();
    if (ptr_variant_non_null(ptr_variant))
        return make_unique<ast2_atomic_game>(std::move(ptr_variant));

    ptr_variant = _parse_ast2_explicit_game();
    if (ptr_variant_non_null(ptr_variant))
        return make_unique<ast2_atomic_game>(std::move(ptr_variant));

    return nullptr;
}

unique_ptr<ast2_plusminus_game> ast2_parser::_parse_ast2_plusminus_game()
{
    print_rule pr("Plusminus game");

    const size_t idx_start = _idx;

    const ast2_token* tok1 = _get_token(_idx);
    if (!(tok1 && tok1->type == TOKEN_TYPE_PLUSMINUS))
        return nullptr;

    _mark_consumed(_idx, AST2_RULE_PLUSMINUS_GAME);
    _idx++;

    ast2_plusminus_game::variant_t ptr_variant;

    ptr_variant = _parse_ast2_atomic_game();
    if (ptr_variant_non_null(ptr_variant))
        return make_unique<ast2_plusminus_game>(std::move(ptr_variant));

    ptr_variant = _parse_ast2_braced_option_list();
    if (ptr_variant_non_null(ptr_variant))
        return make_unique<ast2_plusminus_game>(std::move(ptr_variant));

    _idx = idx_start;
    return nullptr;
}

unique_ptr<ast2_qualified_game> ast2_parser::_parse_ast2_qualified_game()
{
    print_rule pr("Qualified game");

    const size_t idx_start = _idx;

    ast2_qualified_game::variant_t ptr_variant;
    sign_enum unary_sign = SIGN_POSITIVE;

    ptr_variant = _parse_ast2_plusminus_game();

    if (ptr_variant_null(ptr_variant))
        ptr_variant = _parse_ast2_atomic_game();

    if (ptr_variant_null(ptr_variant))
    {
        const ast2_token* tok1 = _get_token(_idx);

        if (!tok1)
            return nullptr;

        if (tok1->type == TOKEN_TYPE_PLUS)
            unary_sign = SIGN_POSITIVE;
        else if (tok1->type == TOKEN_TYPE_MINUS)
            unary_sign = SIGN_NEGATIVE;
        else
            return nullptr;

        _mark_consumed(_idx, AST2_RULE_QUALIFIED_GAME);
        _idx++;

        ptr_variant = _parse_ast2_atomic_game();
    }

    if (ptr_variant_null(ptr_variant))
    {
        _idx = idx_start;
        return nullptr;
    }

    return std::visit([&](auto& ptr) -> unique_ptr<ast2_qualified_game>
    {
        THROW_ASSERT(ptr);
        using element_t = std::decay_t<decltype(*ptr)>;

        if constexpr (std::is_same_v<element_t, ast2_plusminus_game>)
            return make_unique<ast2_qualified_game>(std::move(ptr));
        else
            return make_unique<ast2_qualified_game>(std::move(ptr), unary_sign);

    }, ptr_variant);
}

// Parse CGT games
unique_ptr<ast2_sum> ast2_parser::_parse_ast2_sum()
{
    print_rule pr("Sum");

    const size_t idx_start = _idx;
    vector<pair<sign_enum, ast2_sum::variant_t>> summands;

    // Empty?
    if (!_get_token(_idx))
        return make_unique<ast2_sum>(std::move(summands));

    // 1st summand
    ast2_sum::variant_t summand_variant = _parse_ast2_qualified_game();

    if (ptr_variant_null(summand_variant))
        return nullptr;

    summands.emplace_back(SIGN_POSITIVE, std::move(summand_variant));

    // Tail
    while (1)
    {
        const size_t idx_checkpoint = _idx;

        // Try plusminus game
        summand_variant = _parse_ast2_plusminus_game();

        if (ptr_variant_non_null(summand_variant))
        {
            summands.emplace_back(SIGN_POSITIVE, std::move(summand_variant));
            continue;
        }

        // Get binary op
        sign_enum binary_sign;

        const ast2_token* tok1 = _get_token(_idx);
        if (tok1 && tok1->type == TOKEN_TYPE_PLUS)
            binary_sign = SIGN_POSITIVE;
        else if (tok1 && tok1->type == TOKEN_TYPE_MINUS)
            binary_sign = SIGN_NEGATIVE;
        else
            break;

        _mark_consumed(_idx, AST2_RULE_SUM);
        _idx++;

        summand_variant = _parse_ast2_qualified_game();

        if (ptr_variant_non_null(summand_variant))
            summands.emplace_back(binary_sign, std::move(summand_variant));
        else
        {
            _idx = idx_checkpoint;
            break;
        }
    }

    assert(ptr_variant_null(summand_variant));
    return make_unique<ast2_sum>(std::move(summands));
}

unique_ptr<ast2_bracket_sum> ast2_parser::_parse_ast2_bracket_sum()
{
    print_rule pr("Bracket sum");

    const size_t idx_start = _idx;

    // '('
    const ast2_token* tok1 = _get_token(_idx);
    if (!(tok1 && tok1->type == TOKEN_TYPE_LBRACK))
        return nullptr;
    _mark_consumed(_idx, AST2_RULE_BRACKET_SUM);
    _idx++;

    // Sum
    unique_ptr<ast2_sum> s = _parse_ast2_sum();
    if (!s)
    {
        _idx = idx_start;
        return nullptr;
    }

    // ')'
    const ast2_token* tok2 = _get_token(_idx);
    if (!(tok2 && tok2->type == TOKEN_TYPE_RBRACK))
    {
        _idx = idx_start;
        return nullptr;
    }
    _mark_consumed(_idx, AST2_RULE_BRACKET_SUM);
    _idx++;

    return make_unique<ast2_bracket_sum>(std::move(s));
}

unique_ptr<ast2_option_list> ast2_parser::_parse_ast2_option_list()
{
    print_rule pr("Option list");

    const size_t idx_start = _idx;
    vector<unique_ptr<ast2_sum>> option_nodes;

    // Empty alternative
    if (!_get_token(_idx))
        return make_unique<ast2_option_list>(std::move(option_nodes));

    // Initial option
    unique_ptr<ast2_sum> option = _parse_ast2_sum();
    if (!option)
        return nullptr;

    option_nodes.emplace_back(std::move(option));

    while (1)
    {
        const size_t idx_checkpoint = _idx;
        const ast2_token* tok1 = _get_token(_idx);

        if (!(tok1 && tok1->type == TOKEN_TYPE_COMMA))
            break;

        _mark_consumed(_idx, AST2_RULE_OPTION_LIST);
        _idx++;

        option = _parse_ast2_sum();
        if (option)
        {
            option_nodes.emplace_back(std::move(option));
            continue;
        }

        _idx = idx_checkpoint;
        break;
    }

    assert(!option);
    return make_unique<ast2_option_list>(std::move(option_nodes));
}

unique_ptr<ast2_braced_option_list> ast2_parser::_parse_ast2_braced_option_list()
{
    print_rule pr("Braced option list");

    const size_t idx_start = _idx;

    // '{'
    const ast2_token* tok1 = _get_token(_idx);
    if (!(tok1 && tok1->type == TOKEN_TYPE_LBRACE))
        return nullptr;
    _mark_consumed(_idx, AST2_RULE_BRACED_OPTION_LIST);
    _idx++;

    // Options
    unique_ptr<ast2_option_list> option_list = _parse_ast2_option_list();
    if (!option_list)
    {
        _idx = idx_start;
        return nullptr;
    }

    // '}'
    const ast2_token* tok2 = _get_token(_idx);
    if (!(tok2 && tok2->type == TOKEN_TYPE_RBRACE))
    {
        _idx = idx_start;
        return nullptr;
    }

    _mark_consumed(_idx, AST2_RULE_BRACED_OPTION_LIST);
    _idx++;

    return make_unique<ast2_braced_option_list>(std::move(option_list));
}

unique_ptr<ast2_unbraced_cgt_game> ast2_parser::_parse_ast2_unbraced_cgt_game()
{
    print_rule pr("Unbraced CGT game");

    const size_t idx_start = _idx;

    // Find largest bar in our current scope
    optional<size_t> max_bar_idx;
    int64_t max_bar_size = -1;
    bool max_bar_unique = true;

    size_t brace_stack_size = 0;

    auto update_max_bar = [&](const ast2_token* tok_bar, size_t i) -> void
    {
        assert(tok_bar &&                         //
               tok_bar->type == TOKEN_TYPE_BAR && //
               _get_token(i) == tok_bar &&        //
               brace_stack_size == 0              //
        );

        const int64_t tok_bar_size = tok_bar->num;
        assert(tok_bar_size > 0);

        if (tok_bar_size == max_bar_size)
            max_bar_unique = false;

        if (tok_bar_size > max_bar_size)
        {
            max_bar_idx = i;
            max_bar_size = tok_bar_size;
            max_bar_unique = true;
        }
    };

    for (size_t i = _idx; ; i++)
    {
        const ast2_token* tok = _get_token(i);
        if (tok == nullptr)
            break;

        if (tok->type == TOKEN_TYPE_LBRACE)
            brace_stack_size++;
        if (tok->type == TOKEN_TYPE_RBRACE)
        {
            // No underflow
            if (brace_stack_size == 0)
                return nullptr;

            brace_stack_size--;
        }

        if (brace_stack_size == 0 && tok->type == TOKEN_TYPE_BAR)
            update_max_bar(tok, i);
    }

    if (!(max_bar_idx.has_value() && max_bar_unique))
        return nullptr;

    // Compute left and right scopes
    const ast2_token_scope current_scope = _get_active_scope();

    size_t idx_left = _idx;
    const size_t idx_left_end = *max_bar_idx;
    const ast2_token_scope left_scope = current_scope.cut(idx_left, idx_left_end);

    size_t idx_right = *max_bar_idx + 1;
    const size_t idx_right_end = current_scope.end;
    const ast2_token_scope right_scope = current_scope.cut(idx_right, idx_right_end);

    using variant_t = ast2_unbraced_cgt_game::variant_t;

    // Get left set
    _push_scope(left_scope);

    variant_t left_set = _parse_ast2_unbraced_cgt_game();
    if (ptr_variant_null(left_set))
        left_set = _parse_ast2_option_list();

    const bool left_ok =
        ptr_variant_non_null(left_set) && _get_token(_idx) == nullptr;

    _pop_scope();

    if (!left_ok)
    {
        _idx = idx_start;
        return nullptr;
    }


    // Consume bar
    assert(_idx == *max_bar_idx);
    _mark_consumed(_idx, AST2_RULE_UNBRACED_CGT_GAME);
    _idx++;

    // Get right set
    _push_scope(right_scope);

    variant_t right_set = _parse_ast2_unbraced_cgt_game();
    if (ptr_variant_null(right_set))
        right_set = _parse_ast2_option_list();

    const bool right_ok =
        ptr_variant_non_null(right_set) && (_get_token(_idx) == nullptr);
    _pop_scope();

    if (!right_ok)
    {
        _idx = idx_start;
        return nullptr;
    }

    return make_unique<ast2_unbraced_cgt_game>(std::move(left_set),
                                               std::move(right_set));
}

unique_ptr<ast2_braced_cgt_game> ast2_parser::_parse_ast2_braced_cgt_game()
{
    print_rule pr("Braced CGT game");

    const size_t idx_start = _idx;

    // Opening brace
    const ast2_token* tok1 = _get_token(_idx);
    if (!(tok1 && tok1->type == TOKEN_TYPE_LBRACE))
        return nullptr;

    _mark_consumed(_idx, AST2_RULE_BRACED_CGT_GAME);
    _idx++;

    // Find closing brace that defines the end of our scope
    size_t brace_stack_size = 1;
    size_t idx_closing_brace;

    for (size_t i = _idx; ; i++)
    {
        const ast2_token* tok2 = _get_token(i);
        if (!tok2)
        {
            _idx = idx_start;
            return nullptr;
        }

        if (tok2->type == TOKEN_TYPE_LBRACE)
            brace_stack_size++;
        if (tok2->type == TOKEN_TYPE_RBRACE)
            brace_stack_size--;

        if (brace_stack_size == 0)
        {
            idx_closing_brace = i;
            break;
        }
    }

    // Don't mark closing brace consumed yet!

    const ast2_token_scope current_scope = _get_active_scope();
    const ast2_token_scope new_scope = current_scope.cut(_idx, idx_closing_brace);

    _push_scope(new_scope);
    unique_ptr<ast2_unbraced_cgt_game> unbraced = _parse_ast2_unbraced_cgt_game();

    // Consumed all?
    assert(LOGICAL_IMPLIES(unbraced, _get_token(_idx) == nullptr));
    _pop_scope();

    if (!unbraced)
    {
        _idx = idx_start;
        return nullptr;
    }

    assert(_idx == idx_closing_brace);
    _mark_consumed(idx_closing_brace, AST2_RULE_BRACED_CGT_GAME);
    _idx++;

    return make_unique<ast2_braced_cgt_game>(std::move(unbraced));
}

////////////////////////////////////////////////// Parsing functions
//shared_ptr<ast2_integer> ast2_parser::_parse_ast_integer(bool at_least_0)
//{
//    const ast2_token* tok1 = _get_token(_idx);
//    if (!tok1)
//        return nullptr;
//
//    int64_t abs_value;
//    bool is_negative = false;
//
//    if (tok1->type == TOKEN_TYPE_MINUS)
//    {
//        if (at_least_0)
//            return nullptr;
//
//        _mark_consumed(_idx, AST2_RULE_INTEGER);
//        is_negative = true;
//
//        const ast2_token* tok2 = _get_token(_idx + 1);
//
//        if (!tok2 || tok2->type != TOKEN_TYPE_INT)
//            return nullptr;
//
//        _mark_consumed(_idx + 1, AST2_RULE_INTEGER);
//        abs_value = tok1->num;
//    }
//    else
//    {
//        if (tok1->type != TOKEN_TYPE_INT)
//            return nullptr;
//
//        _mark_consumed(_idx, AST2_RULE_INTEGER);
//        abs_value = tok1->num;
//    }
//
//    THROW_ASSERT(abs_value >= 0 && negate_is_safe(abs_value));
//
//    if (is_negative)
//        abs_value = -abs_value;
//
//    _idx += (1 + is_negative);
//    return make_shared<ast2_integer>(abs_value);
//}
//
//shared_ptr<ast2_rational> ast2_parser::_parse_ast_rational()
//{
//    shared_ptr<ast2_integer> top = _parse_ast_integer(false);
//    if (!top)
//        return nullptr;
//
//    const size_t idx_after_top = _idx;
//
//    const int64_t top_value = top->value;
//    int64_t bottom_value = 1;
//
//    const ast2_token* tok_slash = _get_token(_idx);
//    if (tok_slash && tok_slash->type == TOKEN_TYPE_SLASH)
//    {
//        _mark_consumed(_idx, AST2_RULE_RATIONAL);
//        _idx++;
//        shared_ptr<ast2_integer> bottom = _parse_ast_integer(false);
//
//        if (bottom)
//            bottom_value = bottom->value;
//        else
//            _idx = idx_after_top;
//    }
//
//    return make_shared<ast2_rational>(top_value, bottom_value);
//}
//
//shared_ptr<ast2_up> ast2_parser::_parse_ast_up()
//{
//    const ast2_token* tok1 = _get_token(_idx);
//    if (!tok1 || tok1->type != TOKEN_TYPE_UP_DOWN)
//        return nullptr;
//    _mark_consumed(_idx, AST2_RULE_UP);
//    _idx++;
//
//    const int64_t arrow_value = tok1->num;
//    assert(arrow_value != 0);
//
//    if (abs(arrow_value) > 1)
//        return make_shared<ast2_up>(arrow_value);
//
//    shared_ptr<ast2_integer> int_suffix = _parse_ast_integer(true);
//    if (int_suffix)
//    {
//        int64_t final_value = int_suffix->value;
//        if (arrow_value == -1)
//            final_value = -final_value;
//
//        return make_shared<ast2_up>(final_value);
//    }
//
//    return make_shared<ast2_up>(arrow_value);
//}
//
//shared_ptr<ast2_nimber> ast2_parser::_parse_ast_nimber()
//{
//    const ast2_token* tok1 = _get_token(_idx);
//    if (!tok1 || tok1->type != TOKEN_TYPE_STAR)
//        return nullptr;
//    _mark_consumed(_idx, AST2_RULE_NIMBER);
//    _idx++;
//
//    shared_ptr<ast2_integer> int_suffix = _parse_ast_integer(true);
//    if (int_suffix)
//    {
//        const int64_t nim_value = int_suffix->value;
//        THROW_ASSERT(nim_value >= 0);
//
//        return make_shared<ast2_nimber>(nim_value);
//    }
//
//    return make_shared<ast2_nimber>(1);
//}
//
//shared_ptr<ast2_rational_up_nimber> ast2_parser::_parse_ast_rational_up_nimber()
//{
//    shared_ptr<ast2_rational> rational = _parse_ast_rational();
//    shared_ptr<ast2_up> up = _parse_ast_up();
//    shared_ptr<ast2_nimber> nimber = _parse_ast_nimber();
//
//    if (!(rational || up || nimber))
//        return nullptr;
//
//    return make_shared<ast2_rational_up_nimber>(rational, up, nimber);
//}
//
//shared_ptr<ast2_game> ast2_parser::_parse_ast_game()
//{
//    const size_t idx_start = _idx;
//
//    // MINUS? (bracket_sum|braced_cgt_game|explicit_game)
//    const ast2_token* tok1 = _get_token(_idx);
//    if (!tok1)
//        return nullptr;
//
//    const bool unary_minus = tok1->type == TOKEN_TYPE_MINUS;
//    bool actually_minus = unary_minus;
//
//    if (unary_minus)
//    {
//        _mark_consumed(_idx, AST2_RULE_GAME);
//        _idx++;
//    }
//
//    shared_ptr<i_ast2_atomic_game> atom;
//
//    if (!atom)
//        atom = _parse_ast_bracket_sum();
//    if (!atom)
//        atom = _parse_ast_braced_cgt_game();
//    if (!atom)
//        atom = _parse_ast_explicit_game();
//
//    // rational_up_nimber
//    if (!atom)
//    {
//        // Rewind unary minus
//        _idx = idx_start;
//        actually_minus = false;
//        atom = _parse_ast_rational_up_nimber();
//    }
//
//
//    // MINUS? rational_up_nimber
//    if (!atom && unary_minus)
//    {
//        _idx = idx_start + 1;
//        actually_minus = true;
//        atom = _parse_ast_rational_up_nimber();
//    }
//
//    const sign_enum sign_type = actually_minus ? SIGN_NEGATIVE : SIGN_POSITIVE;
//    return make_shared<ast2_game>(sign_type, atom);
//}
//
//shared_ptr<ast2_sum> ast2_parser::_parse_ast_sum()
//{
//    const size_t idx_start = _idx;
//
//    static vector<size_t> cycle_stack;
//
//    for (const size_t i : cycle_stack)
//        if (_idx == i)
//            return nullptr;
//
//    cycle_stack.push_back(_idx);
//
//    vector<pair<sign_enum, shared_ptr<ast2_game>>> operands;
//
//    while (1)
//    {
//        const size_t idx_checkpoint = _idx;
//        optional<sign_enum> sign_type;
//
//        if (operands.empty())
//            sign_type = SIGN_POSITIVE;
//        else
//        {
//            const ast2_token* tok1 = _get_token(_idx);
//
//            if (tok1 && tok1->type == TOKEN_TYPE_PLUS)
//                sign_type = SIGN_POSITIVE;
//            else if (tok1 && tok1->type == TOKEN_TYPE_MINUS)
//                sign_type = SIGN_NEGATIVE;
//
//            if (sign_type)
//            {
//                _mark_consumed(_idx, AST2_RULE_SUM);
//                _idx++;
//            }
//        }
//
//        if (sign_type)
//        {
//            shared_ptr<ast2_game> game_operand = _parse_ast_game();
//            if (game_operand)
//            {
//                operands.emplace_back(*sign_type, game_operand);
//                continue;
//            }
//        }
//
//        _idx = idx_checkpoint;
//        break;
//    }
//
//    cycle_stack.pop_back();
//
//    if (_idx == idx_start && _get_token(_idx) != nullptr)
//        return nullptr;
//
//    return make_shared<ast2_sum>(operands);
//}
//
//shared_ptr<ast2_bracket_sum> ast2_parser::_parse_ast_bracket_sum()
//{
//    const size_t idx_start = _idx;
//
//    // '('
//    const ast2_token* tok1 = _get_token(_idx);
//    if (!(tok1 && tok1->type == TOKEN_TYPE_LBRACK))
//        return nullptr;
//    _mark_consumed(_idx, AST2_RULE_BRACKET_SUM);
//    _idx++;
//
//    // Sum
//    shared_ptr<ast2_sum> s = _parse_ast_sum();
//    if (!s)
//    {
//        _idx = idx_start;
//        return nullptr;
//    }
//
//    // ')'
//    const ast2_token* tok2 = _get_token(_idx);
//    if (!(tok2 && tok2->type == TOKEN_TYPE_RBRACK))
//    {
//        _idx = idx_start;
//        return nullptr;
//    }
//    _mark_consumed(_idx, AST2_RULE_BRACKET_SUM);
//    _idx++;
//
//    // OK
//    return make_shared<ast2_bracket_sum>(s);
//}
//
//shared_ptr<ast2_game_list> parse_ast_game_list(const ast2_token_scope& tokens,
//                                               size_t& idx)
//{
//    vector<shared_ptr<ast2_sum>> games;
//
//    bool first = true;
//
//    while (1)
//    {
//        const size_t idx_checkpoint = idx;
//
//        // Require comma?
//        if (!first)
//        {
//            const ast2_token* tok1 = get_nth_token(tokens, idx);
//            if (!(tok1 && tok1->type == TOKEN_TYPE_COMMA))
//                break;
//            idx++;
//        }
//
//        shared_ptr<ast2_sum> g = parse_ast_sum(tokens, idx);
//
//        if (!g)
//        {
//            idx = idx_checkpoint;
//            break;
//        }
//        first = false;
//
//        if (g->operands.empty())
//            g.reset();
//        else
//            games.emplace_back(g);
//    }
//
//    return make_shared<ast2_game_list>(games);
//}
//
//shared_ptr<ast2_braced_cgt_game> parse_ast_braced_cgt_game(
//    const ast2_token_scope& tokens, size_t& idx)
//{
//    const size_t idx_start = idx;
//
//    // Opening brace
//    const ast2_token* tok1 = get_nth_token(tokens, idx);
//    if (!(tok1 && tok1->type == TOKEN_TYPE_LBRACE))
//        return nullptr;
//
//    // Find closing brace that defines the end of our scope
//    size_t brace_stack_size = 1;
//    size_t idx_closing_brace;
//
//    for (size_t i = idx + 1; ; i++)
//    {
//        const ast2_token* tok2 = get_nth_token(tokens, i);
//        if (!tok2)
//            return nullptr;
//
//        if (tok2->type == TOKEN_TYPE_LBRACE)
//            brace_stack_size++;
//        if (tok2->type == TOKEN_TYPE_RBRACE)
//            brace_stack_size--;
//
//        if (brace_stack_size == 0)
//        {
//            idx_closing_brace = i;
//            break;
//        }
//    }
//
//    const ast2_token_scope new_scope = tokens.cut(idx + 1, idx_closing_brace);
//    idx++;
//
//    shared_ptr<ast2_unbraced_cgt_game> unbraced =
//        parse_unbraced_cgt_game(new_scope, idx);
//
//    const bool consumed_all = !new_scope.idx_inside_scope(idx);
//    idx++;
//
//    if (unbraced && consumed_all)
//        return make_shared<ast2_braced_cgt_game>(unbraced);
//
//    idx = idx_start;
//    return nullptr;
//}
//
//
//shared_ptr<ast2_unbraced_cgt_game> parse_unbraced_cgt_game(
//    const ast2_token_scope& tokens, size_t& idx)
//{
//    const size_t idx_start = idx;
//
//    // Find largest bar in our current scope
//    optional<size_t> max_bar_idx;
//    int64_t max_bar_size = -1;
//    bool max_bar_unique = true;
//
//    int brace_stack_size = 0;
//
//    auto update_max_bar = [&](const ast2_token* tok_bar, size_t i) -> void
//    {
//        assert(tok_bar &&                             //
//               tok_bar->type == TOKEN_TYPE_BAR &&     //
//               get_nth_token(tokens, i) == tok_bar && //
//               brace_stack_size == 0                  //
//        );
//
//        const int64_t tok_bar_size = tok_bar->num;
//        assert(tok_bar_size > 0);
//
//        if (tok_bar_size == max_bar_size)
//            max_bar_unique = false;
//
//        if (tok_bar_size > max_bar_size)
//        {
//            max_bar_idx = i;
//            max_bar_size = tok_bar_size;
//            max_bar_unique = true;
//        }
//    };
//
//    for (size_t i = idx; ; i++)
//    {
//        const ast2_token* tok = get_nth_token(tokens, i);
//        if (tok == nullptr)
//            break;
//
//        if (tok->type == TOKEN_TYPE_LBRACE)
//            brace_stack_size++;
//        if (tok->type == TOKEN_TYPE_RBRACE)
//            brace_stack_size--;
//
//        if (brace_stack_size < 0)
//            return nullptr;
//
//        if (brace_stack_size == 0 && tok->type == TOKEN_TYPE_BAR)
//            update_max_bar(tok, i);
//    }
//
//    if (!(max_bar_idx.has_value() && max_bar_unique))
//        return nullptr;
//
//    size_t idx_left = idx;
//    const size_t idx_left_end = *max_bar_idx;
//    const ast2_token_scope scope_left = tokens.cut(idx_left, idx_left_end);
//
//    size_t idx_right = *max_bar_idx + 1;
//    const size_t idx_right_end = tokens.end;
//    const ast2_token_scope scope_right = tokens.cut(idx_right, idx_right_end);
//
//    shared_ptr<i_ast2_option_set> left_set =
//        parse_option_set(scope_left, idx_left);
//
//    if (!(left_set && scope_left.idx_outside_scope(idx_left)))
//        return nullptr;
//
//    shared_ptr<i_ast2_option_set> right_set =
//        parse_option_set(scope_right, idx_right);
//
//    if (!(right_set && scope_right.idx_outside_scope(idx_right)))
//        return nullptr;
//
//    idx = idx_right;
//    return make_shared<ast2_unbraced_cgt_game>(left_set, right_set);
//}
//
//
//shared_ptr<i_ast2_option_set> parse_option_set(const ast2_token_scope& tokens,
//                                               size_t& idx)
//{
//    shared_ptr<ast2_unbraced_cgt_game> unbraced =
//        parse_unbraced_cgt_game(tokens, idx);
//
//    if (unbraced)
//        return unbraced;
//
//    shared_ptr<ast2_game_list> games = parse_ast_game_list(tokens, idx);
//
//    if (games)
//        return games;
//
//    return nullptr;
//}
//
//shared_ptr<ast2_explicit_game> parse_ast_explicit_game(
//    const ast2_token_scope& tokens, size_t& idx)
//{
//    const ast2_token* tok1 = get_nth_token(tokens, idx);
//    const ast2_token* tok2 = get_nth_token(tokens, idx + 1);
//    const ast2_token* tok3 = get_nth_token(tokens, idx + 2);
//
//    if (!(tok1 && tok1->type == TOKEN_TYPE_IDENT &&          //
//          tok2 && tok2->type == TOKEN_TYPE_COLON &&          //
//          tok3 && tok3->type == TOKEN_TYPE_EXP_GAME_CONTENTS //
//          ))
//        return nullptr;
//
//    const string& game_title = tok1->str;
//    const string& game_contents = tok3->str;
//
//    idx += 3;
//    return make_shared<ast2_explicit_game>(game_title, game_contents);
//}

} // namespace

////////////////////////////////////////////////// Exported functions
cgt_environment::cgt_environment(shared_ptr<const ast2_sum> sum_node)
    : _sum_node(sum_node)
{
    assert(_sum_node);
}

game* cgt_environment::make_game() const
{
    THROW_ASSERT(_sum_node);
    return _sum_node->make_game(false);
}

void test_cgt_environment(const string& env_string, size_t line_start, size_t column_start)
{
    cout << "Input string (";
    cout << "L" << line_start << " C" << column_start;
    cout << "):" << endl;

    cout << "`" << env_string << "`" << endl;

    cout << endl;

    ast2_lexer lexer;
    const vector<ast2_token> tokens =
        lexer.get_tokens(&env_string, line_start, column_start);

    cout << "Token stream (after lexer rules):" << endl;
    cout << "[" << endl;
    for (const ast2_token& tok : tokens)
        cout << "\t" << tok << endl;
    cout << "]" << endl;

    cout << endl;

    //const ast2_token_scope ts(0, tokens.size());
    //size_t idx = 0;
    ast2_parser parser;
    unique_ptr<ast2_sum> s = parser.parse(&tokens);

    cout << "Parse tree:" << endl;
    assert(s);
    s->print(cout, 0);
    cout << endl;

    static unsigned int graph_number = 0;

    const string file_name = "graphs/" + to_string(graph_number++) + ".dot";
    generic_graph_printer graph;
    s->print_graph(graph);
    graph.print_to_file(file_name, env_string);

    //THROW_ASSERT(idx == tokens.size(), "Parsing incomplete!");
}

cgt_environment parse_cgt_environment(const string& env_string, size_t line_start, size_t column_start)
{
    ast2_lexer lexer;
    const vector<ast2_token> tokens =
        lexer.get_tokens(&env_string, line_start, column_start);

    ast2_parser parser;
    unique_ptr<ast2_sum> sum = parser.parse(&tokens);

    return cgt_environment(shared_ptr<const ast2_sum>(sum.release()));
}

