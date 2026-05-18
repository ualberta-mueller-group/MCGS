#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>

#include "serializer.h"
#include "serializer_lib_therm.h"
#include "hashing.h"

#include "ThGraph.h"

typedef uint64_t thgraph_id_t;

constexpr thgraph_id_t THGRAPH_ID_NONE = 0;

class thermograph_cache
{
public:

    // Precondition: graph != nullptr. Caller gives up ownership.
    std::shared_ptr<ThGraph> insert_and_release(ThGraph* graph);

    // Precondition: if graph != nullptr then it must already have been inserted
    thgraph_id_t get_graph_id(const ThGraph* graph_nullable) const;

    // Returns nullptr if graph_id == THGRAPH_ID_NONE
    std::shared_ptr<ThGraph> get_graph_from_id(thgraph_id_t graph_id);

    // TODO: compare value, not "verbatim equals"
    bool operator==(const thermograph_cache& rhs) const;

    inline bool operator!=(const thermograph_cache& rhs) const
    {
        return !(*this == rhs);
    }

private:
    std::vector<std::shared_ptr<ThGraph>> _graphs;
    std::unordered_map<hash_t, thgraph_id_t> _hash_to_graph_id;

    friend struct serializer<thermograph_cache>;
};

template <>
struct serializer<thermograph_cache>
{
    inline static void save(obuffer& os, const thermograph_cache& cache, serializer_ctx* ctx)
    {
        serializer_save(os, cache._graphs, ctx);
        serializer_save(os, cache._hash_to_graph_id, ctx);
    }

    inline static thermograph_cache load(ibuffer& is, serializer_ctx* ctx)
    {
        thermograph_cache cache;
        serializer_load(is, cache._graphs, ctx);
        serializer_load(is, cache._hash_to_graph_id, ctx);
        return cache;
    }
};

