#include "cgt_environment.h"
#include "integral_conversion.h"
#include "n_bit_int.h"
#include "safe_arithmetic.h"
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

namespace {
////////////////////////////////////////////////// Declarations
shared_ptr<ast2_bracket_sum> parse_ast_bracket_sum(
    const ast2_token_scope& tokens, size_t& idx);

shared_ptr<i_ast2_option_set> parse_option_set(const ast2_token_scope& tokens,
                                               size_t& idx);

shared_ptr<ast2_unbraced_cgt_game> parse_unbraced_cgt_game(
    const ast2_token_scope& tokens, size_t& idx);

shared_ptr<ast2_braced_cgt_game> parse_ast_braced_cgt_game(
    const ast2_token_scope& tokens, size_t& idx);

shared_ptr<ast2_explicit_game> parse_ast_explicit_game(
    const ast2_token_scope& tokens, size_t& idx);


shared_ptr<ast2_sum> parse_ast_sum(const ast2_token_scope& tokens, size_t& idx);

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

        CALL_GET_TOKEN_FN(_get_token_exp_game_contents);
        CALL_GET_TOKEN_FN(_get_token_simple_char);
        CALL_GET_TOKEN_FN(_get_token_int);
        CALL_GET_TOKEN_FN(_get_token_up_down);
        CALL_GET_TOKEN_FN(_get_token_bar);
        CALL_GET_TOKEN_FN(_get_token_identifier);

        THROW_ASSERT(false,
                     "Lexer error in CGT environment: unmatched text at line " +
                         to_string(_line_no) + " col " + to_string(_column_no));
    }

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
const ast2_token* get_nth_token(const ast2_token_scope& tscope, size_t idx)
{
    if (!(idx >= tscope.start && idx < tscope.end))
        return nullptr;
    return &(*tscope.tokens)[idx];
}

shared_ptr<ast2_integer> parse_ast_integer(const ast2_token_scope& tokens,
                                           size_t& idx, bool at_least_0)
{
    const ast2_token* tok1 = get_nth_token(tokens, idx);
    if (!tok1)
        return nullptr;

    int64_t abs_value;
    bool is_negative = false;

    if (tok1->type == TOKEN_TYPE_MINUS)
    {
        if (at_least_0)
            return nullptr;

        is_negative = true;
        const ast2_token* tok2 = get_nth_token(tokens, idx + 1);

        if (!tok2 || tok2->type != TOKEN_TYPE_INT)
            return nullptr;

        abs_value = tok2->num;
    }
    else
    {
        if (tok1->type != TOKEN_TYPE_INT)
            return nullptr;
        abs_value = tok1->num;
    }

    THROW_ASSERT(abs_value >= 0 && negate_is_safe(abs_value));

    if (is_negative)
        abs_value = -abs_value;

    idx += (1 + is_negative);
    return make_shared<ast2_integer>(abs_value);
}

shared_ptr<ast2_rational> parse_ast_rational(const ast2_token_scope& tokens,
                                             size_t& idx)
{
    shared_ptr<ast2_integer> top = parse_ast_integer(tokens, idx, false);
    if (!top)
        return nullptr;

    const size_t idx_after_top = idx;

    const int64_t top_value = top->value;
    int64_t bottom_value = 1;

    const ast2_token* tok_slash = get_nth_token(tokens, idx);
    if (tok_slash && tok_slash->type == TOKEN_TYPE_SLASH)
    {
        idx++;
        shared_ptr<ast2_integer> bottom = parse_ast_integer(tokens, idx, false);

        if (bottom)
            bottom_value = bottom->value;
        else
            idx = idx_after_top;
    }

    return make_shared<ast2_rational>(top_value, bottom_value);
}

