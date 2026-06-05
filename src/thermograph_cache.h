#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>

#include "serializer.h"
#include "hashing.h"

#include "ThGraph.h"

// Disk ID of a thermograph
typedef uint64_t thgraph_id_t;

inline constexpr thgraph_id_t THGRAPH_ID_NONE = 0;

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
