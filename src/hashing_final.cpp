#include "hashing_final.h"
#include <iostream>

////////////////////////////////////////////////// design

/*
hash_t
    Typedef of uint64_t

random_table
    Singleton. Shared by all games? Still make this a class so it's easy to
    make multiple random tables later.

    // do any necessary rotations, wrap computations, and combining etc.
    hash_t get<T>(const size_t& position, const T& color) const;

    vector<hash_t> _number_table;

local_hash
    Just a zobrist hash for now.

    void reset();
    hash_t get_value() const;

    void toggle_tile<T>(const size_t& position, const T& color);
    // TODO: do we need an alternative method for basic CGT games? Maybe this is sufficient?


    hash_t _value;

game
    local_hash _hash;
    bool _hash_valid; // private

    void _mark_hash_valid();
    void _mark_hash_invalid();

    const local_hash& get_hash() const;

    // Instead of virtual play() and undo_move() methods, have:
    void play(const move& m, bw to_play);
    void undo_move();

    virtual void _play_impl(const move& m, bw to_play);
    virtual void _undo_impl(const move& m, bw to_play);

    virtual bool update_hash_init(local_hash& hash) = 0;
    virtual bool update_hash_play(local_hash& hash, const move& m, const bw& to_play)
    { return false; }
    virtual bool update_hash_undo(local_hash& hash, const move& m, const bw& to_play);
    { return false; }

    // When calling game::play()
        // 1: game::play()
        // 2: If _hash_valid: _hash_valid = game::update_hash_play()
        // 3: game::_play_impl()

    // When calling game::undo()
        // 1: game::undo()
        // 2: If _hash_valid: _hash_valid = game::update_hash_undo()
        // 3: game::_undo_impl()

    // When calling get_hash()
        // 1: get_hash()
        // 2: if !_hash_valid: hash_update_init()
        // 3: return _hash, or throw, or do something...

global_hash
    Owned by a sumgame

--------------------

Possibly missing: LCG

--------------------

Are `game`s hash hooks optional in some sense?
    -- Can they be totally absent?
        -- Make them return bools, have default implementation; returns false.
           If a game doesn't implement them, do the correct thing.
    -- Can they immediately return, and have their functionality be implemented
       by _play_impl() and _undo_impl()?

    SOLUTION: Make the hooks virtual, give default implementation which returns false.
              Throw when get_hash() can't produce a valid hash. We can check which
              games need implementations by removing the default implementation.
              Make hash_update_init() mandatory (just make it pure abstract)

              At some point hashes may be optional, and we can deal with this
              later

              Maybe the other hooks get removed later and we only keep the
              hash_update_init() hook?

How does global_hash know about the local_hashes?

_xyz_implementation() --> _xyz_impl() ???

A game's hash starts as invalid. play() and undo_move() will call their respective
update_hash hooks IF the hash is valid. get_hash() will also call init_hash() if not valid.


*/



////////////////////////////////////////////////// .h files

////////////////////////////////////////////////// .cpp files

////////////////////////////////////////////////// "main"


using namespace std;

void test_hashing_final()
{
    cout << "TEST FINAL" << endl;
}
