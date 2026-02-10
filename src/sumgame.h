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

#include "ThValue.h"
#include "alternating_move_game.h"
#include "cgt_move.h"
#include "game.h"
#include "global_options.h"
#include "sumgame_change_record.h"
#include "transposition.h"
#include "timeout_token.h"

typedef std::optional<std::vector<std::optional<ThValue>>> temp_vec_opt_t;

struct ttable_sumgame_entry
{
};

typedef ttable<ttable_sumgame_entry> ttable_sumgame;

//////////////////////////////////////// forward declarations
class sumgame_move_generator;
class assert_restore_sumgame;

namespace sumgame_impl {
class change_record;
}

// Used by class sumgame::undo_stack_unwinder
enum sumgame_undo_code
{
    SUMGAME_UNDO_STACK_FRAME = 0,
    SUMGAME_UNDO_SIMPLIFY_BASIC,
    SUMGAME_UNDO_PLAY,
    SUMGAME_UNDO_SIMPLIFY_DB,
    SUMGAME_UNDO_SIMPLIFY_IMPARTIAL,
    SUMGAME_UNDO_PRE_SOLVE_PASS,
    SUMGAME_UNDO_SPLIT_AND_NORMALIZE,
    SUMGAME_UNDO_DB_REPLACEMENT_PASS,
    SUMGAME_UNDO_DB_ANALYZE_PASS,

};

//////////////////////////////////////// sumgame_move
struct sumgame_move
{
    sumgame_move() {} // TODO remove this?
    sumgame_move(int subg, move m) : subgame_idx(subg), m(m) {}

    int subgame_idx;
    move m;
};

//////////////////////////////////////// play_record
struct play_record
{
    play_record(sumgame_move sm)
        : did_split(false), sm(sm), new_games(), deactivated_g(false)
    {
    }

    inline void add_game(game* game) { new_games.push_back(game); }

    bool did_split;
    sumgame_move sm;
    // doesn't own games, just stores them for debugging
    std::vector<game const*> new_games;
    bool deactivated_g;
};


//////////////////////////////////////// solve_result
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

//////////////////////////////////////// sumgame
class sumgame : public alternating_move_game
{
public:
    sumgame(bw color);
    virtual ~sumgame();

    void play_sum(const sumgame_move& m, bw to_play);
    void undo_move() override;
    void simplify_basic();
    void undo_simplify_basic();

    void simplify_impartial();
    void undo_simplify_impartial();

    std::optional<solve_result> simplify_db(
        temp_vec_opt_t& temperatures);
    void undo_simplify_db();

    //////////////////////////////////////////////////
    //////////////////////////////////////////////////
    void db_replacement_pass();
    void undo_db_replacement_pass();

    std::optional<solve_result> db_analyze_pass();
    void undo_db_analyze_pass();
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////

    void add(game* g);
    void add(std::vector<game*>& gs);
    void pop(const game* g);
    void pop(const std::vector<game*>& gs);

    bool solve() const override;

    /*
        Timeout is in milliseconds. 0 means never timeout

        On timeout, the returned optional has no value
    */
    std::optional<solve_result> solve_with_timeout(
        unsigned long long timeout) const;

    std::optional<solve_result> solve_with_timeout_token(
        const timeout_token& timeout_tok, uint64_t depth) const;

    bool solve_with_games(std::vector<game*>& gs) const;
    bool solve_with_games(game* g) const;

    int num_total_games() const;
    int num_active_games() const;
    bool is_empty() const; // no active games, game over

    const game* subgame_const(int i) const { return _subgames[i]; }

    game* subgame(int i) const { return _subgames[i]; }

    const std::vector<game*>& subgames() const { return _subgames; }

    sumgame_move_generator* create_sum_move_generator(bw to_play) const;

    // Resizes temperature vector
    sumgame_move_generator* create_sum_move_generator(
        bw to_play, temp_vec_opt_t& temperatures) const;

    void print(std::ostream& str) const;

    hash_t get_global_hash(bool invalidate_game_hashes = false) const;
    hash_t get_global_hash_for_player(ebw for_player, bool invalidate_game_hashes = false) const;

    // TODO this function is strange, as alternating_move_game could have a
    // game...
    hash_t game_hash() const override;

    bool all_impartial() const; // considers inactive games

