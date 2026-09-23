#pragma once

#include <ostream>
#include <memory>
#include <vector>

#include "cgt_game.h"
#include "generic_graph_printer.h"
#include "sign_enum.h"

class game;

////////////////////////////////////////////////// Node interfaces
class i_ast2_node
{
public:
    virtual ~i_ast2_node() {}
    virtual void print(std::ostream& os, uint64_t depth) const = 0;
    virtual void print_graph(generic_graph_printer& graph) const = 0;
};

class i_ast2_game: public i_ast2_node
{
public:
    virtual game* make_game(bool negate) const = 0;
};

class i_ast2_game_list: public i_ast2_node
{
public:
    virtual std::vector<game*> make_game_list(bool negate) const = 0;
};

////////////////////////////////////////////////// Node type declarations
class ast2_integer;
class ast2_rational;
class ast2_up;
class ast2_nimber;
class ast2_rational_up_nimber;
class ast2_explicit_game;
class ast2_atomic_game;
class ast2_plusminus_game;
class ast2_qualified_game;
class ast2_sum;
class ast2_bracket_sum;
class ast2_option_list;
class ast2_braced_option_list;
class ast2_unbraced_cgt_game;
class ast2_braced_cgt_game;

////////////////////////////////////////////////// Basic games
class ast2_integer : public i_ast2_node
{
public:
    ast2_integer(int64_t value);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    int64_t value;
};

class ast2_rational: public i_ast2_game
{
public:
    ast2_rational(int64_t top, int64_t bottom);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_game(bool negate) const override;

    int64_t top;
    int64_t bottom;
};

class ast2_up: public i_ast2_game
{
public:
    ast2_up(int64_t up_value);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_game(bool negate) const override;
    game* make_game_with_star(bool negate, bool with_star) const;


    int64_t up_value;
};

class ast2_nimber: public i_ast2_game
{
public:
    ast2_nimber(int64_t nim_value);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_game(bool negate) const override;

    int64_t nim_value;
};

class ast2_rational_up_nimber: public i_ast2_game
{
public:
    ast2_rational_up_nimber(std::unique_ptr<ast2_rational> rational,
                            std::unique_ptr<ast2_up> up,
                            std::unique_ptr<ast2_nimber> nimber);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_game(bool negate) const override;

    std::unique_ptr<ast2_rational> rational;
    std::unique_ptr<ast2_up> up;
    std::unique_ptr<ast2_nimber> nimber;
};

class ast2_explicit_game: public i_ast2_game
{
public:
    ast2_explicit_game(std::string game_title, std::string game_contents);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_game(bool negate) const override;

    std::string game_title;
    std::string game_contents;
};

////////////////////////////////////////////////// Composite games
class ast2_atomic_game: public i_ast2_game
{
public:
    using variant_t =                                 //
        std::variant<                                 //
            std::unique_ptr<ast2_rational_up_nimber>, //
            std::unique_ptr<ast2_bracket_sum>,        //
            std::unique_ptr<ast2_braced_cgt_game>,    //
            std::unique_ptr<ast2_explicit_game>>;     //

    ast2_atomic_game(variant_t node_ptr_variant);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_game(bool negate) const override;

    variant_t node_ptr_variant;
};

class ast2_plusminus_game: public i_ast2_game
{
public:
    using variant_t =                                  //
        std::variant<                                  //
            std::unique_ptr<ast2_atomic_game>,         //
            std::unique_ptr<ast2_braced_option_list>>; //

    ast2_plusminus_game(variant_t node_ptr_variant);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_game(bool negate) const override;

    variant_t node_ptr_variant;
};

class ast2_qualified_game: public i_ast2_game
{
public:
    using variant_t =                              //
        std::variant<                              //
            std::unique_ptr<ast2_atomic_game>,     //
            std::unique_ptr<ast2_plusminus_game>>; //

    ast2_qualified_game(std::unique_ptr<ast2_atomic_game> atomic_game,
                        sign_enum unary_sign);
    ast2_qualified_game(std::unique_ptr<ast2_plusminus_game> plusminus_game);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_game(bool negate) const override;

    variant_t node_ptr_variant;
    sign_enum unary_sign;
};

////////////////////////////////////////////////// CGT games
class ast2_sum: public i_ast2_game
{
public:
    using variant_t =                              //
        std::variant<                              //
            std::unique_ptr<ast2_qualified_game>,  //
            std::unique_ptr<ast2_plusminus_game>>; //

    ast2_sum(std::vector<std::pair<sign_enum, variant_t>> summands);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_game(bool negate) const override;

    std::vector<std::pair<sign_enum, variant_t>> summands;
};

class ast2_bracket_sum: public i_ast2_game
{
public:
    ast2_bracket_sum(std::unique_ptr<ast2_sum> sum);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_game(bool negate) const override;

    std::unique_ptr<ast2_sum> sum;
};

class ast2_option_list: public i_ast2_game_list
{
public:
    ast2_option_list(std::vector<std::unique_ptr<ast2_sum>> option_nodes);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    std::vector<game*> make_game_list(bool negate) const override;

    std::vector<std::unique_ptr<ast2_sum>> option_nodes;
};

class ast2_braced_option_list: public i_ast2_game_list
{
public:
    ast2_braced_option_list(std::unique_ptr<ast2_option_list> option_list);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    std::vector<game*> make_game_list(bool negate) const override;

    std::unique_ptr<ast2_option_list> option_list;
};

class ast2_unbraced_cgt_game: public i_ast2_game
{
public:
    using variant_t = std::variant<              //
        std::unique_ptr<ast2_unbraced_cgt_game>, //
        std::unique_ptr<ast2_option_list>>;      //

    ast2_unbraced_cgt_game(variant_t left_node_ptr_variant,
                           variant_t right_node_ptr_variant);

    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

private:
    void _print_graph_helper(generic_graph_printer& graph,
                             const variant_t& node_ptr_variant,
                             const std::string& edge_label) const;

public:
    game* make_game(bool negate) const override;

    variant_t left_node_ptr_variant;
    variant_t right_node_ptr_variant;
};

class ast2_braced_cgt_game: public i_ast2_game
{
public:
    ast2_braced_cgt_game(std::unique_ptr<ast2_unbraced_cgt_game> unbraced_game);
                          
    void print(std::ostream& os, uint64_t depth) const override;
    void print_graph(generic_graph_printer& graph) const override;

    game* make_game(bool negate) const override;

    std::unique_ptr<ast2_unbraced_cgt_game> unbraced_game;
};
