#include "thermograph_cache.h"

#include <vector>
#include <cassert>
#include <memory>
#include <cstddef>

#include "hashing.h"
#include "ThGraph.h"
#include "integral_conversion.h"
#include "thermograph_helpers.h"
#include "throw_assert.h"

using namespace std;

shared_ptr<ThGraph> thermograph_cache::insert_and_release(ThGraph* graph)
{
    assert(graph != nullptr);

    const hash_t hash = get_thermograph_hash(*graph);

    static_assert(THGRAPH_ID_NONE == thgraph_id_t());
    thgraph_id_t& graph_id = _hash_to_graph_id[hash];

    if (graph_id != THGRAPH_ID_NONE)
    {
        // Graph already inserted
        delete graph;
        return get_graph_from_id(graph_id);
    }

    // Allocate new ID
    graph_id = integral_cast_unsafe<thgraph_id_t>(_graphs.size()) + 1;
    assert(graph_id != THGRAPH_ID_NONE);

    _graphs.emplace_back(graph);

    shared_ptr<ThGraph> cached_graph = get_graph_from_id(graph_id);
    assert(graph == cached_graph.get());

    return cached_graph;
}

thgraph_id_t thermograph_cache::get_graph_id(
    const ThGraph* graph_nullable) const
{
    if (graph_nullable == nullptr)
        return THGRAPH_ID_NONE;

    const hash_t hash = get_thermograph_hash(*graph_nullable);
    const auto result = _hash_to_graph_id.find(hash);

    THROW_ASSERT(result != _hash_to_graph_id.end());

    assert(result->second != THGRAPH_ID_NONE);
    return result->second;
}

shared_ptr<ThGraph> thermograph_cache::get_graph_from_id(thgraph_id_t graph_id)
{
    if (graph_id == THGRAPH_ID_NONE)
        return nullptr;

    assert(graph_id > 0);

    const size_t idx = integral_cast_checked<size_t>(graph_id - 1);
    THROW_ASSERT(0 <= idx && idx < _graphs.size());

    return _graphs[idx];
}

bool thermograph_cache::operator==(const thermograph_cache& rhs) const
{
    if (_graphs.size() != rhs._graphs.size())
        return false;

    const size_t n_graphs = _graphs.size();
    for (size_t i = 0; i < n_graphs; i++)
        if (!(*_graphs[i] == *rhs._graphs[i]))
            return false;

    if (_hash_to_graph_id != rhs._hash_to_graph_id)
        return false;

    return true;
}
