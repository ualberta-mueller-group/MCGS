#include "ast2_node.h"

#include <cstring>
#include <sstream>
#include <type_traits>
#include <variant>

#include "cgt_dyadic_rational.h"
#include "cgt_game.h"
#include "cgt_integer_game.h"
#include "cgt_nimber.h"
#include "cgt_up_star.h"
#include "file_parser.h"

#include "game.h"
#include "integral_conversion.h"
#include "safe_arithmetic.h"
#include "sign_enum.h"
#include "throw_assert.h"
#include "utilities.h"

using namespace std;

////////////////////////////////////////////////// Helpers
namespace {
void print_indent(ostream& os, uint64_t depth)
{
    for (uint64_t i = 0; i < depth; i++)
        os << '_';
    os.flush();
}

} // namespace

////////////////////////////////////////////////// ast2_integer methods
ast2_integer::ast2_integer(int64_t value) : value(value)
{
}

void ast2_integer::print(std::ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Integer: " << value << endl;
}

void ast2_integer::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Integer: " << value;
    graph.add_vertex(this, str.str());
}

////////////////////////////////////////////////// ast2_rational methods
ast2_rational::ast2_rational(int64_t top, int64_t bottom)
    : top(top), bottom(bottom)
{
}

void ast2_rational::print(std::ostream& os, uint64_t depth) const
{
    print_indent(os, depth);

    os << "Rational: " << top;
    if (bottom != 0)
        os << "/" << bottom;

    os << endl;
}

void ast2_rational::print_graph(generic_graph_printer& graph) const
{
    stringstream str;

    str << "Rational: " << top;
    if (bottom != 0)
        str << "/" << bottom;

    graph.add_vertex(this, str.str());
}

game* ast2_rational::make_game(bool negate) const
{
    int top_as_int = integral_cast_checked<int>(top);
    const int bottom_as_int = integral_cast_checked<int>(bottom);

    THROW_ASSERT(negate_is_safe(top_as_int));

    if (negate)
        top_as_int = -top_as_int;

    if (bottom_as_int == 1)
        return new integer_game(top_as_int);
    return new dyadic_rational(top_as_int, bottom_as_int);
}

////////////////////////////////////////////////// ast2_up methods
ast2_up::ast2_up(int64_t up_value) : up_value(up_value)
{
}

void ast2_up::print(std::ostream& os, uint64_t depth) const
{
    print_indent(os, depth);

    os << "Up: ";

    if (up_value > 0)
        os << "^";
    if (up_value < 0)
        os << "v";

    os << up_value;
    os << endl;
}

void ast2_up::print_graph(generic_graph_printer& graph) const
{
    stringstream str;

    str << "Up: ";

    if (up_value > 0)
        str << "^";
    if (up_value < 0)
        str << "v";

    str << up_value;

    graph.add_vertex(this, str.str());
}

game* ast2_up::make_game(bool negate) const
{
    return make_game_with_star(negate, false);
}

game* ast2_up::make_game_with_star(bool negate, bool with_star) const
{

    int up_value_as_int = integral_cast_checked<int>(up_value);
    THROW_ASSERT(negate_is_safe(up_value_as_int));

    if (negate)
        up_value_as_int = -up_value_as_int;

    return new up_star(up_value_as_int, with_star);
}

////////////////////////////////////////////////// ast2_nimber methods
ast2_nimber::ast2_nimber(int64_t nim_value) : nim_value(nim_value)
{
    THROW_ASSERT(nim_value >= 0);
}

void ast2_nimber::print(std::ostream& os, uint64_t depth) const
{
    print_indent(os, depth);

    os << "Nimber: ";
    if (nim_value != 0)
        os << "*";
    os << nim_value;

    os << endl;
}

void ast2_nimber::print_graph(generic_graph_printer& graph) const
{
    stringstream str;

    str << "Nimber: ";
    if (nim_value != 0)
        str << "*";
    str << nim_value;

    graph.add_vertex(this, str.str());
}

game* ast2_nimber::make_game(bool negate) const
{
    const int nim_value_as_int = integral_cast_checked<int>(nim_value);
    return new nimber(nim_value_as_int);
}

//////////////////////////////////////////////////
// ast2_rational_up_nimber methods

ast2_rational_up_nimber::ast2_rational_up_nimber(
    std::unique_ptr<ast2_rational> rational, std::unique_ptr<ast2_up> up,
    std::unique_ptr<ast2_nimber> nimber)
    : rational(std::move(rational)),
      up(std::move(up)),
      nimber(std::move(nimber))
{
    THROW_ASSERT(this->rational || this->up || this->nimber);
}

