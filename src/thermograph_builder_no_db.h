/*
    TODO merge this with the DB-dependent version ???

    TODO the builder's `ThGraph`s may be shared with the database's. This
    could conceivably be a problem because the function signature for
    `ThGraph::MakeGraphFromOptions` doesn't respect constness of `ThGraph`s.
*/
#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "ThGraph.h"
#include "sumgame.h"
#include "cgt_basics.h"
#include "hashing.h"
#include "timeout_token.h"

class database;

class thermograph_builder_no_db
{
public:
    std::shared_ptr<const ThGraph> build_thermograph(
        sumgame& sum, const database* db_nullable = nullptr);

    // May return nullptr. timeout=0 means never timeout
    std::shared_ptr<const ThGraph> build_thermograph_with_timeout(
        sumgame& sum, unsigned long long timeout,
        const database* db_nullable = nullptr);

    void clear();

    static thermograph_builder_no_db& get_global_instance();

private:
    std::shared_ptr<ThGraph> _build_thermograph_from_options(
        sumgame& sum, const timeout_token& timeout_tok,
        const database* db_nullable);

    std::shared_ptr<ThGraph> _get_thermograph_from_cache(
        sumgame& sum, const timeout_token& timeout_tok,
        const database* db_nullable);

    std::vector<std::shared_ptr<ThGraph>> _get_option_graphs_for_player(
        sumgame& sum, bw player, const timeout_token& timeout_tok,
        const database* db_nullable);

    std::unordered_map<hash_t, std::shared_ptr<ThGraph>> _therm_cache;
};
