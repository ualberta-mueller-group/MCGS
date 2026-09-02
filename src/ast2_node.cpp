#include "ast2_node.h"

#include <cstring>
#include <sstream>

#include "cgt_dyadic_rational.h"
#include "cgt_game.h"
#include "cgt_nimber.h"
#include "cgt_up_star.h"
#include "file_parser.h"

#include "integral_conversion.h"
#include "sign_enum.h"
#include "throw_assert.h"

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

////////////////////////////////////////////////// ast2_explicit_game methods
ast2_explicit_game::ast2_explicit_game(string game_title, string game_contents)
    : game_title(game_title), game_contents(game_contents)
{
    THROW_ASSERT(!game_title.empty());
}

void ast2_explicit_game::print(ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Explicit_game: ";
    os << "`" << game_title << "`";
    os << " : ";
    os << "`" << game_contents << "`";
    os << endl;
}

void ast2_explicit_game::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Explicit_game: ";
    str << "`" << game_title << "`";
    str << ":";
    str << "`" << game_contents << "`";

    graph.add_vertex(this, str.str());
}

game* ast2_explicit_game::make_atomic_game(bool negate) const
{
    // TODO line number, error handling
    game* g = file_parser::construct_game(game_title, 0, game_contents);

    if (negate)
    {
        game* g_inv = g->inverse();
        delete g;
        g = g_inv;
    }

    return g;
}

////////////////////////////////////////////////// ast2_integer methods
void ast2_integer::print(ostream& os, uint64_t depth) const
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
    THROW_ASSERT(bottom != 0);
}

void ast2_rational::print(ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Rational: " << top << "/" << bottom << endl;
}

void ast2_rational::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Rational: " << top << "/" << bottom;
    graph.add_vertex(this, str.str());
}


////////////////////////////////////////////////// ast2_up methods
void ast2_up::print(ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Up: " << up_value << endl;
}

void ast2_up::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Up: " << up_value;
    graph.add_vertex(this, str.str());
}

////////////////////////////////////////////////// ast2_nimber methods
ast2_nimber::ast2_nimber(int64_t nim_value) : nim_value(nim_value)
{
    THROW_ASSERT(nim_value >= 0);
}

void ast2_nimber::print(ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Nimber: " << nim_value << endl;
}

void ast2_nimber::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Nimber: " << nim_value;
    graph.add_vertex(this, str.str());
}


//////////////////////////////////////////////////
// ast2_rational_up_nimber methods

ast2_rational_up_nimber::ast2_rational_up_nimber(
    shared_ptr<ast2_rational> rational, shared_ptr<ast2_up> up,
    shared_ptr<ast2_nimber> nimber)
    : rational(rational), up(up), nimber(nimber)
{
    THROW_ASSERT(rational || up || nimber);
}

void ast2_rational_up_nimber::print(ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Rational_up_nimber:" << endl;

    const uint64_t depth_next = depth + 1;

    if (rational)
        rational->print(os, depth_next);

    if (up)
        up->print(os, depth_next);

    if (nimber)
        nimber->print(os, depth_next);
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

game* ast2_rational_up_nimber::make_atomic_game(bool negate) const
{
    vector<game*> games;

    if (rational)
    {
        int64_t top64 = rational->top;
        const int64_t bottom64 = rational->bottom;

        if (negate)
            top64 = -top64;

        const int top = integral_cast_checked<int>(top64);
        const int bottom = integral_cast_checked<int>(bottom64);

        games.emplace_back(new dyadic_rational(top, bottom));
    }

    if (up)
    {
        int64_t up_value64 = up->up_value;
        if (negate)
            up_value64 = -up_value64;

        const int up_value = integral_cast_checked<int>(up_value64);

        games.emplace_back(new up_star(up_value, false));
    }

    if (nimber)
        games.emplace_back(new class nimber(nimber->nim_value));

    if (games.size() == 1)
        return games.back();

    vector<shared_ptr<const game>> games_shared_ptr;
    for (game* g : games)
        games_shared_ptr.emplace_back(g);
    return new game_sum(games_shared_ptr);
}


////////////////////////////////////////////////// ast2_game methods
ast2_game::ast2_game(sign_enum sign_type,
                     shared_ptr<i_ast2_atomic_game> atomic_game)
    : sign_type(sign_type), atomic_game(atomic_game)
{
}

void ast2_game::print(ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Game (" << sign_enum_to_string(sign_type) << "):" << endl;

    THROW_ASSERT(atomic_game);
    atomic_game->print(os, depth + 1);
}

void ast2_game::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Game (" << sign_enum_to_string(sign_type) << ")";
    graph.add_vertex(this, str.str());

    THROW_ASSERT(atomic_game);
    atomic_game->print_graph(graph);

    graph.add_edge(this, atomic_game.get(), "");
}

game* ast2_game::make_game(bool negate) const
{
    const bool unary_minus = (sign_type == SIGN_NEGATIVE);
    assert(atomic_game);
    return atomic_game->make_atomic_game(negate ^ unary_minus);
}


////////////////////////////////////////////////// ast2_sum methods
ast2_sum::ast2_sum(vector<pair<sign_enum, shared_ptr<ast2_game>>> operands)
    : operands(operands)
{
}

void ast2_sum::print(ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Sum (" << operands.size() << "):" << endl;

    for (const pair<sign_enum, shared_ptr<ast2_game>>& p : operands)
    {
        print_indent(os, depth);
        os << "[";
        os << ((p.first == SIGN_POSITIVE) ? "PLUS" : "MINUS");
        os << "]" << endl;

        THROW_ASSERT(p.second);
        p.second->print(os, depth + 1);
    }
}