void ast2_rational_up_nimber::print(std::ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    const uint64_t next_depth = depth + 1;

    os << "Rational_up_nimber: ";
    os << endl;

    if (rational)
        rational->print(os, next_depth);
    if (up)
        up->print(os, next_depth);
    if (nimber)
        nimber->print(os, next_depth);
}

void ast2_rational_up_nimber::print_graph(generic_graph_printer& graph) const
{
    stringstream str;

    str << "Rational_up_nimber";
    graph.add_vertex(this, str.str());

    if (rational)
    {
        rational->print_graph(graph);
        graph.add_edge(this, rational.get(), "");
    }

    if (up)
    {
        up->print_graph(graph);
        graph.add_edge(this, up.get(), "");
    }

    if (nimber)
    {
        nimber->print_graph(graph);
        graph.add_edge(this, nimber.get(), "");
    }
}

game* ast2_rational_up_nimber::make_game(bool negate) const
{
    vector<game*> games;

    if (rational)
        games.push_back(rational->make_game(negate));

    if (up && nimber && nimber->nim_value == 1)
        games.push_back(up->make_game_with_star(negate, true));
    else
    {
        if (up)
            games.push_back(up->make_game(negate));

        if (nimber)
            games.push_back(nimber->make_game(negate));
    }

    THROW_ASSERT(games.size() > 0);

    if (games.size() == 1)
        return games.back();

    vector<shared_ptr<const game>> games_shared;
    games_shared.reserve(games.size());

    for (game* g : games)
        games_shared.emplace_back(g);

    return new game_sum(games_shared);
}


////////////////////////////////////////////////// ast2_explicit_game methods
ast2_explicit_game::ast2_explicit_game(std::string game_title,
                                       std::string game_contents)
    : game_title(game_title), game_contents(game_contents)

{
}

void ast2_explicit_game::print(std::ostream& os, uint64_t depth) const
{
    print_indent(os, depth);

    os << "Explicit_game: ";
    os << "`" << game_title << "`";
    os << " ";
    os << "`" << game_contents << "`";
    os << endl;
}

void ast2_explicit_game::print_graph(generic_graph_printer& graph) const
{
    stringstream str;

    str << "Explicit_game: ";
    str << game_title << "(" << game_contents << ")";

    graph.add_vertex(this, str.str());
}

game* ast2_explicit_game::make_game(bool negate) const
{
    game* g = file_parser::construct_game(game_title, 0, game_contents);

    if (negate)
    {
        game* g_temp = g->inverse();
        delete g;
        g = g_temp;
    }

    return g;
}

////////////////////////////////////////////////// ast2_atomic_game methods
ast2_atomic_game::ast2_atomic_game(variant_t node_ptr_variant)
    : node_ptr_variant(std::move(node_ptr_variant))
{
    std::visit([&](const auto& node_ptr) -> void
    {
        THROW_ASSERT(node_ptr);
    }, this->node_ptr_variant);
}

void ast2_atomic_game::print(std::ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Atomic_game: " << endl;

    std::visit([&](const auto& node_ptr) -> void
    {
        THROW_ASSERT(node_ptr);
        node_ptr->print(os, depth + 1);
    }, node_ptr_variant);
}

void ast2_atomic_game::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Atomic_game";
    graph.add_vertex(this, str.str());

    std::visit([&](const auto& node_ptr) -> void
    {
        THROW_ASSERT(node_ptr);
        node_ptr->print_graph(graph);
        graph.add_edge(this, node_ptr.get(), "");
    }, node_ptr_variant);
}

game* ast2_atomic_game::make_game(bool negate) const
{
    return std::visit([&](const auto& node_ptr) -> game*
    {
        THROW_ASSERT(node_ptr);
        return node_ptr->make_game(negate);
    }, node_ptr_variant);
}

////////////////////////////////////////////////// ast2_plusminus_game methods
ast2_plusminus_game::ast2_plusminus_game(variant_t node_ptr_variant)
    : node_ptr_variant(std::move(node_ptr_variant))
{
    std::visit([&](const auto& node_ptr) -> void
    {
        THROW_ASSERT(node_ptr);
    }, this->node_ptr_variant);
}

void ast2_plusminus_game::print(std::ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Plusminus_game:" << endl;

    std::visit([&](const auto& node_ptr) -> void
    {
        THROW_ASSERT(node_ptr);
        node_ptr->print(os, depth + 1);
    }, node_ptr_variant);
}

