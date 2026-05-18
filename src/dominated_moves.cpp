#include "dominated_moves.h"

std::ostream& operator<<(std::ostream& os, const db_dom_moves_t& dom)
{
#warning TODO implement me!
    assert(false);
}

bool db_dom_moves_t::operator==(const db_dom_moves_t& rhs) const
{
    if (_kind != rhs._kind)
        return false;

    if (_black_moves != rhs._black_moves)
        return false;

    if (_white_moves != rhs._white_moves)
        return false;

    return true;
}

void db_dom_moves_t::add_move(hash_t subgame_hash, bw player,
                              move move_db_encoded, db_dom_moves_kind move_kind)
{
    assert(is_black_white(player));
    assert(_kind != DB_DOM_MOVES_KIND_NONE && _kind == move_kind);

    switch (move_kind)
    {
        case DB_DOM_MOVES_KIND_NONE:
            THROW_ASSERT(false);
        case DB_DOM_MOVES_KIND_DOMINATED:
        {
            std::set<move>* container =
                _get_or_create_move_container<HASH_TO_SET_VARIANT_IDX>(
                    subgame_hash, player);

            assert(container != nullptr);

            auto inserted = container->insert(move_db_encoded);
            THROW_ASSERT(inserted.second, "Tried to insert duplicate dominated "
                                          "move into db_dom_moves_t!");

            break;
        }
        case DB_DOM_MOVES_KIND_NONDOMINATED:
        {
            std::vector<move>* container =
                _get_or_create_move_container<HASH_TO_VEC_VARIANT_IDX>(
                    subgame_hash, player);

            assert(container != nullptr);

            // TODO should this also check for duplicates somehow?
            container->push_back(move_db_encoded);
            break;
        }
    }
}

bool db_dom_moves_t::move_is_dominated(hash_t subgame_hash, bw player,
                                       move move_db_encoded) const
{
    assert(is_black_white(player));
    assert(_kind == DB_DOM_MOVES_KIND_DOMINATED);

    const std::set<move>* container =
        _get_move_container_if_exists<HASH_TO_SET_VARIANT_IDX>(subgame_hash,
                                                               player);

    if (container == nullptr)
        return false;

    return container->find(move_db_encoded) != container->end();
}

const std::set<move>* db_dom_moves_t::get_dominated_moves(hash_t subgame_hash,
                                                          bw player) const
{
    assert(is_black_white(player));
    assert(_kind == DB_DOM_MOVES_KIND_NONDOMINATED);

    const std::set<move>* container =
        _get_move_container_if_exists<HASH_TO_SET_VARIANT_IDX>(subgame_hash,
                                                               player);
    return container;
}

const std::vector<move>* db_dom_moves_t::get_nondominated_moves(
    hash_t subgame_hash, bw player) const
{
    assert(is_black_white(player));
    assert(_kind == DB_DOM_MOVES_KIND_NONDOMINATED);

    const std::vector<move>* container =
        _get_move_container_if_exists<HASH_TO_VEC_VARIANT_IDX>(subgame_hash,
                                                               player);
    return container;
}

template <size_t variant_idx>
db_dom_moves_t::move_container_t<variant_idx>* db_dom_moves_t::
    _get_or_create_move_container(hash_t subgame_hash, bw player)
{
    assert(_kind == variant_idx && is_black_white(player));

    variant_map_t& m_variant = (player == BLACK) ? _black_moves : _white_moves;
    const size_t m_variant_idx = m_variant.index();

    if (m_variant_idx == MONOSTATE_VARIANT_IDX)
        m_variant.emplace<variant_idx>();

    std::map<hash_t, move_container_t<variant_idx>>& m =
        std::get<variant_idx>(m_variant);

    return &m[subgame_hash];
}

template <size_t variant_idx>
const db_dom_moves_t::move_container_t<variant_idx>* db_dom_moves_t::
    _get_move_container_if_exists(hash_t subgame_hash, bw player) const
{
    assert(_kind == variant_idx && is_black_white(player));

    const variant_map_t& m_variant =
        (player == BLACK) ? _black_moves : _white_moves;

    const size_t m_variant_idx = m_variant.index();

    if (m_variant_idx == MONOSTATE_VARIANT_IDX)
        return nullptr;

    const std::map<hash_t, move_container_t<variant_idx>>& m =
        std::get<variant_idx>(m_variant);

    const auto it = m.find(subgame_hash);
    if (it == m.end())
        return nullptr;

    return &it->second;
}