shared_ptr<ast2_up> parse_ast_up(const ast2_token_scope& tokens, size_t& idx)
{
    const ast2_token* tok1 = get_nth_token(tokens, idx);
    if (!tok1 || tok1->type != TOKEN_TYPE_UP_DOWN)
        return nullptr;
    idx++;

    const int64_t arrow_value = tok1->num;
    assert(arrow_value != 0);

    if (abs(arrow_value) > 1)
        return make_shared<ast2_up>(arrow_value);

    shared_ptr<ast2_integer> int_suffix = parse_ast_integer(tokens, idx, true);
    if (int_suffix)
    {
        int64_t final_value = int_suffix->value;
        if (arrow_value == -1)
            final_value = -final_value;

        return make_shared<ast2_up>(final_value);
    }

    return make_shared<ast2_up>(arrow_value);
}

shared_ptr<ast2_nimber> parse_ast_nimber(const ast2_token_scope& tokens,
                                         size_t& idx)
{
    const ast2_token* tok1 = get_nth_token(tokens, idx);
    if (!tok1 || tok1->type != TOKEN_TYPE_STAR)
        return nullptr;
    idx++;

    shared_ptr<ast2_integer> int_suffix = parse_ast_integer(tokens, idx, true);
    if (int_suffix)
    {
        const int64_t nim_value = int_suffix->value;
        THROW_ASSERT(nim_value >= 0);
        return make_shared<ast2_nimber>(nim_value);
    }

    return make_shared<ast2_nimber>(1);
}

shared_ptr<ast2_rational_up_nimber> parse_ast_rational_up_nimber(
    const ast2_token_scope& tokens, size_t& idx)
{
    shared_ptr<ast2_rational> rational = parse_ast_rational(tokens, idx);
    shared_ptr<ast2_up> up = parse_ast_up(tokens, idx);
    shared_ptr<ast2_nimber> nimber = parse_ast_nimber(tokens, idx);

    if (!(rational || up || nimber))
        return nullptr;

    return make_shared<ast2_rational_up_nimber>(rational, up, nimber);
}

shared_ptr<ast2_game> parse_ast_game(const ast2_token_scope& tokens, size_t& idx)
{
    const size_t idx_start = idx;

    // MINUS? (bracket_sum|braced_cgt_game|explicit_game)
    const ast2_token* tok1 = get_nth_token(tokens, idx);
    if (!tok1)
        return nullptr;

    const bool unary_minus = tok1->type == TOKEN_TYPE_MINUS;
    bool actually_minus = unary_minus;

    if (unary_minus)
        idx++;

    shared_ptr<i_ast2_atomic_game> atom;

    if (!atom)
        atom = parse_ast_bracket_sum(tokens, idx);
    if (!atom)
        atom = parse_ast_braced_cgt_game(tokens, idx);
    if (!atom)
        atom = parse_ast_explicit_game(tokens, idx);

    // rational_up_nimber
    if (!atom)
    {
        // Rewind unary minus
        idx = idx_start;
        actually_minus = false;
        atom = parse_ast_rational_up_nimber(tokens, idx);
    }


    // MINUS? rational_up_nimber
    if (!atom && unary_minus)
    {
        idx = idx_start + 1;
        actually_minus = true;
        atom = parse_ast_rational_up_nimber(tokens, idx);
    }

    const sign_enum sign_type = actually_minus ? SIGN_NEGATIVE : SIGN_POSITIVE;
    return make_shared<ast2_game>(sign_type, atom);
}

shared_ptr<ast2_sum> parse_ast_sum(const ast2_token_scope& tokens, size_t& idx)
{
    const size_t idx_start = idx;

    static vector<size_t> cycle_stack;

    for (const size_t i : cycle_stack)
        if (idx == i)
            return nullptr;

    cycle_stack.push_back(idx);

    vector<pair<sign_enum, shared_ptr<ast2_game>>> operands;

    while (1)
    {
        const size_t idx_checkpoint = idx;
        optional<sign_enum> sign_type;

        if (operands.empty())
            sign_type = SIGN_POSITIVE;
        else
        {
            const ast2_token* tok1 = get_nth_token(tokens, idx);
            idx++;

            if (tok1 && tok1->type == TOKEN_TYPE_PLUS)
                sign_type = SIGN_POSITIVE;
            else if (tok1 && tok1->type==TOKEN_TYPE_MINUS)
                sign_type = SIGN_NEGATIVE;
        }

        if (sign_type)
        {
            shared_ptr<ast2_game> game_operand = parse_ast_game(tokens, idx);
            if (game_operand)
            {
                operands.emplace_back(*sign_type, game_operand);
                continue;
            }
        }

        idx = idx_checkpoint;
        break;
    }

    cycle_stack.pop_back();

    if (idx == idx_start && tokens.idx_inside_scope(idx))
        return nullptr;

    return make_shared<ast2_sum>(operands);
}