void ast2_plusminus_game::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Plusminus_game";
    graph.add_vertex(this, str.str());

    std::visit([&](const auto& node_ptr) -> void
    {
        THROW_ASSERT(node_ptr);
        node_ptr->print_graph(graph);
        graph.add_edge(this, node_ptr.get(), "");
    }, node_ptr_variant);
}

game* ast2_plusminus_game::make_game(bool negate) const
{
    /*
       +-G := {G | -G}

       +-{O1, ..., On} := {O1, ..., On | -O1, ..., -On}
    */

    negate = false; // -(+-G) = +-G

    vector<game*> left_options =
        std::visit([&](const auto& node_ptr) -> vector<game*>
    {
        THROW_ASSERT(node_ptr);

        using node_t = std::decay_t<decltype(*node_ptr)>;
        constexpr bool IS_GAME = std::is_base_of_v<i_ast2_game, node_t>;
        constexpr bool IS_GAME_LIST = std::is_base_of_v<i_ast2_game_list, node_t>;
        static_assert(!(IS_GAME && IS_GAME_LIST));

        if constexpr (IS_GAME)
            return vector<game*>{node_ptr->make_game(negate)};
        else if constexpr (IS_GAME_LIST)
            return node_ptr->make_game_list(negate);
        else
            static_assert(false);

        assert(false);
    }, node_ptr_variant);

    vector<shared_ptr<const game>> left_options_shared;
    vector<shared_ptr<const game>> right_options_shared;

    const size_t n_options = left_options.size();
    left_options_shared.reserve(n_options);
    right_options_shared.reserve(n_options);

    for (game* g : left_options)
    {
        left_options_shared.emplace_back(g);
        right_options_shared.emplace_back(g->inverse());
    }

    return new cgt_game(left_options_shared, right_options_shared);
}

////////////////////////////////////////////////// ast2_qualified_game
ast2_qualified_game::ast2_qualified_game(
    std::unique_ptr<ast2_atomic_game> atomic_game, sign_enum unary_sign)
    : node_ptr_variant(std::move(atomic_game)), unary_sign(unary_sign)
{
    std::visit([&](const auto& node_ptr) -> void
    {
        THROW_ASSERT(node_ptr);
    }, this->node_ptr_variant);
}

ast2_qualified_game::ast2_qualified_game(
    std::unique_ptr<ast2_plusminus_game> plusminus_game)
    : node_ptr_variant(std::move(plusminus_game)), unary_sign(SIGN_POSITIVE)
{
    std::visit([&](const auto& node_ptr) -> void
    {
        THROW_ASSERT(node_ptr);
    }, this->node_ptr_variant);
}

void ast2_qualified_game::print(std::ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Qualified_game";

    if (unary_sign == SIGN_NEGATIVE)
        os << " (NEGATIVE)";

    os << ":" << endl;

    std::visit([&](const auto& node_ptr) -> void
    {
        THROW_ASSERT(node_ptr);
        node_ptr->print(os, depth + 1);
    }, node_ptr_variant);
}

void ast2_qualified_game::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Qualified_game";
    graph.add_vertex(this, str.str());

    const string& edge_label = (unary_sign == SIGN_NEGATIVE) ? "-" : "+";

    std::visit([&](const auto& node_ptr) -> void
    {
        THROW_ASSERT(node_ptr);
        node_ptr->print_graph(graph);
        graph.add_edge(this, node_ptr.get(), edge_label);
    }, node_ptr_variant);
}

game* ast2_qualified_game::make_game(bool negate) const
{
    negate ^= (unary_sign == SIGN_NEGATIVE);

    return std::visit([&](const auto& node_ptr) -> game*
    {
        THROW_ASSERT(node_ptr);
        return node_ptr->make_game(negate);
    }, node_ptr_variant);
}

////////////////////////////////////////////////// ast2_sum methods
ast2_sum::ast2_sum(std::vector<std::pair<sign_enum, variant_t>> summands)
    : summands(std::move(summands))
{
    THROW_ASSERT(LOGICAL_IMPLIES(!this->summands.empty(),
                                 this->summands.front().first == SIGN_POSITIVE));

    for (const pair<sign_enum, variant_t>& summand : this->summands)
    {
        std::visit([&](const auto& node_ptr) -> void
        {
            THROW_ASSERT(node_ptr);
        }, summand.second);
    }
}

