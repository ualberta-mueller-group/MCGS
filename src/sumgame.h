//---------------------------------------------------------------------------
// Sum of combinatorial games and solving algorithms
//---------------------------------------------------------------------------
#pragma once

#include <ctime>
#include <memory>
#include <vector>
#include <set>
#include <optional>
#include <cstdint>
#include <utility>
#include <ostream>
#include <cassert>

#include "alternating_move_game.h"
#include "game.h"
#include "sumgame_change_record.h"
#include "dominated_moves.h"
#include "transposition.h"
#include "timeout_token.h"
#include "ThValue.h"

////////////////////////////////////////////////// Forward declarations
namespace sumgame_impl {
class change_record;
}

class sumgame_move_generator;

class assert_restore_sumgame;

////////////////////////////////////////////////// Simple types
typedef std::vector<std::optional<ThValue>> temperature_vec_t;
typedef std::vector<std::shared_ptr<const db_dom_moves_t>> dom_object_vec_t;


struct ttable_sumgame_entry
{
};

typedef ttable<ttable_sumgame_entry> ttable_sumgame;

enum sumgame_undo_code
{
    SUMGAME_UNDO_STACK_FRAME = 0,
    SUMGAME_UNDO_SIMPLIFY_BASIC,
    SUMGAME_UNDO_PLAY,
    SUMGAME_UNDO_DB_LOOKUP_PASS,
    SUMGAME_UNDO_PRE_SOLVE_PASS,
    SUMGAME_UNDO_SPLIT_AND_NORMALIZE,
    SUMGAME_UNDO_DB_REPLACEMENT_PASS,
};

////////////////////////////////////////////////// struct sumgame_move
struct sumgame_move
{
    sumgame_move() {} // TODO remove this?
    sumgame_move(int subg, move m) : subgame_idx(subg), m(m) {}

    bool operator==(const sumgame_move& rhs) const;
    bool operator!=(const sumgame_move& rhs) const;

    int subgame_idx;
    move m;
};

inline bool sumgame_move::operator==(const sumgame_move& rhs) const
{
    return (subgame_idx == rhs.subgame_idx) && (m == rhs.m);
}

inline bool sumgame_move::operator!=(const sumgame_move& rhs) const
{
    return !(*this == rhs);
}

////////////////////////////////////////////////// struct play_record
struct play_record
{
    play_record(sumgame_move sm) : sm(sm), deactivated_g(false), split_g(false)
    {
    }

    inline void add_game(game* game) { new_games.push_back(game); }

    sumgame_move sm;
    bool deactivated_g;
    bool split_g;
    // doesn't own games, just stores them for debugging
    std::vector<game const*> new_games;
};

////////////////////////////////////////////////// struct solve_result
struct solve_result
{
    solve_result() = delete;
    solve_result(bool win) : win(win) {}

    // return this on timeout
    inline static std::optional<solve_result> invalid()
    {
        return std::optional<solve_result>();
    }

    bool win;
};

////////////////////////////////////////////////// class sumgame
class sumgame : public alternating_move_game
{
public:
    sumgame(bw color);
    virtual ~sumgame();

    /*
        Basic mutators.

        The caller should `pop` and delete any games `add`ed.
    */
    void add(game* g);
    void add(const std::vector<game*>& games);

    void pop(const game* g);
    void pop(const std::vector<game*>& games);

    /*
        Basic accessors.
    */
    game* subgame(int i) const;
    const game* subgame_const(int i) const;
    const std::vector<game*>& subgames() const;

    /*
        Move playing.
    */
    sumgame_move_generator* create_sum_move_generator(bw to_play) const;
    
    void play_sum(const sumgame_move& m, bw to_play);
    void undo_move() override;

    /*
        Properties.

        TODO v1.7: implement `is_logically_empty()`, `is_physically_empty()`,
        etc.
    */
    int num_total_games() const;
    int num_active_games() const;

    bool is_empty() const; // no active games, game over

    bool all_impartial() const; // considers inactive games
    bool all_partisan() const; // considers inactive games

    hash_t get_global_hash(bool invalidate_game_hashes = false) const;
    hash_t get_global_hash_for_player(
        ebw for_player, bool invalidate_game_hashes = false) const;

