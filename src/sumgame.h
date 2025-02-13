//---------------------------------------------------------------------------
// Sum of combinatorial games and solving algorithms
//---------------------------------------------------------------------------
#pragma once

#include "alternating_move_game.h"
#include "cgt_move.h"
#include "game.h"
#include <chrono>
#include <ctime>
#include <limits>

struct sumgame_move
{
    sumgame_move(int subg, move m) : _subgame_idx(subg), _move(m) { }

    int _subgame_idx;
    move _move;
};

struct play_record
{
    play_record(sumgame_move move) :
        did_split(false), move(move), new_games()
    { }

    inline void add_game(game* game) { new_games.push_back(game); }

    bool did_split;
    sumgame_move move;
    std::vector<game const*> new_games; // doesn't own games, just stores them for debugging

};

class sumgame_move_generator;
struct play_record;

struct solve_result
{
    bool win;

    solve_result() = delete;

    solve_result(bool win) : win(win)
    { }

    // return this on timeout
    inline static std::optional<solve_result> invalid()
    {
        return std::optional<solve_result>();
    }
};

class sumgame : public alternating_move_game
{
public:
    sumgame(bw color);
    ~sumgame();
    
    void play_sum(const sumgame_move& m, bw to_play);
    void undo_move();
    void add(game* g);
    void add_vec(std::vector<game*>& gs);

    bool solve() const;

    /*
        Timeout is in milliseconds. 0 means never timeout.

        On timeout, the returned optional has no value
    */
    std::optional<solve_result> solve_with_timeout(unsigned long long timeout) const;

    const play_record& last_play_record() const;
    int num_total_games() const;
    int num_active_games() const;
    const game* subgame_const(int i) const {return _subgames[i]; }
    game* subgame(int i) const {return _subgames[i]; }
    const std::vector<game*>& subgames() const { return _subgames; }
    sumgame_move_generator* create_sum_move_generator(bw to_play) const;
    void print(std::ostream& str) const;
private:
    bool over_time() const;

    /*
        mutable makes sense here? 

        these values are used by _solve_with_timeout() and are here so they don't
            need to be passed as arguments to the function every time it's called
    */
    mutable bool should_stop;
    std::optional<solve_result> _solve_with_timeout();

    std::vector<game*> _subgames; // sumgame owns these subgames
    std::vector<play_record> _play_record_stack;
};
//---------------------------------------------------------------------------

inline sumgame::sumgame(bw color) :
    alternating_move_game(color),
    _subgames(),
    _play_record_stack()
{ }

inline const play_record& sumgame::last_play_record() const
{
    return _play_record_stack.back();
}

inline int sumgame::num_total_games() const
{
    return static_cast<int>(_subgames.size());
}

inline int sumgame::num_active_games() const
{
    int active = 0;
    for (const game* g: _subgames)
        if (g->is_active())
            ++active;
    return active;
}

//---------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const sumgame& s);

//---------------------------------------------------------------------------

