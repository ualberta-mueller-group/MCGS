//---------------------------------------------------------------------------
// Combinatorial game and move generator
//---------------------------------------------------------------------------

#pragma once

#include <iostream>
#include <type_traits>
#include <vector>
#include <optional>
#include "cgt_basics.h"
#include "cgt_move.h"
#include "game_type.h"

//---------------------------------------------------------------------------

class move_generator;
class game;
//---------------------------------------------------------------------------
typedef std::optional<std::vector<game*>> split_result;

class game : public i_game_type
{
public:
    game();

    virtual ~game() {}

    bool is_active() const;
    void set_active(bool status);
    move last_move() const;
    // Used to verify that game is restored after search
    int game_hash() const; // TODO placeholder - do a proper implementation
    bool has_moves() const;
    int num_moves_played() const;

    /*
        When overriding play() and undo_move(), the derived type's
        function should first call these functions.

        i.e. in derived type's play():
            // pushes move onto move stack for later,
            // after encoding player color into it
            game::play(m, to_play);

        and in derived type's undo_move():
            // get back the move we remembered
            move m_encoded = game::last_move();
            game::undo_move(); // pop it off the stack
            // now decode the move
            bw to_play = cgt_move::get_color(m_encoded);
            move m = cgt_move::decode(m_encoded);

    */
    virtual void play(const move& m, bw to_play);
    virtual void undo_move();

    // calls _split_implementation() and filters out games having no moves
    split_result split() const;

protected:
    /*
        Return list of games to replace current game. Empty list means game is
       0. No value means split didn't occur. See std::optional. The games within
       the list should be new objects, and not a pointer to the game object
       returning the list.

        In the case where a game doesn't split into subgames, return an absent
       split result.

        The returned games are owned by the caller.

        To create an empty but present split_result:
            split_result sr = split_result(vector<game*>());

        To create an absent split_result:
            split_result sr = split_result();
    */
    virtual split_result _split_implementation() const;

public:
    virtual move_generator* create_move_generator(bw to_play) const = 0;

    /*
        Print a string representation for a game. This should include a name
            to differentiate the game from other types of games.
    */
    virtual void print(std::ostream& str) const = 0;

    /*
        Return a new game representing the inverse of this game.
            i.e. for the game "4", this should return the game "-4"
    */
    virtual game* inverse() const = 0; // caller takes ownership

private:
    std::vector<move> _move_stack;
    bool _is_active;
}; // class game

inline game::game() : _move_stack(), _is_active(true)
{
}

inline bool game::is_active() const
{
    return _is_active;
}

inline void game::set_active(bool status)
{
    _is_active = status;
}

inline move game::last_move() const
{
    return _move_stack.back();
}

inline void game::play(const move& m, int to_play)
{
    assert(cgt_move::get_color(m) == 0);
    const move mc = cgt_move::encode(m, to_play);
    _move_stack.push_back(mc);
    //     std::cout << "move "<< cgt_move::print(m) << "\n";
    //     std::cout << "move + color "<< cgt_move::print(mc) << std::endl;
}

inline void game::undo_move()
{
    _move_stack.pop_back();
}

inline split_result game::split() const
{
    split_result sr = _split_implementation();

    // no split happened
    if (!sr)
    {
        return sr;
    }

    // filter games
    split_result result = split_result(std::vector<game*>());

    for (game* g : *sr)
    {
        assert(g != this);
        if (g->has_moves())
            result->push_back(g);
        else
            delete g;
    }

    return result;
}

inline split_result game::_split_implementation() const
{
    return split_result(); // no value
}

inline int game::num_moves_played() const
{
    return _move_stack.size();
}

inline int game::game_hash() const
{
    return num_moves_played(); // TODO placeholder only
}

inline std::ostream& operator<<(std::ostream& out, const game& g)
{
    g.print(out);
    return out;
}

//---------------------------------------------------------------------------
class move_generator
{
public:
    move_generator(bw to_play);

    virtual ~move_generator() {}

    int to_play() const { return _to_play; }

    int opponent() const { return ::opponent(_to_play); }

    virtual void operator++() = 0;
    virtual operator bool() const = 0;
    virtual move gen_move() const = 0;

private:
    const bw _to_play;
}; // class move_generator

inline move_generator::move_generator(bw to_play) : _to_play(to_play)
{
}

//---------------------------------------------------------------------------

class assert_restore_game
{
public:
    assert_restore_game(const game& g);
    ~assert_restore_game();

private:
    const game& _game;
    const int _game_hash;
};
//---------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const split_result& split);

template <class T> // NOLINTNEXTLINE(readability-identifier-naming)
constexpr bool is_concrete_game_v =
    !std::is_abstract_v<T> && std::is_base_of_v<game, T>;

/*
    Convert game pointer types, after doing a few asserts.

    NOTE: This cast is generally unsafe. Only use in place of reinterpret_cast,
   not dynamic_cast
*/
template <class T_Ptr>
inline T_Ptr cast_game(game* g)
{
    static_assert(std::is_pointer_v<T_Ptr>);
    // NOLINTNEXTLINE(readability-identifier-naming)
    using T = typename std::remove_pointer<T_Ptr>::type;

    static_assert(is_concrete_game_v<T>);

    assert(g != nullptr);
    assert(g->is_active());
    assert(g->game_type() == game_type<T>());

    return reinterpret_cast<T_Ptr>(g);
}