    /*
        Printing.

        - `print` shows number of total and active games, and prints active
            games.
        - `print_simple` shows to_play, and active games.
        - `print_sorted` shows active games (sorted by hash).
    */
    void print(std::ostream& str) const;
    void print_simple(std::ostream& str) const;
    void print_sorted(std::ostream& str) const;

    /*
        Solving.

        Timeout is in milliseconds. 0 means never timeout. On timeout, the
        returned optional has no value.
    */
    bool solve() const override;

    bool solve_with_games(game* g) const;
    bool solve_with_games(const std::vector<game*>& games) const;

    std::optional<solve_result> solve_with_timeout(
        unsigned long long timeout) const;

    std::optional<solve_result> solve_with_timeout_token(
        const timeout_token& timeout_tok, uint64_t depth) const;

    /*
        Simplification.
    */
    void simplify_basic();
    void undo_simplify_basic();

    std::optional<solve_result> db_lookup_pass(temperature_vec_t& temperatures,
                                               dom_object_vec_t& dom_objects);
    void undo_db_lookup_pass();

    void split_and_normalize(); // for DB
    void undo_split_and_normalize(); // for DB

    void db_replacement_pass();
    void undo_db_replacement_pass();

    /*
        Utilities and static functions.
    */
    std::optional<sumgame_move> get_winning_or_random_move(bw for_player) const;

    // called by mcgs_init_all()
    static void init_sumgame(size_t index_bits);

    // Called by derived classes of i_test_case, in their _run_impl() methods
    static void clear_ttable();

    /*
        Deprecated functions.

        TODO: remove (artifact from alternating_move_game)
    */
    hash_t game_hash() const override;

private:
    /*
        Friends and typedefs.
    */
    class undo_stack_unwinder;
    friend class assert_restore_sumgame;

    /*
       Utilities.
    */
    bool _over_time() const;

    game* _pop_game();

    void _push_undo_code(sumgame_undo_code code);
    void _pop_undo_code(sumgame_undo_code code);

    void _pre_solve_pass();
    void _undo_pre_solve_pass();

    /*
        Search implementation, and other algorithms.

        `_solve_impl` Respects timeout defined by sumgame::_over_time(),
        which uses a (possibly absent) timeout_token. If not present, search
        never times out.
    */
    std::optional<solve_result> _solve_impl(uint64_t depth);

    std::optional<ttable_sumgame::search_result> _do_ttable_lookup() const;

    /*
        Debugging/asserts.
    */
    void _debug_extra() const;
    void _assert_games_unique() const;

    /*
        Transient data. Used during search and intermediate computations.
    */
    std::optional<timeout_token> _timeout_tok;
    mutable bool _need_cgt_simplify;
    mutable global_hash _sumgame_hash;

    /*
        Persistent data. Has meaning outside of search.
    */
    std::vector<game*> _subgames;

    std::vector<sumgame_undo_code> _undo_code_stack;
    std::vector<play_record> _play_record_stack;
    std::vector<sumgame_impl::change_record> _change_record_stack;

    static std::shared_ptr<ttable_sumgame> _tt;
};

// Calls `sumgame::print`
std::ostream& operator<<(std::ostream& out, const sumgame& s);

//////////////////////////////////////////////////
// class sumgame_move_generator
class sumgame_move_generator : public move_generator
{
public:
    /*
        Either/both pointers can be nullptr.

        When not nullptr, uses std::move to steal the contents of the vectors.
        Caller still owns the (now emptied) vectors.
    */
    sumgame_move_generator(const sumgame& sum, bw to_play,
                           temperature_vec_t* temperatures,
                           dom_object_vec_t* dom_move_objects);

    void operator++() override;
    operator bool() const override;

    sumgame_move gen_sum_move() const;

    move gen_move() const override { assert(false); }

private:
    void _increment(bool init);
    bool _increment_subgame(bool init);
    bool _increment_move(bool init);

    bool _should_skip_game(const game& g);

    move_generator* _make_subgame_move_generator(
        const game& g, const db_dom_moves_t* dom_moves_object) const;

    const sumgame& _sum;

    dom_object_vec_t _dom_objects;

    std::vector<std::pair<int, const game*>> _subgame_pairs;
    size_t _subgame_idx_local; // index into _subgame_pairs

