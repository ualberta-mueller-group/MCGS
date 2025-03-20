//---------------------------------------------------------------------------
// Sum of combinatorial games and solving algorithms
//---------------------------------------------------------------------------
#pragma once

#include "alternating_move_game.h"
#include "cgt_move.h"
#include "game.h"
#include <ctime>
#include "sumgame_change_record.h"

//////////////////////////////////////// forward declarations
class sumgame_move_generator;

namespace sumgame_impl {
class change_record;
}

enum sumgame_undo_code
{
    SUMGAME_UNDO_STACK_FRAME = 0,
    SUMGAME_UNDO_SIMPLIFY_BASIC,
    SUMGAME_UNDO_PLAY,
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
    std::vector<game const*>
        new_games; // doesn't own games, just stores them for debugging
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
    ~sumgame();

    // TODO should these be public?
    void play_sum(const sumgame_move& m, bw to_play);
    void undo_move() override;
    void simplify_basic();
    void undo_simplify_basic();

    void add(game* g);
    void add(std::vector<game*>& gs);
    void pop(game* g);

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

    const game* subgame_const(int i) const { return _subgames[i]; }

    game* subgame(int i) const { return _subgames[i]; }

    const std::vector<game*>& subgames() const { return _subgames; }

    sumgame_move_generator* create_sum_move_generator(bw to_play) const;
    void print(std::ostream& str) const;

private:
    class undo_stack_unwinder;

    bool _over_time() const;
    game* _pop_game();
    std::optional<solve_result> _solve_with_timeout();
    void _push_undo_code(sumgame_undo_code code);
    void _pop_undo_code(sumgame_undo_code code);

    void _debug_extra() const;
    void _assert_games_unique() const;

    mutable bool _should_stop;
    std::vector<game*> _subgames;

    std::vector<sumgame_undo_code> _undo_code_stack;
    std::vector<play_record> _play_record_stack;
    std::vector<sumgame_impl::change_record> _change_record_stack;
};

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

//---------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const sumgame& s);

//---------------------------------------------------------------------------
