#pragma once

#include <set>
#include <map>

#include "cgt_basics.h"
#include "sumgame.h"
#include "hashing.h"
#include "serializer.h"

////////////////////////////////////////////////// class dominated_moves_t
class dominated_moves_t
{
public:
    bool move_is_dominated(hash_t local_hash, move m, bw player) const
    {
        assert(is_black_white(player));

        const std::set<move>* move_set = _get_set(player, local_hash);

        if (move_set == nullptr)
            return false;

        return move_set->find(local_hash) != move_set->end();
    }

    void add_move(hash_t local_hash, move m, bw player)
    {
        assert(is_black_white(player));

        std::set<move>* move_set = _get_or_create_set(player, local_hash);
        assert(move_set != nullptr);

        move_set->insert(m);
    }

    bool operator==(const dominated_moves_t& rhs) const;
    bool operator!=(const dominated_moves_t& rhs) const;

private:
    const std::set<move>* _get_set(bw player, hash_t subgame_hash) const
    {
        assert(is_black_white(player));

        const move_map_t& move_map = (player == BLACK) ? _black_moves : _white_moves;

        auto result = move_map.find(subgame_hash);

        if (result == move_map.end())
            return nullptr;

        return &result->second;
    }

    std::set<move>* _get_or_create_set(bw player, hash_t subgame_hash)
    {
        assert(is_black_white(player));

        move_map_t& move_map = (player == BLACK) ? _black_moves : _white_moves;
        return &move_map[subgame_hash];
    }

    typedef std::map<hash_t, std::set<move>> move_map_t;
    friend struct serializer<dominated_moves_t>;

    move_map_t _black_moves;
    move_map_t _white_moves;
};

inline bool dominated_moves_t::operator==(const dominated_moves_t& rhs) const
{
    return (_black_moves == rhs._black_moves) && //
           (_white_moves == rhs._white_moves);   //
}

inline bool dominated_moves_t::operator!=(const dominated_moves_t& rhs) const
{
    return !(*this == rhs);
}

////////////////////////////////////////////////// serializer<dominated_moves_t>
template<>
struct serializer<dominated_moves_t>
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

    inline static void save(obuffer& os, const dominated_moves_t& dm)
    {
        serializer_save(os, dm._black_moves);
        serializer_save(os, dm._white_moves);
    }

    inline static dominated_moves_t load(ibuffer& is)
    {
        dominated_moves_t dm;

        serializer_load(is, dm._black_moves);
        serializer_load(is, dm._white_moves);

        return dm;
    }
};

//////////////////////////////////////////////////
dominated_moves_t db_make_dominated_moves(const sumgame& sum);
