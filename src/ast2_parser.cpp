#include "ast2_parser.h"

#include <sstream>
#include <string>
#include <cassert>
#include <memory>
#include <vector>
#include <cstddef>
#include <utility>

#include "ast2_node.h"
#include "ast2_token.h"
#include "safe_arithmetic.h"
#include "throw_assert.h"
#include "variant_utils.h"


using namespace std;

////////////////////////////////////////////////// enum ast2_rule_enum
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

        case AST2_RULE_BRACKET_SUM:
            return "bracket_sum";
        case AST2_RULE_SUM:
            return "sum";
        case AST2_RULE_BRACED_OPTION_LIST:
            return "braced_option_list";
        case AST2_RULE_OPTION_LIST:
            return "option_list";
        case AST2_RULE_BRACED_CGT_GAME:
            return "braced_cgt_game";
        case AST2_RULE_UNBRACED_CGT_GAME:
            return "unbraced_cgt_game";
    }

    THROW_ASSERT(false);
}

////////////////////////////////////////////////// ast2_parser methods
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

unique_ptr<ast2_integer> ast2_parser::_parse_ast2_integer(bool at_least_0)
{

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

unique_ptr<ast2_atomic_game> ast2_parser::_parse_ast2_atomic_game()
{

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

unique_ptr<ast2_bracket_sum> ast2_parser::_parse_ast2_bracket_sum()
{

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

unique_ptr<ast2_sum> ast2_parser::_parse_ast2_sum()
{
    //const size_t idx_start = _idx;
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

unique_ptr<ast2_braced_option_list> ast2_parser::_parse_ast2_braced_option_list()
{
    const size_t idx_start = _idx;

    // Opening brace
    const ast2_token* tok1 = _get_token(_idx);
    if (!(tok1 && tok1->type == TOKEN_TYPE_LBRACE))
        return nullptr;

    _mark_consumed(_idx, AST2_RULE_BRACED_OPTION_LIST);
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
    unique_ptr<ast2_option_list> unbraced = _parse_ast2_option_list();

    // Consumed all?
    assert(LOGICAL_IMPLIES(unbraced, _get_token(_idx) == nullptr));
    _pop_scope();

    if (!unbraced)
    {
        _idx = idx_start;
        return nullptr;
    }

    assert(_idx == idx_closing_brace);
    _mark_consumed(idx_closing_brace, AST2_RULE_BRACED_OPTION_LIST);
    _idx++;

    return make_unique<ast2_braced_option_list>(std::move(unbraced));
}

unique_ptr<ast2_option_list> ast2_parser::_parse_ast2_option_list()
{
    //const size_t idx_start = _idx;
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

unique_ptr<ast2_braced_cgt_game> ast2_parser::_parse_ast2_braced_cgt_game()
{
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

unique_ptr<ast2_unbraced_cgt_game> ast2_parser::_parse_ast2_unbraced_cgt_game()
{

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