void ast2_sum::print(std::ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Sum (size " << summands.size() << "):" << endl;

    for (const pair<sign_enum, variant_t>& summand : summands)
    {
        const sign_enum binary_sign = summand.first;
        const variant_t& node_ptr_variant = summand.second;

        print_indent(os, depth);
        os << "[BINARY ";
        os << ((binary_sign == SIGN_POSITIVE) ? "PLUS" : "MINUS");
        os << "]" << endl;

        std::visit([&](const auto& node_ptr) -> void
        {
            THROW_ASSERT(node_ptr);
            node_ptr->print(os, depth + 1);
        }, node_ptr_variant);
    }
}

void ast2_sum::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Sum (size " << summands.size() << ")";
    graph.add_vertex(this, str.str());

    for (const pair<sign_enum, variant_t>& summand : summands)
    {
        const sign_enum binary_sign = summand.first;
        const variant_t& node_ptr_variant = summand.second;

        const string& edge_label = (binary_sign == SIGN_NEGATIVE) ? "-" : "+";

        std::visit([&](const auto& node_ptr) -> void
        {
            THROW_ASSERT(node_ptr);
            node_ptr->print_graph(graph);
            graph.add_edge(this, node_ptr.get(), edge_label);
        }, node_ptr_variant);
    }
}

game* ast2_sum::make_game(bool negate) const
{
    vector<game*> games;
    games.reserve(summands.size());

    for (const pair<sign_enum, variant_t>& summand : summands)
    {
        const bool negate_single = negate ^ (summand.first == SIGN_NEGATIVE);
        const variant_t& node_ptr_variant = summand.second;

        std::visit([&](const auto& node_ptr) -> void
        {
            THROW_ASSERT(node_ptr);
            games.push_back(node_ptr->make_game(negate_single));
        }, node_ptr_variant);
    }

    if (games.size() == 1)
        return games.back();

    vector<shared_ptr<const game>> games_shared;
    games_shared.reserve(games.size());

    for (game* g : games)
        games_shared.emplace_back(g);

    return new game_sum(games_shared);
}

////////////////////////////////////////////////// ast2_bracket_sum methods
ast2_bracket_sum::ast2_bracket_sum(std::unique_ptr<ast2_sum> sum)
    : sum(std::move(sum))
{
    THROW_ASSERT(this->sum);
}

void ast2_bracket_sum::print(std::ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Bracket_sum:" << endl;

    THROW_ASSERT(sum);
    sum->print(os, depth + 1);
}

void ast2_bracket_sum::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Bracket_sum";
    graph.add_vertex(this, str.str());

    THROW_ASSERT(sum);
    sum->print_graph(graph);
    graph.add_edge(this, sum.get(), "");
}

game* ast2_bracket_sum::make_game(bool negate) const
{
    THROW_ASSERT(sum);
    return sum->make_game(negate);
}

////////////////////////////////////////////////// ast2_option_list methods
ast2_option_list::ast2_option_list(
    std::vector<std::unique_ptr<ast2_sum>> option_nodes)
    : option_nodes(std::move(option_nodes))
{
    for (const unique_ptr<ast2_sum>& node_ptr : this->option_nodes)
    {
        THROW_ASSERT(node_ptr);
    }
}

void ast2_option_list::print(std::ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Option_list (size " << option_nodes.size() << "):" << endl;

    for (const unique_ptr<ast2_sum>& option : option_nodes)
    {
        THROW_ASSERT(option);
        option->print(os, depth + 1);
    }
}

void ast2_option_list::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Option_list (size " << option_nodes.size() << ")";
    graph.add_vertex(this, str.str());

    for (const unique_ptr<ast2_sum>& option : option_nodes)
    {
        THROW_ASSERT(option);
        option->print_graph(graph);
        graph.add_edge(this, option.get(), "");
    }
}

std::vector<game*> ast2_option_list::make_game_list(bool negate) const
{
    vector<game*> games;
    games.reserve(option_nodes.size());

    for (const unique_ptr<ast2_sum>& option : option_nodes)
    {
        THROW_ASSERT(option);
        games.push_back(option->make_game(negate));
    }

    return games;
}
//////////////////////////////////////////////////
// ast2_braced_option_list methods

ast2_braced_option_list::ast2_braced_option_list(
    std::unique_ptr<ast2_option_list> option_list)
    : option_list(std::move(option_list))
{
    THROW_ASSERT(this->option_list);
}

void ast2_braced_option_list::print(std::ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Braced_option_list:" << endl;
    THROW_ASSERT(option_list);
    option_list->print(os, depth + 1);
}

void ast2_braced_option_list::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Braced_option_list";
    graph.add_vertex(this, str.str());

    THROW_ASSERT(option_list);
    option_list->print_graph(graph);
    graph.add_edge(this, option_list.get(), "");
}