shared_ptr<ast2_bracket_sum> parse_ast_bracket_sum(const ast2_token_scope& tokens,
                                                   size_t& idx)
{
    const size_t idx_start = idx;

    // '('
    const ast2_token* tok1 = get_nth_token(tokens, idx);
    if (!(tok1 && tok1->type == TOKEN_TYPE_LBRACK))
        return nullptr;
    idx++;

    // Sum
    shared_ptr<ast2_sum> s = parse_ast_sum(tokens, idx);
    if (!s)
    {
        idx = idx_start;
        return nullptr;
    }

    // ')'
    const ast2_token* tok2 = get_nth_token(tokens, idx);
    if (!(tok2 && tok2->type == TOKEN_TYPE_RBRACK))
    {
        idx = idx_start;
        return nullptr;
    }
    idx++;

    // OK
    return make_shared<ast2_bracket_sum>(s);
}

shared_ptr<ast2_game_list> parse_ast_game_list(const ast2_token_scope& tokens,
                                               size_t& idx)
{
    vector<shared_ptr<ast2_sum>> games;

    bool first = true;

    while (1)
    {
        const size_t idx_checkpoint = idx;

        // Require comma?
        if (!first)
        {
            const ast2_token* tok1 = get_nth_token(tokens, idx);
            if (!(tok1 && tok1->type == TOKEN_TYPE_COMMA))
                break;
            idx++;
        }

        shared_ptr<ast2_sum> g = parse_ast_sum(tokens, idx);

        if (!g)
        {
            idx = idx_checkpoint;
            break;
        }
        first = false;

        if (g->operands.empty())
            g.reset();
        else
            games.emplace_back(g);
    }

    return make_shared<ast2_game_list>(games);
}

