/*
   prototyping for file parser refactor

   TODO move some functions to cpp file
*/
#pragma once

#include "cgt_basics.h"
#include "throw_assert.h"
#include "utilities.h"
#include <limits>
#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <iostream>
#include <optional>

// TODO think of a better name for fp_visitor_generate...

////////////////////////////////////////////////// forward declarations
class i_fp_visitor; // interface for visitors to fp_chunk (a chunk of input)
class fp_visitor_print; // visits fp_chunk and prints it to some std::ostream
class fp_visitor_generate; // visits fp_chunk and gives you a test case

/*
    void fp_visitor_print::visit(const fp_chunk&, int command_idx, ostream&)
    void fp_visitor_generate::visit(const fp_chunk&, int command_idx, game_case&)
*/

class i_fp_expr; // interface for all input expressions

class i_fp_expr_content; // interface for all non-command expressions
class fp_expr_title; // game title i.e. "[clobber]"
class fp_expr_game; // game token, i.e. "XXO" or "(1, 2)"
class fp_expr_comment; // comment (possibly prefixed by "_", "#0", "#1", or "#2")

class i_fp_expr_command; // interface for all commands inside of curly braces
class fp_expr_command_solve_bw; // solve for BLACK or WHITE
class fp_expr_command_solve_n; // solve nim value
//class fp_expr_command_winning_moves;

/*
    Represents a chunk of input (CLI game string, or .test file, or stdin)

    Content expressions should be visited first, then command expression(s)

    The "print" visitor may visit all command expressions
    The "generate" visitor will only visit one command expression

    TODO record the last title expression added, and keep it after
    the expression vectors are cleared. Chunks can have implicit titles which
    carry over from a previous chunk. This is easy to handle for the "generate"
    visitor: if a game is visited before any title, check some persistent
    fp_chunk variable holding the implicit title.

    For the "print" visitor this is a problem. Titles should be annotated? The
    output file should be an AST?

*/
class fp_chunk;

////////////////////////////////////////////////// interfaces
//////////////////////////////////////// interface i_fp_visitor
class i_fp_visitor
{
public:
    virtual ~i_fp_visitor() {}


    virtual void visit(const fp_chunk& chunk, int case_idx) = 0;
    //virtual void visit(fp_expr_version& expr) = 0;
    virtual void visit(const fp_expr_title& expr) = 0;
    virtual void visit(const fp_expr_game& expr) = 0;
    virtual void visit(const fp_expr_comment& expr) = 0;
    virtual void visit(const fp_expr_command_solve_bw& expr) = 0;
    virtual void visit(const fp_expr_command_solve_n& expr) = 0;
    //virtual void visit(fp_expr_command_winning_moves& expr) = 0;

private:
};

//////////////////////////////////////// interface i_fp_expr
class i_fp_expr
{
public:
    i_fp_expr(int line_no);
    virtual ~i_fp_expr() {}

    virtual void accept(i_fp_visitor& visitor) const = 0;

    int get_line_no() const;

private:
    const int _line_no;
};

inline i_fp_expr::i_fp_expr(int line_no) : _line_no(line_no)
{
}

inline int i_fp_expr::get_line_no() const
{
    return _line_no;
}

//////////////////////////////////////// interface i_fp_expr_content
class i_fp_expr_content: public i_fp_expr
{
public:
    i_fp_expr_content(int line_no);

private:
};

inline i_fp_expr_content::i_fp_expr_content(int line_no)
    : i_fp_expr(line_no)
{
}

//////////////////////////////////////// interface i_fp_expr_command
class i_fp_expr_command: public i_fp_expr
{
public:
    i_fp_expr_command(int file_no);

private:
};

inline i_fp_expr_command::i_fp_expr_command(int line_no)
    : i_fp_expr(line_no)
{
}

////////////////////////////////////////////////// classes
//////////////////////////////////////// class fp_expr_title
class fp_expr_title: public i_fp_expr_content
{
public:
    fp_expr_title(int line_no, const std::string& title);
    void accept(i_fp_visitor& visitor) const override;

    const std::string& get_title() const;

private:
    const std::string _title;
};

inline fp_expr_title::fp_expr_title(int line_no, const std::string& title)
    : i_fp_expr_content(line_no),
      _title(title)
{
}

inline void fp_expr_title::accept(i_fp_visitor& visitor) const
{
    visitor.visit(*this);
}

inline const std::string& fp_expr_title::get_title() const
{
    return _title;
}

//////////////////////////////////////// class fp_expr_game
class fp_expr_game: public i_fp_expr_content
{
public:
    fp_expr_game(int line_no, const std::string& game_token, bool is_bracketed);
    void accept(i_fp_visitor& visitor) const override;

    const std::string& get_game_token() const;
    bool is_bracketed() const;

private:
    const std::string _game_token;
    const bool _is_bracketed;
};

inline fp_expr_game::fp_expr_game(int line_no, const std::string& game_token,
                           bool is_bracketed)
    : i_fp_expr_content(line_no),
      _game_token(game_token),
      _is_bracketed(is_bracketed)
{
    THROW_ASSERT(LOGICAL_IMPLIES(string_contains_whitespace(_game_token),
                                 _is_bracketed));
}

