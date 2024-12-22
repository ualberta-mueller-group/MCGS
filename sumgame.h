//---------------------------------------------------------------------------
// Sum of combinatorial games and solving algorithms
//---------------------------------------------------------------------------
#ifndef sumgame_H
#define sumgame_H

#include "alternating_move_game.h"
#include "cgt_move.h"
#include "game.h"

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
    vector<game const*> new_games; // doesn't own games, just stores them for debugging

};


// TODO dummy since alternating_move_game requires a game argument
class empty_game : public game
{
public:
    empty_game() : game() { }
    void play(const move& m, bw to_play) {}
    void undo_move() {}
    void print(std::ostream& str) const {}
    game* inverse() const
    { assert(false); return 0; }
    move_generator* create_move_generator(bw to_play) const
    { assert(false); return 0; }
};

class sumgame_move_generator;
struct play_record;

class sumgame : public alternating_move_game
{
public:
    sumgame(bw color);
    ~sumgame();
    
    void play_sum(const sumgame_move& m, bw to_play);
    void undo_move();
    void add(game* g);
    bool solve() const;




    const play_record& last_play_record() const;
    int num_total_games() const;
    int num_active_games() const;
    const game* subgame_const(int i) const {return _subgames[i]; }
    game* subgame(int i) const {return _subgames[i]; }
    sumgame_move_generator* create_sum_move_generator(bw to_play) const;
    void print(std::ostream& str) const;
private:
    bool _solve();
    empty_game _empty_game;
    vector<game*> _subgames; // sumgame owns these subgames
    vector<play_record> _play_record_stack;
};
//---------------------------------------------------------------------------

inline sumgame::sumgame(bw color) :
    alternating_move_game(_empty_game, color),
    _empty_game(),
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

#endif // sumgame_H

