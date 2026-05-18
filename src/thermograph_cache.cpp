#include "thermograph_cache.h"

#include <type_traits>
#include <vector>
#include <memory>

#include "SgBlackWhite.h"
#include "ThValue.h"
#include "hashing.h"

#include "ThGraph.h"
#include "ThScaffold.h"
#include "iobuffer.h"
#include "thermograph_helpers.h"

using namespace std;


std::shared_ptr<ThGraph> thermograph_cache::insert_and_release(ThGraph* graph)
{
    assert(graph != nullptr);

    const hash_t hash = get_thermograph_hash(*graph);
    thgraph_id_t& graph_id = _hash_to_graph_id[hash];

    if (graph_id != THGRAPH_ID_NONE)
    {
        // Graph already inserted
        delete graph;
        return get_graph_from_id(graph_id);
    }

    graph_id = static_cast<thgraph_id_t>(_graphs.size() + 1);
    assert(graph_id != THGRAPH_ID_NONE);

    _graphs.emplace_back(graph);

    std::shared_ptr<ThGraph> cached_graph = get_graph_from_id(graph_id);
    assert(graph == cached_graph.get());

    return cached_graph;
}

thgraph_id_t thermograph_cache::get_graph_id(const ThGraph* graph_nullable) const
{
    if (graph_nullable == nullptr)
        return THGRAPH_ID_NONE;

    const hash_t hash = get_thermograph_hash(*graph_nullable);
    const auto result = _hash_to_graph_id.find(hash);

    THROW_ASSERT(result != _hash_to_graph_id.end());

    return result->second;
}

std::shared_ptr<ThGraph> thermograph_cache::get_graph_from_id(thgraph_id_t graph_id)
{
    if (graph_id == THGRAPH_ID_NONE)
        return nullptr;

    assert(graph_id > 0);

#warning TODO use numeric cast from header
    const size_t idx = static_cast<size_t>(graph_id - 1);
    THROW_ASSERT(0 <= idx && idx < _graphs.size());

    return _graphs[idx];
}

#warning TODO thermograph_cache equality
bool thermograph_cache::operator==(const thermograph_cache& rhs) const
{
    if (_graphs.size() != rhs._graphs.size())
        return false;

    const size_t n_graphs = _graphs.size();
    for (size_t i = 0; i < n_graphs; i++)
        if (!(*_graphs[i] == *rhs._graphs[i]))
            return false;

    return _hash_to_graph_id == rhs._hash_to_graph_id;
}

