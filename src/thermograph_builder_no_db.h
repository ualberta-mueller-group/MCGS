#pragma once

// TODO merge this with the DB-dependent version

#include <memory>
#include <vector>
#include <unordered_map>

#include "ThGraph.h"
#include "sumgame.h"
#include "cgt_basics.h"
#include "hashing.h"

class thermograph_builder_no_db
{
public:
    std::shared_ptr<ThGraph> build_thermograph(sumgame& sum);

private:
    std::shared_ptr<ThGraph> _get_thermograph_from_cache(sumgame& sum);

    std::vector<std::shared_ptr<ThGraph>> _get_option_graphs_for_player(
        sumgame& sum, bw player);

    std::unordered_map<hash_t, std::shared_ptr<ThGraph>> _therm_cache;
};