shared_ptr<ast2_braced_cgt_game> parse_ast_braced_cgt_game(
    const ast2_token_scope& tokens, size_t& idx)
{
    const size_t idx_start = idx;

    // Opening brace
    const ast2_token* tok1 = get_nth_token(tokens, idx);
    if (!(tok1 && tok1->type == TOKEN_TYPE_LBRACE))
        return nullptr;

    // Find closing brace that defines the end of our scope
    size_t brace_stack_size = 1;
    size_t idx_closing_brace;

    for (size_t i = idx + 1; ; i++)
    {
        const ast2_token* tok2 = get_nth_token(tokens, i);
        if (!tok2)
            return nullptr;

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

    const ast2_token_scope new_scope = tokens.cut(idx + 1, idx_closing_brace);
    idx++;

    shared_ptr<ast2_unbraced_cgt_game> unbraced =
        parse_unbraced_cgt_game(new_scope, idx);

    const bool consumed_all = !new_scope.idx_inside_scope(idx);
    idx++;

    if (unbraced && consumed_all)
        return make_shared<ast2_braced_cgt_game>(unbraced);

    idx = idx_start;
    return nullptr;
}


shared_ptr<ast2_unbraced_cgt_game> parse_unbraced_cgt_game(
    const ast2_token_scope& tokens, size_t& idx)
{
    const size_t idx_start = idx;

    // Find largest bar in our current scope
    optional<size_t> max_bar_idx;
    int64_t max_bar_size = -1;
    bool max_bar_unique = true;

    int brace_stack_size = 0;

    auto update_max_bar = [&](const ast2_token* tok_bar, size_t i) -> void
    {
        assert(tok_bar &&                             //
               tok_bar->type == TOKEN_TYPE_BAR &&     //
               get_nth_token(tokens, i) == tok_bar && //
               brace_stack_size == 0                  //
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

    for (size_t i = idx; ; i++)
    {
        const ast2_token* tok = get_nth_token(tokens, i);
        if (tok == nullptr)
            break;

        if (tok->type == TOKEN_TYPE_LBRACE)
            brace_stack_size++;
        if (tok->type == TOKEN_TYPE_RBRACE)
            brace_stack_size--;

        if (brace_stack_size < 0)
            return nullptr;

        if (brace_stack_size == 0 && tok->type == TOKEN_TYPE_BAR)
            update_max_bar(tok, i);
    }

    if (!(max_bar_idx.has_value() && max_bar_unique))
        return nullptr;

    size_t idx_left = idx;
    const size_t idx_left_end = *max_bar_idx;
    const ast2_token_scope scope_left = tokens.cut(idx_left, idx_left_end);

    size_t idx_right = *max_bar_idx + 1;
    const size_t idx_right_end = tokens.end;
    const ast2_token_scope scope_right = tokens.cut(idx_right, idx_right_end);

    shared_ptr<i_ast2_option_set> left_set =
        parse_option_set(scope_left, idx_left);

    if (!(left_set && scope_left.idx_outside_scope(idx_left)))
        return nullptr;

    shared_ptr<i_ast2_option_set> right_set =
        parse_option_set(scope_right, idx_right);

    if (!(right_set && scope_right.idx_outside_scope(idx_right)))
        return nullptr;

    idx = idx_right;
    return make_shared<ast2_unbraced_cgt_game>(left_set, right_set);
}


shared_ptr<i_ast2_option_set> parse_option_set(const ast2_token_scope& tokens,
                                               size_t& idx)
{
    shared_ptr<ast2_unbraced_cgt_game> unbraced =
        parse_unbraced_cgt_game(tokens, idx);

    if (unbraced)
        return unbraced;

    shared_ptr<ast2_game_list> games = parse_ast_game_list(tokens, idx);

    if (games)
        return games;

    return nullptr;
}

shared_ptr<ast2_explicit_game> parse_ast_explicit_game(
    const ast2_token_scope& tokens, size_t& idx)
{
    const ast2_token* tok1 = get_nth_token(tokens, idx);
    const ast2_token* tok2 = get_nth_token(tokens, idx + 1);
    const ast2_token* tok3 = get_nth_token(tokens, idx + 2);

    if (!(tok1 && tok1->type == TOKEN_TYPE_IDENT &&          //
          tok2 && tok2->type == TOKEN_TYPE_COLON &&          //
          tok3 && tok3->type == TOKEN_TYPE_EXP_GAME_CONTENTS //
          ))
        return nullptr;

    const string& game_title = tok1->str;
    const string& game_contents = tok3->str;

    idx += 3;
    return make_shared<ast2_explicit_game>(game_title, game_contents);
}

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
    return _sum_node->make_game_sum(false);
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

    const ast2_token_scope ts(&tokens);
    size_t idx = 0;
    shared_ptr<ast2_sum> s = parse_ast_sum(ts, idx);

    cout << "Parse tree:" << endl;
    assert(s);
    s->print(cout, 0);
    cout << endl;

    static unsigned int graph_number = 0;

    const string file_name = "graphs/" + to_string(graph_number++) + ".dot";
    generic_graph_printer graph;
    s->print_graph(graph);
    graph.print_to_file(file_name, env_string);

    THROW_ASSERT(idx == tokens.size(), "Parsing incomplete!");
}

cgt_environment parse_cgt_environment(const string& env_string, size_t line_start, size_t column_start)
{
    ast2_lexer lexer;
    const vector<ast2_token> tokens =
        lexer.get_tokens(&env_string, line_start, column_start);

    const ast2_token_scope ts(&tokens);
    size_t idx = 0;
    shared_ptr<ast2_sum> s = parse_ast_sum(ts, idx);

    return cgt_environment(s);
}