    // Used by player
    std::optional<sumgame_move> get_winning_or_random_move(bw for_player) const;

    // called by mcgs_init_all()
    static void init_sumgame(size_t index_bits);

    // Called by derived classes of i_test_case, in their _run_impl() methods
    static void clear_ttable();

    void split_and_normalize(); // for DB
    void undo_split_and_normalize(); // for DB

private:
    class undo_stack_unwinder;

    void _pre_solve_pass();
    void _undo_pre_solve_pass();

    bool _over_time() const;
    game* _pop_game();

    /*
        Main search logic. Respects timeout defined by sumgame::_over_time(),
        which uses a (possibly absent) timeout_token. If not present, search
        never times out.
    */
    std::optional<solve_result> _solve_impl(uint64_t depth);

    void _push_undo_code(sumgame_undo_code code);
    void _pop_undo_code(sumgame_undo_code code);

    std::optional<ttable_sumgame::search_result> _do_ttable_lookup() const;

    void _debug_extra() const;
    void _assert_games_unique() const;

    std::optional<timeout_token> _timeout_tok;
    mutable bool _need_cgt_simplify;
    mutable global_hash _sumgame_hash;
    std::vector<game*> _subgames;

    std::vector<sumgame_undo_code> _undo_code_stack;
    std::vector<play_record> _play_record_stack;
    std::vector<sumgame_impl::change_record> _change_record_stack;

    static std::shared_ptr<ttable_sumgame> _tt;

    friend class assert_restore_sumgame;
};


/*
    TODO: Should sumgame_move_generator be semi-hidden in a namespace? Or leave
    it public? Users of sumgame can't use the normal move_generator interface...
*/

////////////////////////////////////////////////// Move generators
#define SUMGAME_MOVE_GENERATOR_1_6

//////////////////////////////////////// sumgame_move_generator (i.e. v1.5)
#ifndef SUMGAME_MOVE_GENERATOR_1_6

class sumgame_move_generator : public move_generator
{
public:
    sumgame_move_generator(const sumgame& game, bw to_play);
    ~sumgame_move_generator();

    void operator++() override;

    // TODO make private
    void next_move(bool init);

    operator bool() const override;
    sumgame_move gen_sum_move() const;

    move gen_move() const override { assert(false); }

private:
    std::pair<int, const game*> _current() const;

    const sumgame& _game;
    const int _num_subgames;

    std::vector<std::pair<int, const game*>> _skipped_games;
    bool _use_skipped_games;

    std::set<hash_t> _seen_games;

    /*
       When _use_skipped_games is true, this is an index into our list of
       skipped games, and not the sumgame
    */
    int _subgame_idx;

    move_generator* _subgame_generator;
};

inline std::pair<int, const game*> sumgame_move_generator::_current() const
{
    if (_use_skipped_games)
        return _skipped_games[_subgame_idx];
    else
        return {_subgame_idx, _game.subgame(_subgame_idx)};
}

//////////////////////////////////////// sumgame_move_generator (v1.6 changes)
#else
class sumgame_move_generator : public move_generator
{
public:
    sumgame_move_generator(const sumgame& sum, bw to_play,
                           const temp_vec_opt_t& temperatures = {});

    ~sumgame_move_generator();

    void operator++() override;
    operator bool() const override;

    sumgame_move gen_sum_move() const;

    move gen_move() const override { assert(false); }

private:
    void _increment(bool init);

    // Index into _subgames, not subgame index in _sum
    size_t _subgame_idx_local;
    std::vector<std::pair<int, const game*>> _subgames;

    std::unique_ptr<move_generator> _mg;
    std::set<hash_t> _seen_games;

    const sumgame& _sum;
};


#endif
//////////////////////////////////////////////////

//---------------------------------------------------------------------------

inline sumgame::sumgame(bw color) : alternating_move_game(color), _subgames()
{
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

inline hash_t sumgame::game_hash() const
{
    return get_global_hash();
}

inline void sumgame::clear_ttable()
{
    assert(global::clear_tt());

    if (_tt.get() == nullptr)
    {
        assert(global::tt_sumgame_idx_bits() == 0);
        return;
    }

    _tt->clear();
}

//---------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const sumgame& s);

//---------------------------------------------------------------------------
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

//---------------------------------------------------------------------------
