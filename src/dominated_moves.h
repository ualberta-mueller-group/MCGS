/*
    Class dominated_moves_t represents dominated moves for a sum. It's used
    as a field in the db_entry_partisan struct.

    Query/add moves with:
        - Subgame hash
        - (DB encoded) move
        - Player to play

    TODO: review inlining
*/
#pragma once

#include <set>
#include <map>

#include "cgt_basics.h"
#include "hashing.h"
#include "game.h"
#include "serializer.h"

////////////////////////////////////////////////// class dominated_moves_t
class dominated_moves_t
{
public:
    // Map subgame hash to a set of its (DB encoded) dominated moves
    typedef std::map<hash_t, std::set<move>> move_map_t;

    bool move_is_dominated(hash_t subgame_hash, move move_db_encoded,
                           bw player) const;

    void add_move(hash_t subgame_hash, move move_db_encoded, bw player);

    // Used in testing. Unlikely to be useful elsewhere
    const move_map_t& get_black_move_map() const;
    const move_map_t& get_white_move_map() const;

    bool operator==(const dominated_moves_t& rhs) const;
    bool operator!=(const dominated_moves_t& rhs) const;

private:
    // May be nullptr
    const std::set<move>* _get_set_if_exists(hash_t subgame_hash,
                                             bw player) const;
    std::set<move>& _get_or_create_set(hash_t subgame_hash, bw player);

    friend struct serializer<dominated_moves_t*>;

    move_map_t _black_moves;
    move_map_t _white_moves;
};

////////////////////////////////////////////////// dominated_moves_t methods
inline bool dominated_moves_t::move_is_dominated(hash_t subgame_hash,
                                                 move move_db_encoded,
                                                 bw player) const
{
    assert(is_black_white(player));

    const std::set<move>* move_set = _get_set_if_exists(subgame_hash, player);

    if (move_set == nullptr)
        return false;

    return move_set->find(move_db_encoded) != move_set->end();
}

inline void dominated_moves_t::add_move(hash_t subgame_hash,
                                        move move_db_encoded, bw player)
{
    assert(is_black_white(player));
    std::set<move>& move_set = _get_or_create_set(subgame_hash, player);
    move_set.insert(move_db_encoded);
}

inline const dominated_moves_t::move_map_t& dominated_moves_t::
    get_black_move_map() const
{
    return _black_moves;
}

inline const dominated_moves_t::move_map_t& dominated_moves_t::
    get_white_move_map() const
{
    return _white_moves;
}

inline bool dominated_moves_t::operator==(const dominated_moves_t& rhs) const
{
    return (_black_moves == rhs._black_moves) && //
           (_white_moves == rhs._white_moves);   //
}

inline bool dominated_moves_t::operator!=(const dominated_moves_t& rhs) const
{
    return !(*this == rhs);
}

inline const std::set<move>* dominated_moves_t::_get_set_if_exists(
    hash_t subgame_hash, bw player) const
{
    assert(is_black_white(player));

    const move_map_t& move_map =
        (player == BLACK) ? _black_moves : _white_moves;

    auto result = move_map.find(subgame_hash);

    if (result == move_map.end())
        return nullptr;

    return &result->second;
}

inline std::set<move>& dominated_moves_t::_get_or_create_set(
    hash_t subgame_hash, bw player)
{
    assert(is_black_white(player));
    move_map_t& move_map = (player == BLACK) ? _black_moves : _white_moves;
    return move_map[subgame_hash];
}

////////////////////////////////////////////////// serializer<dominated_moves_t>
template <>
struct serializer<dominated_moves_t*>
{
#warning TODO: dominated_moves_t serializer not portable due to `move`

    /*
        iobuffer.h should implement:
            write_integral_as<Int1, Int2>(...)
            read_integral_as<Int1, Int2>(...)

        and do runtime checks
        - Maybe modify and use n_bit_int.h
        - Or define `move` as `int32_t`?
    */

    inline static void save(obuffer& os, const dominated_moves_t* dm)
    {
        serializer_save(os, dm->_black_moves);
        serializer_save(os, dm->_white_moves);
    }

    inline static dominated_moves_t* load(ibuffer& is)
    {
        dominated_moves_t* dm = new dominated_moves_t();

        serializer_load(is, dm->_black_moves);
        serializer_load(is, dm->_white_moves);

        return dm;
    }
};
