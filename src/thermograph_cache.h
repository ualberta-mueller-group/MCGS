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
    thermograph_cache();
    ~thermograph_cache();

    // Takes ownership from the caller and returns an ID
    thgraph_id_t insert(ThGraph* graph);

    std::shared_ptr<const ThGraph> get_graph_from_id(thgraph_id_t thgraph_id) const;
    std::shared_ptr<ThGraph> get_nonconst_graph_from_id(thgraph_id_t thgraph_id);

#warning TODO thermograph_cache equality
    // TODO: compare value, not "verbatim equals"
    bool operator==(const thermograph_cache& rhs) const;

    inline bool operator!=(const thermograph_cache& rhs) const
    {
        return !(*this == rhs);
    }

private:
    std::vector<std::shared_ptr<ThGraph>> _graphs;
    std::unordered_map<hash_t, thgraph_id_t> _id_to_graph_map;

    friend struct serializer<thermograph_cache>;
};

template <>
struct serializer<thermograph_cache>
{
    inline static void save(obuffer& os, const thermograph_cache& cache)
    {
        serializer_save(os, cache._graphs);
        serializer_save(os, cache._id_to_graph_map);
    }

    inline static thermograph_cache load(ibuffer& is)
    {
        thermograph_cache cache;
        serializer_load(is, cache._graphs);
        serializer_load(is, cache._id_to_graph_map);
        return cache;
    }
};

