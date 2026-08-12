/*
    Class db_dom_moves_t represents dominated or nondominated moves for each
    subgame of a sum. It's used as a field in the db_entry_partisan struct.

    Query/add moves with:
        - Subgame hash
        - (DB encoded) move
        - Player to play
*/
#pragma once

#include <set>
#include <map>
#include <variant>
#include <cassert>
#include <vector>
#include <cstddef>
#include <ostream>

#include "cgt_basics.h"
#include "hashing.h"
#include "game.h"
#include "serializer.h"
#include "iobuffer.h"

enum db_dom_moves_kind
{
    DB_DOM_MOVES_KIND_NONE = 0,
    DB_DOM_MOVES_KIND_DOMINATED,
    DB_DOM_MOVES_KIND_NONDOMINATED,
};

////////////////////////////////////////////////// class db_dom_moves_t
class db_dom_moves_t
{
public:
    db_dom_moves_t();

    // All subgames have the same kind
    db_dom_moves_kind get_kind() const;

    /*
        Precondition: `get_kind() == DB_DOM_MOVES_KIND_NONE`,
        and `new_kind != DB_DOM_MOVES_KIND_NONE`
    */
    void set_kind(db_dom_moves_kind new_kind);

    bool operator==(const db_dom_moves_t& rhs) const;
    bool operator!=(const db_dom_moves_t& rhs) const;

    /*
        Precondition: `move_kind != DB_DOM_MOVES_KIND_NONE`, and
        `move_kind = get_kind()`
    */
    void add_move(hash_t subgame_hash, bw player, move move_db_encoded,
                  db_dom_moves_kind move_kind);

    //// Following functions are valid only if kind is DOMINATED

    // May be nullptr
    const std::set<move>* get_dominated_moves(hash_t subgame_hash,
                                              bw player) const;

    //// Following functions are valid only if kind is NONDOMINATED
    // May be nullptr
    const std::vector<move>* get_nondominated_moves(hash_t subgame_hash,
                                                    bw player) const;

private:
    typedef std::map<hash_t, std::set<move>> hash_to_set_t;
    typedef std::map<hash_t, std::vector<move>> hash_to_vec_t;

    /*
        NOTE: monostate is 1st alternative for better performance (to prevent
        default construction of one of the other alternatives)
    */
    typedef std::variant<std::monostate, hash_to_set_t, hash_to_vec_t>
        variant_map_t;

    // The container type is either std::set<move> or std::vector<move>
    template <size_t variant_idx>
    using move_container_t =
        typename std::variant_alternative_t<variant_idx,
                                            variant_map_t>::mapped_type;

    // Never nullptr. Return type is either std::set<move>* or
    // std::vector<move>*
    template <size_t variant_idx>
    move_container_t<variant_idx>* _get_or_create_move_container(
        hash_t subgame_hash, bw player);

    // May be nullptr. Return type is either const std::set<move>* or const
    // std::vector<move>*
    template <size_t variant_idx>
    const move_container_t<variant_idx>* _get_move_container_if_exists(
        hash_t subgame_hash, bw player) const;

    inline static constexpr size_t MONOSTATE_VARIANT_IDX = 0;
    inline static constexpr size_t HASH_TO_SET_VARIANT_IDX = 1;
    inline static constexpr size_t HASH_TO_VEC_VARIANT_IDX = 2;

    friend struct serializer<db_dom_moves_t>;
    friend std::ostream& operator<<(std::ostream& os, const db_dom_moves_t& dom);

    db_dom_moves_kind _kind;
    variant_map_t _black_moves;
    variant_map_t _white_moves;
};

std::ostream& operator<<(std::ostream& os, const db_dom_moves_t& dom);

////////////////////////////////////////////////// db_dom_moves_t methods
inline db_dom_moves_t::db_dom_moves_t() : _kind(DB_DOM_MOVES_KIND_NONE)
{
}

inline db_dom_moves_kind db_dom_moves_t::get_kind() const
{
    return _kind;
}

inline void db_dom_moves_t::set_kind(db_dom_moves_kind new_kind)
{
    assert(_kind == DB_DOM_MOVES_KIND_NONE &&
           new_kind != DB_DOM_MOVES_KIND_NONE);

    _kind = new_kind;
}

inline bool db_dom_moves_t::operator!=(const db_dom_moves_t& rhs) const
{
    return !(*this == rhs);
}

//////////////////////////////////////////////////
template <>
struct serializer<db_dom_moves_t>
{
    inline static void save(i_obuffer& os, const db_dom_moves_t& dom,
                            serializer_ctx* ctx)
    {
        os.write_enum(dom._kind);
        serializer<db_dom_moves_t::variant_map_t>::save(os, dom._black_moves,
                                                        ctx);
        serializer<db_dom_moves_t::variant_map_t>::save(os, dom._white_moves,
                                                        ctx);
    }

    inline static db_dom_moves_t load(i_ibuffer& is, serializer_ctx* ctx)
    {
        db_dom_moves_t dom;

        dom._kind = is.read_enum<db_dom_moves_kind>();
        dom._black_moves =
            serializer<db_dom_moves_t::variant_map_t>::load(is, ctx);
        dom._white_moves =
            serializer<db_dom_moves_t::variant_map_t>::load(is, ctx);

        return dom;
    }

    inline static db_dom_moves_t* load_ptr(i_ibuffer& is, serializer_ctx* ctx)
    {
        db_dom_moves_t* dom = new db_dom_moves_t();

        dom->_kind = is.read_enum<db_dom_moves_kind>();
        dom->_black_moves =
            serializer<db_dom_moves_t::variant_map_t>::load(is, ctx);
        dom->_white_moves =
            serializer<db_dom_moves_t::variant_map_t>::load(is, ctx);

        return dom;
    }
};