inline void fp_expr_game::accept(i_fp_visitor& visitor) const
{
    visitor.visit(*this);
}

inline const std::string& fp_expr_game::get_game_token() const
{
    return _game_token;
}

inline bool fp_expr_game::is_bracketed() const
{
    return _is_bracketed;
}

//////////////////////////////////////// class fp_expr_comment
enum fp_expr_comment_type
{
    FP_EXPR_COMMENT_TYPE_SHARED = 0,
    FP_EXPR_COMMENT_TYPE_NUMBERED,
    FP_EXPR_COMMENT_TYPE_SILENT,
};

class fp_expr_comment: public i_fp_expr_content
{
public:
    fp_expr_comment(int line_no, const std::string& comment_string);
    void accept(i_fp_visitor& visitor) const override;

    const std::string& get_comment() const;
    fp_expr_comment_type get_comment_type() const;
    int get_number() const;

private:
    const std::string _comment;
    fp_expr_comment_type _comment_type;
    int _number;
};

inline fp_expr_comment::fp_expr_comment(int line_no,
                                        const std::string& comment_string)
    : i_fp_expr_content(line_no),
      _comment(comment_string)
{
    _comment_type = FP_EXPR_COMMENT_TYPE_SHARED;
    _number = -1;
}

inline void fp_expr_comment::accept(i_fp_visitor& visitor) const
{
    visitor.visit(*this);
}

inline const std::string& fp_expr_comment::get_comment() const
{
    return _comment;
}

inline fp_expr_comment_type fp_expr_comment::get_comment_type() const
{
    return _comment_type;
}

inline int fp_expr_comment::get_number() const
{
    THROW_ASSERT(_comment_type == FP_EXPR_COMMENT_TYPE_SHARED);
    return _number;
}

//////////////////////////////////////// class fp_expr_command_solve_bw
enum minimax_outcome_enum
{
    MINIMAX_OUTCOME_NONE = 0,
    MINIMAX_OUTCOME_WIN,
    MINIMAX_OUTCOME_LOSS,
};

class fp_expr_command_solve_bw: public i_fp_expr_command
{
public:
    fp_expr_command_solve_bw(int line_no, bw player,
                             minimax_outcome_enum expected_outcome);

    void accept(i_fp_visitor& visitor) const override;

    bw get_player() const;
    minimax_outcome_enum get_expected_outcome() const;

private:
    const bw _player;
    const minimax_outcome_enum _expected_outcome;
};

inline fp_expr_command_solve_bw::fp_expr_command_solve_bw(int line_no, bw player,
                         minimax_outcome_enum expected_outcome)
    : i_fp_expr_command(line_no),
      _player(player),
      _expected_outcome(expected_outcome)
{
}

inline void fp_expr_command_solve_bw::accept(i_fp_visitor& visitor) const
{
    visitor.visit(*this);
}

inline bw fp_expr_command_solve_bw::get_player() const
{
    return _player;
}

inline minimax_outcome_enum fp_expr_command_solve_bw::get_expected_outcome() const
{
    return _expected_outcome;
}

//////////////////////////////////////// class fp_expr_command_solve_n
class fp_expr_command_solve_n: public i_fp_expr_command
{
public:
    fp_expr_command_solve_n(int line_no, const std::optional<int>& expected_nim_value);

    void accept(i_fp_visitor& visitor) const override;

    const std::optional<int>& get_expected_nim_value() const;

private:
    const std::optional<int> _expected_nim_value;
};

inline fp_expr_command_solve_n::fp_expr_command_solve_n(
    int line_no, const std::optional<int>& expected_nim_value)
    : i_fp_expr_command(line_no),
      _expected_nim_value(expected_nim_value)
{
}

inline void fp_expr_command_solve_n::accept(i_fp_visitor& visitor) const
{
    visitor.visit(*this);
}

inline const std::optional<int>& fp_expr_command_solve_n::
    get_expected_nim_value() const
{
    return _expected_nim_value;
}

//////////////////////////////////////// class fp_chunk
class fp_chunk
{
public:
    fp_chunk();

    // content exprs
    int n_content_exprs() const;
    void add_content_expr(i_fp_expr_content* expr);
    const i_fp_expr_content& get_content_expr(int idx) const;
    void clear_content_exprs();

    // command exprs
    int n_command_exprs() const;
    void add_command_expr(i_fp_expr_command* expr);
    const i_fp_expr_command& get_command_expr(int idx) const;
    void clear_command_exprs();

    // version string
    void set_version_string(const std::string& version_string);
    void clear_version_string();
    bool has_version_string() const;
    const std::optional<std::string>& get_version_string() const;

private:
    std::vector<std::unique_ptr<i_fp_expr_content>> _content_exprs;
    std::vector<std::unique_ptr<i_fp_expr_command>> _command_exprs;

    std::optional<std::string> _version_string;
};

inline fp_chunk::fp_chunk()
{
}