std::vector<game*> ast2_braced_option_list::make_game_list(bool negate) const
{
    THROW_ASSERT(option_list);
    return option_list->make_game_list(negate);
}

//////////////////////////////////////////////////
// ast2_unbraced_cgt_game methods

ast2_unbraced_cgt_game::ast2_unbraced_cgt_game(variant_t left_node_ptr_variant,
                                               variant_t right_node_ptr_variant)
    : left_node_ptr_variant(std::move(left_node_ptr_variant)),
      right_node_ptr_variant(std::move(right_node_ptr_variant))
{
    auto assert_not_null = [&](const auto& node_ptr) -> void
    {
        THROW_ASSERT(node_ptr);
    };

    std::visit(assert_not_null, this->left_node_ptr_variant);
    std::visit(assert_not_null, this->right_node_ptr_variant);
}

void ast2_unbraced_cgt_game::print(std::ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Unbraced_cgt_game:" << endl;

    auto print_set = [&](const auto& node_ptr) -> void
    {
        THROW_ASSERT(node_ptr);
        node_ptr->print(os, depth + 1);
    };

    print_indent(os, depth);
    os << "(LEFT SET):" << endl;
    std::visit(print_set, left_node_ptr_variant);

    print_indent(os, depth);
    os << "(RIGHT SET):" << endl;
    std::visit(print_set, right_node_ptr_variant);
}

void ast2_unbraced_cgt_game::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Unbraced_cgt_game";
    graph.add_vertex(this, str.str());

    _print_graph_helper(graph, left_node_ptr_variant, "Left_set");
    _print_graph_helper(graph, right_node_ptr_variant, "Right_set");
}

void ast2_unbraced_cgt_game::_print_graph_helper(
    generic_graph_printer& graph, const variant_t& node_ptr_variant,
    const std::string& edge_label) const
{
    std::visit([&](const auto& node_ptr) -> void
    {
        THROW_ASSERT(node_ptr);
        node_ptr->print_graph(graph);
        graph.add_edge(this, node_ptr.get(), edge_label);
    }, node_ptr_variant);
}

game* ast2_unbraced_cgt_game::make_game(bool negate) const
{
    auto get_options =
        [&](const auto& node_ptr) -> vector<shared_ptr<const game>>
    {
        THROW_ASSERT(node_ptr);

        using node_t = std::decay_t<decltype(*node_ptr)>;
        constexpr bool IS_GAME = std::is_base_of_v<i_ast2_game, node_t>;
        constexpr bool IS_GAME_LIST = std::is_base_of_v<i_ast2_game_list, node_t>;
        static_assert(!(IS_GAME && IS_GAME_LIST));

        if constexpr (IS_GAME)
        {
            vector<shared_ptr<const game>> options;
            options.emplace_back(node_ptr->make_game(negate));
            return options;
        }
        else if constexpr (IS_GAME_LIST)
        {
            vector<game*> options_raw = node_ptr->make_game_list(negate);

            vector<shared_ptr<const game>> options_shared;
            options_shared.reserve(options_raw.size());

            for (game* g : options_raw)
                options_shared.emplace_back(g);

            return options_shared;
        }
        else
            static_assert(false);

        assert(false);
    };

    vector<shared_ptr<const game>> left_options =
        std::visit(get_options, left_node_ptr_variant);

    vector<shared_ptr<const game>> right_options =
        std::visit(get_options, right_node_ptr_variant);

    if (negate)
        return new cgt_game(right_options, left_options);
    return new cgt_game(left_options, right_options);
}

////////////////////////////////////////////////// ast2_braced_cgt_game methods
ast2_braced_cgt_game::ast2_braced_cgt_game(
    std::unique_ptr<ast2_unbraced_cgt_game> unbraced_game)
    : unbraced_game(std::move(unbraced_game))
{
    THROW_ASSERT(this->unbraced_game);
}

void ast2_braced_cgt_game::print(std::ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Braced_cgt_game:" << endl;

    THROW_ASSERT(unbraced_game);
    unbraced_game->print(os, depth + 1);
}

void ast2_braced_cgt_game::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Braced_cgt_game";
    graph.add_vertex(this, str.str());

    THROW_ASSERT(unbraced_game);
    unbraced_game->print_graph(graph);
    graph.add_edge(this, unbraced_game.get(), "");
}

game* ast2_braced_cgt_game::make_game(bool negate) const
{
    THROW_ASSERT(unbraced_game);
    return unbraced_game->make_game(negate);
}
