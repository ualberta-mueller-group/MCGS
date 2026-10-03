#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstddef>
#include <utility>

#include "ast2_node.h"
#include "ast2_token.h"

////////////////////////////////////////////////// enum ast2_rule_enum
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

    AST2_RULE_BRACKET_SUM,
    AST2_RULE_SUM,
    AST2_RULE_BRACED_OPTION_LIST,
    AST2_RULE_OPTION_LIST,
    AST2_RULE_BRACED_CGT_GAME,
    AST2_RULE_UNBRACED_CGT_GAME,
};

std::string ast2_rule_enum_to_string(ast2_rule_enum rule_type);

////////////////////////////////////////////////// class ast2_parser
class ast2_parser
{
public:
    std::unique_ptr<ast2_sum> parse(const std::vector<ast2_token>* tokens);

private:
    // Get nth token in current scope. Returns nullptr if idx is outside the scope
    const ast2_token* _get_token(size_t idx) const;

    // Mark nth token consumed. Used for error handling
    void _mark_consumed(size_t idx, ast2_rule_enum rule_type);

    const ast2_token_scope _get_active_scope() const;
    void _push_scope(ast2_token_scope scope);
    void _pop_scope();

    // Parse simple games
    std::unique_ptr<ast2_integer> _parse_ast2_integer(bool at_least_0);
    std::unique_ptr<ast2_rational> _parse_ast2_rational();
    std::unique_ptr<ast2_up> _parse_ast2_up();
    std::unique_ptr<ast2_nimber> _parse_ast2_nimber();
    std::unique_ptr<ast2_rational_up_nimber> _parse_ast2_rational_up_nimber();
    std::unique_ptr<ast2_explicit_game> _parse_ast2_explicit_game();

    // Parse composite games
    std::unique_ptr<ast2_atomic_game> _parse_ast2_atomic_game();
    std::unique_ptr<ast2_plusminus_game> _parse_ast2_plusminus_game();
    std::unique_ptr<ast2_qualified_game> _parse_ast2_qualified_game();

    // Parse CGT games
    std::unique_ptr<ast2_bracket_sum> _parse_ast2_bracket_sum();
    std::unique_ptr<ast2_sum> _parse_ast2_sum();
    std::unique_ptr<ast2_braced_option_list> _parse_ast2_braced_option_list();
    std::unique_ptr<ast2_option_list> _parse_ast2_option_list();
    std::unique_ptr<ast2_braced_cgt_game> _parse_ast2_braced_cgt_game();
    std::unique_ptr<ast2_unbraced_cgt_game> _parse_ast2_unbraced_cgt_game();

    // Data
    const std::vector<ast2_token>* _tokens;
    size_t _idx;

    std::vector<ast2_token_scope> _scope_stack;

    std::pair<size_t, ast2_rule_enum> _deepest_parse;
};

