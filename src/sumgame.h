//---------------------------------------------------------------------------
// Sum of combinatorial games and solving algorithms
//---------------------------------------------------------------------------
#pragma once

#include "alternating_move_game.h"
#include "cgt_move.h"
#include "game.h"
#include <ctime>
#include "global_options.h"
#include "sumgame_change_record.h"
#include "transposition.h"
#include <memory>
#include <vector>
#include <set>
#include <optional>
#include <cstdint>
#include <utility>
#include <ostream>
#include <cassert>

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
    SUMGAME_UNDO_PRE_SOLVE_PASS,
};

//////////////////////////////////////// sumgame_move
struct sumgame_move
{
    sumgame_move(int subg, move m) : subgame_idx(subg), m(m) {}

    int subgame_idx;
    move m;
};

//////////////////////////////////////// play_record
struct play_record
{
    play_record(sumgame_move sm) : did_split(false), sm(sm), new_games() {}

    inline void add_game(game* game) { new_games.push_back(game); }

    bool did_split;
    sumgame_move sm;
    // doesn't own games, just stores them for debugging
    std::vector<game const*> new_games;
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

    std::optional<solve_result> simplify_db();
    void undo_simplify_db();

    void add(game* g);
    void add(std::vector<game*>& gs);
    void pop(const game* g);

    bool solve() const override;

    /*
        Timeout is in milliseconds. 0 means never timeout.

        On timeout, the returned optional has no value
    */
    std::optional<solve_result> solve_with_timeout(
        unsigned long long timeout) const;

    bool solve_with_games(std::vector<game*>& gs) const;
    bool solve_with_games(game* g) const;

    int num_total_games() const;
    int num_active_games() const;
    bool is_empty() const; // no active games, game over

    const game* subgame_const(int i) const { return _subgames[i]; }

    game* subgame(int i) const { return _subgames[i]; }

    const std::vector<game*>& subgames() const { return _subgames; }

    sumgame_move_generator* create_sum_move_generator(bw to_play) const;
    void print(std::ostream& str) const;

    hash_t get_global_hash(bool invalidate_game_hashes = false) const;

    // TODO this function is strange, as alternating_move_game could have a
    // game...
    hash_t game_hash() const override;

    bool all_impartial() const; // considers inactive games

    // called by mcgs_init()
    static void init_sumgame(size_t index_bits);
    static void reset_ttable();

private:
    class undo_stack_unwinder;

    void _pre_solve_pass();
    void _undo_pre_solve_pass();

    bool _over_time() const;
    game* _pop_game();
    std::optional<solve_result> _solve_with_timeout(uint64_t depth);
    void _push_undo_code(sumgame_undo_code code);
    void _pop_undo_code(sumgame_undo_code code);

    std::optional<ttable_sumgame::search_result> _do_ttable_lookup() const;

    void _debug_extra() const;
    void _assert_games_unique() const;

    mutable bool _should_stop;
    mutable bool _need_cgt_simplify;
    mutable global_hash _sumgame_hash;
    std::vector<game*> _subgames;

    std::vector<sumgame_undo_code> _undo_code_stack;
    std::vector<play_record> _play_record_stack;
    std::vector<sumgame_impl::change_record> _change_record_stack;

    static std::shared_ptr<ttable_sumgame> _tt;

    friend class assert_restore_sumgame;
};

//////////////////////////////////////// sumgame_move_generator
/*
    TODO: Should this be semi-hidden in a namespace? Or leave it public?
    Users of sumgame can't use the normal move_generator interface...
*/

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

inline void sumgame::reset_ttable()
{
    assert(global::clear_tt());

    if (_tt.get() == nullptr)
    {
        assert(global::tt_sumgame_idx_bits() == 0);
        return;
    }

    const size_t index_bits = _tt->n_index_bits();
    const size_t entry_bools = _tt->n_entry_bools();
    _tt.reset(new ttable_sumgame(index_bits, entry_bools));
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
