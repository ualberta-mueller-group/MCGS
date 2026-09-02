#pragma once

#include <ostream>
#include <memory>
#include <vector>

#include "cgt_game.h"
#include "generic_graph_printer.h"
#include "sign_enum.h"

class game;

//////////////////////////////////////// struct i_ast2_node
struct i_ast2_node
{
    virtual ~i_ast2_node() {}
    virtual void print(std::ostream& os, uint64_t depth) const = 0;
    virtual void print_graph(generic_graph_printer& graph) const = 0;
};

//////////////////////////////////////// struct i_ast2_atomic_game
struct i_ast2_atomic_game: public i_ast2_node
{
    virtual game* make_atomic_game(bool negate) const = 0;
};

//////////////////////////////////////// struct i_ast2_option_set
struct i_ast2_option_set: public i_ast2_node
{
    virtual std::vector<game*> make_option_set(bool negate) const = 0;
};

//////////////////////////////////////// struct ast2_explicit_game
struct ast2_explicit_game: public i_ast2_atomic_game
{
    ast2_explicit_game(std::string game_title, std::string game_contents);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_atomic_game(bool negate) const override;

    std::string game_title;
    std::string game_contents;
};

//////////////////////////////////////// struct ast2_integer
struct ast2_integer: public i_ast2_node
{
    ast2_integer(int64_t value): value(value) {};
    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    int64_t value;
};

//////////////////////////////////////// struct ast2_rational
struct ast2_rational: public i_ast2_node
{
    ast2_rational(int64_t top, int64_t bottom);
    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    int64_t top;
    int64_t bottom;
};

//////////////////////////////////////// struct ast2_up
struct ast2_up: public i_ast2_node
{
    ast2_up(int64_t up_value): up_value(up_value) {}
    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    int64_t up_value;
};

//////////////////////////////////////// struct ast2_nimber
struct ast2_nimber: public i_ast2_node
{
    ast2_nimber(int64_t nim_value);
    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    int64_t nim_value;
};

//////////////////////////////////////// struct ast2_rational_up_nimber
struct ast2_rational_up_nimber: public i_ast2_atomic_game
{
    ast2_rational_up_nimber(std::shared_ptr<ast2_rational> rational,
                            std::shared_ptr<ast2_up> up,
                            std::shared_ptr<ast2_nimber> nimber);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_atomic_game(bool negate) const override;

    std::shared_ptr<ast2_rational> rational;
    std::shared_ptr<ast2_up> up;
    std::shared_ptr<ast2_nimber> nimber;
};

//////////////////////////////////////// struct ast2_game
struct ast2_game: public i_ast2_node
{
    ast2_game(sign_enum sign_type, std::shared_ptr<i_ast2_atomic_game> atomic_game);
    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_game(bool negate) const;

    sign_enum sign_type;
    std::shared_ptr<i_ast2_atomic_game> atomic_game;
};

//////////////////////////////////////// struct ast2_sum
struct ast2_sum: public i_ast2_node
{
    ast2_sum(std::vector<std::pair<sign_enum, std::shared_ptr<ast2_game>>> operands);
    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_game_sum(bool negate) const;

    std::vector<std::pair<sign_enum, std::shared_ptr<ast2_game>>> operands;
};

//////////////////////////////////////// struct ast2_bracket_sum
struct ast2_bracket_sum: public i_ast2_atomic_game
{
    ast2_bracket_sum(std::shared_ptr<ast2_sum> sum);
    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_atomic_game(bool negate) const override;

    std::shared_ptr<ast2_sum> sum;
};

//////////////////////////////////////// struct ast2_game_list
struct ast2_game_list: public i_ast2_option_set
{
    ast2_game_list(std::vector<std::shared_ptr<ast2_sum>> game_list);
    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    std::vector<game*> make_option_set(bool negate) const override;

    std::vector<std::shared_ptr<ast2_sum>> game_list;
};

//////////////////////////////////////// struct ast2_unbraced_cgt_game
struct ast2_unbraced_cgt_game: public i_ast2_option_set
{
    ast2_unbraced_cgt_game(std::shared_ptr<i_ast2_option_set> left_set,
                         std::shared_ptr<i_ast2_option_set> right_set);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    cgt_game* make_cgt_game(bool negate) const;
    std::vector<game*> make_option_set(bool negate) const override;

    std::shared_ptr<i_ast2_option_set> left_set;
    std::shared_ptr<i_ast2_option_set> right_set;
};

//////////////////////////////////////// struct ast2_braced_cgt_game
struct ast2_braced_cgt_game: public i_ast2_atomic_game
{
    ast2_braced_cgt_game(std::shared_ptr<ast2_unbraced_cgt_game> unbraced_cgt);
    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_atomic_game(bool negate) const override;

    std::shared_ptr<ast2_unbraced_cgt_game> unbraced_cgt;
};