    const game* _subgame_current;
    std::unique_ptr<move_generator> _mg_current;

    std::set<hash_t> _seen_local_hashes;
};

////////////////////////////////////////////////// class assert_restore_sumgame
#ifdef ASSERT_RESTORE_DEBUG
class assert_restore_sumgame : public assert_restore_alternating_game
{
public:
    assert_restore_sumgame(const sumgame& sgame);
    virtual ~assert_restore_sumgame();

private:
    const sumgame& _sgame;

    const hash_t _global_hash;
    const int _total_subgames;

    const size_t _undo_code_stack_size;
    const size_t _play_record_stack_size;
    const size_t _change_record_stack_size;
};

#else
class assert_restore_sumgame : public assert_restore_alternating_game
{
public:
    assert_restore_sumgame(const sumgame& sgame)
        : assert_restore_alternating_game(sgame)
    {
    }

    virtual ~assert_restore_sumgame() {}
};

#endif

////////////////////////////////////////////////// sumgame methods
inline sumgame::sumgame(bw color)
    : alternating_move_game(color), _need_cgt_simplify(true)
{
}

inline sumgame::~sumgame()
{
}

inline void sumgame::add(const std::vector<game*>& games)
{
    for (game* g : games)
        add(g);
}

inline void sumgame::pop(const game* g)
{
    assert(!_subgames.empty());
    assert(_subgames.back() == g);
    _subgames.pop_back();
}

inline void sumgame::pop(const std::vector<game*>& games)
{
    for (auto it = games.rbegin(); it != games.rend(); it++)
        pop(*it);
}

inline game* sumgame::subgame(int i) const
{
    return _subgames[i];
}

inline const game* sumgame::subgame_const(int i) const
{
    return _subgames[i];
}

inline const std::vector<game*>& sumgame::subgames() const
{
    return _subgames;
}

inline sumgame_move_generator* sumgame::create_sum_move_generator(bw to_play) const
{
    return new sumgame_move_generator(*this, to_play, nullptr, nullptr);
}

inline int sumgame::num_total_games() const
{
    return static_cast<int>(_subgames.size());
}

inline int sumgame::num_active_games() const
{
    int active = 0;
    for (const game* g : _subgames)
        if (g->is_active())
            ++active;
    return active;
}

inline bool sumgame::is_empty() const
{
    for (const game* g : _subgames)
        if (g->is_active())
            return false;
    return true;
}

inline bool sumgame::all_impartial() const
{
    return all_games_impartial(_subgames);
}

inline bool sumgame::all_partisan() const
{
    return all_games_partisan(_subgames);
}

inline hash_t sumgame::get_global_hash(bool invalidate_game_hashes) const
{
    return _sumgame_hash.get_global_hash_value(subgames(), to_play(),
                                               invalidate_game_hashes);
}

inline hash_t sumgame::get_global_hash_for_player(ebw for_player,
                                           bool invalidate_game_hashes) const
{
    assert(is_empty_black_white(for_player));
    return _sumgame_hash.get_global_hash_value(subgames(), for_player,
                                               invalidate_game_hashes);
}

inline hash_t sumgame::game_hash() const
{
    return get_global_hash();
}

inline bool sumgame::_over_time() const
{
    if (_timeout_tok.has_value())
        return _timeout_tok->stop_requested();
    return false;
}

inline game* sumgame::_pop_game()
{
    assert(!_subgames.empty());

    game* back = _subgames.back();
    _subgames.pop_back();

    return back;
}

inline void sumgame::_push_undo_code(sumgame_undo_code code)
{
    _undo_code_stack.push_back(code);
}

inline void sumgame::_pop_undo_code(sumgame_undo_code code)
{
    assert(!_undo_code_stack.empty());
    assert(_undo_code_stack.back() == code);
    _undo_code_stack.pop_back();
}

inline std::ostream& operator<<(std::ostream& out, const sumgame& s)
{
    s.print(out);
    return out;
}

//////////////////////////////////////////////////
// sumgame_move_generator methods
inline void sumgame_move_generator::operator++()
{
    assert(*this);
    _increment(false);
}

inline sumgame_move_generator::operator bool() const
{
    return _subgame_idx_local < _subgame_pairs.size();
}