void ast2_sum::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Sum (size " << operands.size() << ")";
    graph.add_vertex(this, str.str());

    const size_t operands_size = operands.size();
    for (size_t i = 0; i < operands_size; i++)
    {
        const pair<sign_enum, shared_ptr<ast2_game>>& p = operands[i];

        THROW_ASSERT(p.second);
        p.second->print_graph(graph);

        const string& edge_label = (p.first == SIGN_POSITIVE) ? "+" : "-";
        graph.add_edge(this, p.second.get(), edge_label);
    }
}

game* ast2_sum::make_game_sum(bool negate) const
{
    vector<game*> games;
    games.reserve(operands.size());

    for (const pair<sign_enum, shared_ptr<ast2_game>>& operand_pair : operands)
    {
        const bool binary_minus = operand_pair.first;
        const shared_ptr<ast2_game>& game_node = operand_pair.second;

        games.emplace_back(game_node->make_game(negate ^ binary_minus));
    }

    if (games.size() == 1)
        return games.back();

    vector<shared_ptr<const game>> games_shared_ptr;
    for (game* g : games)
        games_shared_ptr.emplace_back(g);

    return new game_sum(games_shared_ptr);
}

////////////////////////////////////////////////// ast2_bracket_sum methods
ast2_bracket_sum::ast2_bracket_sum(shared_ptr<ast2_sum> sum) : sum(sum)
{
}

void ast2_bracket_sum::print(ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Bracket_sum: " << endl;

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

game* ast2_bracket_sum::make_atomic_game(bool negate) const
{
    return sum->make_game_sum(negate);
}

////////////////////////////////////////////////// ast2_game_list methods
ast2_game_list::ast2_game_list(vector<shared_ptr<ast2_sum>> game_list)
    : game_list(game_list)
{
}

void ast2_game_list::print(ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Game_list:" << endl;

    for (const shared_ptr<ast2_sum>& g : game_list)
        g->print(os, depth + 1);
}

void ast2_game_list::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Game_list (size " << game_list.size() << ")";
    graph.add_vertex(this, str.str());

    const size_t game_list_size = game_list.size();
    for (size_t i = 0; i < game_list_size; i++)
    {
        const shared_ptr<ast2_sum>& g = game_list[i];

        THROW_ASSERT(g);
        g->print_graph(graph);
        graph.add_edge(this, g.get(), "");
    }
}

std::vector<game*> ast2_game_list::make_option_set(bool negate) const
{
    vector<game*> games;
    games.reserve(game_list.size());

    for (const shared_ptr<ast2_sum>& g_node : game_list)
        games.emplace_back(g_node->make_game_sum(negate));

    return games;
}


//////////////////////////////////////////////////
// ast2_unbraced_cgt_game methods

ast2_unbraced_cgt_game::ast2_unbraced_cgt_game(
    shared_ptr<i_ast2_option_set> left_set,
    shared_ptr<i_ast2_option_set> right_set)
    : left_set(left_set), right_set(right_set)
{
}

void ast2_unbraced_cgt_game::print(ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Unbraced_cgt_game:" << endl;

    const uint64_t depth_next = depth + 1;

    THROW_ASSERT(left_set);
    THROW_ASSERT(right_set);

    print_indent(os, depth);
    os << "(LEFT SET):" << endl;
    left_set->print(os, depth_next);

    print_indent(os, depth);
    os << "(RIGHT SET):" << endl;
    right_set->print(os, depth_next);
}

void ast2_unbraced_cgt_game::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Unbraced_CGT_game";
    graph.add_vertex(this, str.str());

    THROW_ASSERT(left_set);
    left_set->print_graph(graph);
    graph.add_edge(this, left_set.get(), "Left_set");

    THROW_ASSERT(right_set);
    right_set->print_graph(graph);
    graph.add_edge(this, right_set.get(), "Right_set");
}


cgt_game* ast2_unbraced_cgt_game::make_cgt_game(bool negate) const
{
    vector<game*> left_games = left_set->make_option_set(negate);
    vector<game*> right_games = right_set->make_option_set(negate);

    vector<shared_ptr<const game>> left_games_shared;
    left_games_shared.reserve(left_games.size());
    for (game* g : left_games)
        left_games_shared.emplace_back(g);

    vector<shared_ptr<const game>> right_games_shared;
    right_games_shared.reserve(right_games.size());
    for (game* g : right_games)
        right_games_shared.emplace_back(g);

    if (negate)
        return new cgt_game(right_games_shared, left_games_shared);
    return new cgt_game(left_games_shared, right_games_shared);
}

vector<game*> ast2_unbraced_cgt_game::make_option_set(bool negate) const
{
    // Return a single option (a cgt_game)
    vector<game*> games;
    games.emplace_back(make_cgt_game(negate));
    return games;
}

////////////////////////////////////////////////// ast2_braced_cgt_game methods
ast2_braced_cgt_game::ast2_braced_cgt_game(
    shared_ptr<ast2_unbraced_cgt_game> unbraced_cgt)
    : unbraced_cgt(unbraced_cgt)
{
}

void ast2_braced_cgt_game::print(ostream& os, uint64_t depth) const
{
    print_indent(os, depth);
    os << "Braced_cgt_game:" << endl;

    THROW_ASSERT(unbraced_cgt);
    unbraced_cgt->print(os, depth + 1);
}

void ast2_braced_cgt_game::print_graph(generic_graph_printer& graph) const
{
    stringstream str;
    str << "Braced_CGT_game";
    graph.add_vertex(this, str.str());

    THROW_ASSERT(unbraced_cgt);
    unbraced_cgt->print_graph(graph);

    graph.add_edge(this, unbraced_cgt.get(), "");
}

game* ast2_braced_cgt_game::make_atomic_game(bool negate) const
{
    return unbraced_cgt->make_cgt_game(negate);
}

