/*
    TODO remaining tasks:

    0. Enumerate all classes and function signatures before writing any more
       code...

    1. Make a file_parser2 class (copy existing code and rename it).
       Make it maintain an fp_chunk:

       bool file_parser2::parse_chunk();

       int file_parser2::n_test_cases() const;
       csv_row file_parser2::run_test_case(int idx) const; // don't implement
       vector<game*> file_parser2::get_games() const;

       const fp_chunk& file_parser2::get_fp_chunk() const;

       Implement everything except run_test_case(). The new parse_chunk() is
       a bit awkward, but it's not possible to know if there's a chunk to
       be read without trying to read it, which either results in possibly
       blocking, or more complex code...

       Allow empty command braces

    3. Add a csv_row struct/class. Wrap some fields in std::optional, and have
       a `bool is_complete` check to ensure mandatory fields are filled in?
       Add utility functions for fields that are annoying to populate, i.e.
       time, game string, hash, etc

    4. Make file_parser2::run_test_case() call one of the utility functions
       implementing the actual solving/winning move enumeration


    NOTES:
    - Empty command braces should be allowed. --play-mcgs doesn't care about the
      actual command, just the games.

    - When adding the winning moves test, remove --print-winning-moves.
      Instead make this a command: "{B winning moves?}" to print, and
      "{B winning moves move1, move2, move3...}" to specify moves

    - file_parser2 should use several visitors to implement these methods:

      csv_row visitor_run_test::visit(const fp_chunk&, int test_idx) const;
      vector<game*> visitor_get_games::visit(const fp_chunk&) const;

      Class visitor_run_test should gather both the comments and games,
      then call the correct utility function to generate the csv_row.

      Class visitor_get_games should just return the games

      How to copy chunks to a file?

      // -1 test_idx will print command "{}"
      optional<string> visitor_print_test(const fp_chunk&, int test_idx, const
          optional<string>&, ostream&) const;

*/

/*
   prototyping for file parser refactor
*/
#pragma once

#include "cgt_basics.h"
#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <optional>
#include "test_case_enums.h"

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
class fp_expr_command_winning_moves;

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

    virtual void visit(const fp_expr_title& expr) = 0;
    virtual void visit(const fp_expr_game& expr) = 0;
    virtual void visit(const fp_expr_comment& expr) = 0;
    virtual void visit(const fp_expr_command_solve_bw& expr) = 0;
    virtual void visit(const fp_expr_command_solve_n& expr) = 0;
    virtual void visit(const fp_expr_command_winning_moves& expr) = 0;

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

//////////////////////////////////////// interface i_fp_expr_content
class i_fp_expr_content: public i_fp_expr
{
public:
    i_fp_expr_content(int line_no);

private:
};

//////////////////////////////////////// interface i_fp_expr_command
class i_fp_expr_command: public i_fp_expr
{
public:
    i_fp_expr_command(int line_no, command_type_enum command_type);

    command_type_enum get_command_type() const;

private:
    const command_type_enum _command_type;
};

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
    std::string _comment;
    fp_expr_comment_type _comment_type;
    int _number;
};

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

//////////////////////////////////////////////////
// class fp_expr_command_winning_moves

class fp_expr_command_winning_moves: public i_fp_expr_command
{
public:
    fp_expr_command_winning_moves(
        int line_no, ebw player,
        std::optional<std::vector<std::string>> expected_winning_moves);

    void accept(i_fp_visitor& visitor) const override;

    ebw get_player() const;
    const std::optional<std::vector<std::string>>& get_expected_winning_moves()
        const;


private:
    const ebw _player;
    const std::optional<std::vector<std::string>> _expected_winning_moves;
};

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


//////////////////////////////////////// class fp_visitor_print
class fp_visitor_print: i_fp_visitor
{
public:
    fp_visitor_print();

    void visit_chunk(const fp_chunk& chunk);

    void visit(const fp_expr_title& expr) override;
    void visit(const fp_expr_game& expr) override;
    void visit(const fp_expr_comment& expr) override;
    void visit(const fp_expr_command_solve_bw& expr) override;
    void visit(const fp_expr_command_solve_n& expr) override;
    void visit(const fp_expr_command_winning_moves& expr) override;

private:
};


//////////////////////////////////////////////////
void test_file_parser_new_stuff();