inline int fp_chunk::n_content_exprs() const
{
    const size_t n_elements = _content_exprs.size();
    THROW_ASSERT(0 <= n_elements && n_elements <= std::numeric_limits<int>::max());
    return static_cast<int>(n_elements);
}

inline void fp_chunk::add_content_expr(i_fp_expr_content* expr)
{
    _content_exprs.emplace_back(expr);
}

inline const i_fp_expr_content& fp_chunk::get_content_expr(int idx) const
{
    assert(idx < n_content_exprs());
    return *(_content_exprs[idx]);
}

inline void fp_chunk::clear_content_exprs()
{
    _content_exprs.clear();
}

inline int fp_chunk::n_command_exprs() const
{
    const size_t n_elements = _command_exprs.size();
    THROW_ASSERT(0 <= n_elements && n_elements <= std::numeric_limits<int>::max());
    return static_cast<int>(n_elements);
}

inline void fp_chunk::add_command_expr(i_fp_expr_command* expr)
{
    _command_exprs.emplace_back(expr);
}

inline const i_fp_expr_command& fp_chunk::get_command_expr(int idx) const
{
    assert(idx < n_command_exprs());
    return *(_command_exprs[idx]);
}

inline void fp_chunk::clear_command_exprs()
{
    _command_exprs.clear();
}

inline void fp_chunk::set_version_string(const std::string& version_string)
{
    _version_string = version_string;
}

inline void fp_chunk::clear_version_string()
{
    _version_string.reset();
}

inline bool fp_chunk::has_version_string() const
{
    return _version_string.has_value();
}

inline const std::optional<std::string>& fp_chunk::get_version_string() const
{
    return _version_string;
}

//////////////////////////////////////// class fp_visito
class fp_visitor_print: i_fp_visitor
{
public:
    fp_visitor_print();

    void visit(const fp_chunk& chunk, int case_idx) override;
    void visit(const fp_expr_title& expr) override;
    void visit(const fp_expr_game& expr) override;
    void visit(const fp_expr_comment& expr) override;
    void visit(const fp_expr_command_solve_bw& expr) override;
    void visit(const fp_expr_command_solve_n& expr) override;

private:
};

inline fp_visitor_print::fp_visitor_print()
{
}

inline void fp_visitor_print::visit(const fp_chunk& chunk, int case_idx)
{
    THROW_ASSERT(case_idx < chunk.n_command_exprs());

    std::cout << "VISITING CHUNK" << std::endl;

    {
        std::cout << "CHUNK CONTENT EXPRS:" << std::endl;

        const int n_content_exprs = chunk.n_content_exprs();
        for (int i = 0; i < n_content_exprs; i++)
            chunk.get_content_expr(i).accept(*this);
    }

    {
        std::cout << "CHUNK COMMAND EXPR:" << std::endl;
        chunk.get_command_expr(case_idx).accept(*this);
    }
}

inline void fp_visitor_print::visit(const fp_expr_title& expr)
{
    std::cout << "|" << expr.get_line_no() << "| ";
    std::cout << "TITLE: " << expr.get_title() << std::endl;
}

inline void fp_visitor_print::visit(const fp_expr_game& expr)
{
    std::cout << "|" << expr.get_line_no() << "| ";
    std::cout << "GAME (BRACKETED: " << expr.is_bracketed() << ") \"";
    std::cout << expr.get_game_token() << "\"" << std::endl;
}

inline void fp_visitor_print::visit(const fp_expr_comment& expr)
{
    std::cout << "|" << expr.get_line_no() << "| ";
    std::cout << "COMMENT (TYPE: " << expr.get_comment_type();
    std::cout << " NUMBER: ";
    if (expr.get_comment_type() == FP_EXPR_COMMENT_TYPE_NUMBERED)
        std::cout << expr.get_number();
    else
        std::cout << "?";
    std::cout << ") \"" << expr.get_comment() << "\"" << std::endl;
}

inline void fp_visitor_print::visit(const fp_expr_command_solve_bw& expr)
{
    std::cout << "|" << expr.get_line_no() << "| ";
    std::cout << "SOLVE BW: ";
    std::cout << color_to_player_char(expr.get_player()) << " ";
    const minimax_outcome_enum expected_outcome = expr.get_expected_outcome();

    switch (expected_outcome)
    {
        case MINIMAX_OUTCOME_NONE:
        {
            std::cout << "?";
            break;
        }

        case MINIMAX_OUTCOME_WIN:
        {
            std::cout << "WIN";
            break;
        }

        case MINIMAX_OUTCOME_LOSS:
        {
            std::cout << "LOSS";
            break;
        }
    }

    std::cout << std::endl;
}

inline void fp_visitor_print::visit(const fp_expr_command_solve_n& expr)
{
    std::cout << "|" << expr.get_line_no() << "| ";

    std::cout << "SOLVE N: ";
    const std::optional<int>& expected_nim_value = expr.get_expected_nim_value();

    if (expected_nim_value.has_value())
        std::cout << expected_nim_value.value();
    else
        std::cout << "?";

    std::cout << std::endl;
}

//////////////////////////////////////////////////
void test_file_parser_new_stuff();
