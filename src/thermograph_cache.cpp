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

thermograph_cache::thermograph_cache()
{
}

thermograph_cache::~thermograph_cache()
{
}

thgraph_id_t thermograph_cache::insert(ThGraph* graph)
{
    assert(graph != nullptr);

    const hash_t hash = get_thermograph_hash(*graph);
    thgraph_id_t& map_id = _id_to_graph_map[hash];

    // map_id == 0 == THGRAPH_ID_NONE if the element didn't already exist
    if (map_id != THGRAPH_ID_NONE)
    {
        delete graph;
        return map_id;
    }

    const thgraph_id_t next_id = _graphs.size() + 1;
    assert(next_id != THGRAPH_ID_NONE);
    map_id = next_id;

    _graphs.emplace_back(graph);

    assert(graph == get_graph_from_id(map_id).get());
    return map_id;
}

shared_ptr<const ThGraph> thermograph_cache::get_graph_from_id(
    thgraph_id_t thgraph_id) const
{
    assert(thgraph_id <= _graphs.size());

    if (thgraph_id == THGRAPH_ID_NONE)
        return nullptr;

    return _graphs[thgraph_id - 1];
}

shared_ptr<ThGraph> thermograph_cache::get_nonconst_graph_from_id(
    thgraph_id_t thgraph_id)
{
    assert(thgraph_id <= _graphs.size());

    if (thgraph_id == THGRAPH_ID_NONE)
        return nullptr;

    return _graphs[thgraph_id - 1];
}

bool thermograph_cache::operator==(const thermograph_cache& rhs) const
{
    if (_graphs.size() != rhs._graphs.size())
        return false;

    const size_t n_graphs = _graphs.size();
    for (size_t i = 0; i < n_graphs; i++)
        if (!(*_graphs[i] == *rhs._graphs[i]))
            return false;

    return _id_to_graph_map == rhs._id_to_graph_map;
}

