/*
    serializer templates for various cgt_lib/therm data types:
*/
#pragma once
#include "serializer.h"

#include <cstdint>
#include <cassert>
#include <memory>

#include "iobuffer.h"

#include "ThValue.h"
#include "ThPoint.h"
#include "ThScaffold.h"
#include "ThGraph.h"
#include "SgBlackWhite.h"
#include "thermograph_cache.h"

template <>
struct serializer<ThValue>
{
    inline static void save(obuffer& os, const ThValue& value, serializer_ctx* ctx)
    {
        os.write_i32(value.P());
        os.write_i32(value.Q());
    }

    inline static ThValue load(ibuffer& is, serializer_ctx* ctx)
    {
        const int32_t p = is.read_i32();
        const int32_t q = is.read_i32();

        return ThValue(p, q);
    }

    inline static ThValue* load_ptr(ibuffer& is, serializer_ctx* ctx)
    {
        const int32_t p = is.read_i32();
        const int32_t q = is.read_i32();

        return new ThValue(p, q);
    }

};

template <>
struct serializer<ThPoint>
{
    inline static void save(obuffer& os, const ThPoint& point, serializer_ctx* ctx)
    {
        const ThValue& value = point.Value();
        const ThValue& temp = point.Temp();

        serializer<ThValue>::save(os, value, ctx);
        serializer<ThValue>::save(os, temp, ctx);
    }

    inline static ThPoint load(ibuffer& is, serializer_ctx* ctx)
    {
        const ThValue value = serializer<ThValue>::load(is, ctx);
        const ThValue temp = serializer<ThValue>::load(is, ctx);

        return ThPoint(value, temp);
    }

    inline static ThPoint* load_ptr(ibuffer& is, serializer_ctx* ctx)
    {
        const ThValue value = serializer<ThValue>::load(is, ctx);
        const ThValue temp = serializer<ThValue>::load(is, ctx);

        return new ThPoint(value, temp);
    }
};

template <>
struct serializer<ThScaffold>
{
    inline static void save(obuffer& os, const ThScaffold& sc, serializer_ctx* ctx)
    {
        const int32_t n_points = sc.NuPoints();
        os.write_i32(n_points);

        for (int32_t i = 1; i <= n_points; i++)
        {
            const ThPoint* point = sc.NthPoint(i);
            assert(point != nullptr);

            serializer<ThPoint>::save(os, *point, ctx);
        }
    }

    inline static ThScaffold load(ibuffer& is, serializer_ctx* ctx)
    {
        ThScaffold sc;

        const int32_t n_points = is.read_i32();

        for (int32_t i = 1; i <= n_points; i++)
            sc.AppendPoint(serializer<ThPoint>::load(is, ctx));

        return sc;
    }

    inline static ThScaffold* load_ptr(ibuffer& is, serializer_ctx* ctx)
    {
        ThScaffold* sc = new ThScaffold();

        const int32_t n_points = is.read_i32();

        for (int32_t i = 1; i <= n_points; i++)
            sc->AppendPoint(serializer<ThPoint>::load(is, ctx));

        return sc;
    }
};

template <>
struct serializer<ThGraph>
{
    inline static void save(obuffer& os, const ThGraph& graph, serializer_ctx* ctx)
    {
        const ThScaffold* sc_black = graph.Sc(SG_BLACK);
        const ThScaffold* sc_white = graph.Sc(SG_WHITE);
        assert(sc_black != nullptr && sc_white != nullptr);

        serializer<ThScaffold>::save(os, *sc_black, ctx);
        serializer<ThScaffold>::save(os, *sc_white, ctx);
    }

    inline static ThGraph load(ibuffer& is, serializer_ctx* ctx)
    {
        const ThScaffold sc_black = serializer<ThScaffold>::load(is, ctx);
        const ThScaffold sc_white = serializer<ThScaffold>::load(is, ctx);

        return ThGraph(sc_black, sc_white);
    }

    inline static ThGraph* load_ptr(ibuffer& is, serializer_ctx* ctx)
    {
        const ThScaffold sc_black = serializer<ThScaffold>::load(is, ctx);
        const ThScaffold sc_white = serializer<ThScaffold>::load(is, ctx);

        return new ThGraph(sc_black, sc_white);
    }
};

template <>
struct serializer<std::shared_ptr<ThGraph>>
{
    inline static void save(obuffer& os, const std::shared_ptr<ThGraph>& ptr,
                            serializer_ctx* ctx)
    {
        thermograph_cache* cache = get_thermograph_cache(ctx);

        if (cache == nullptr)
            serializer_impl<std::shared_ptr<ThGraph>>::save(os, ptr, ctx);
        else
        {
            const thgraph_id_t graph_id = cache->get_graph_id(ptr.get());
            serializer<thgraph_id_t>::save(os, graph_id, ctx);
        }
    }

    inline static std::shared_ptr<ThGraph> load(ibuffer& is,
                                                serializer_ctx* ctx)
    {
        thermograph_cache* cache = get_thermograph_cache(ctx);

        if (cache == nullptr)
            return serializer_impl<std::shared_ptr<ThGraph>>::load(is, ctx);
        else
        {
            const thgraph_id_t graph_id = serializer<thgraph_id_t>::load(is, ctx);
            return cache->get_graph_from_id(graph_id);
        }
    }

    inline static thermograph_cache* get_thermograph_cache(serializer_ctx* ctx_nullable)
    {
        if (ctx_nullable == nullptr)
            return nullptr;

        return reinterpret_cast<thermograph_cache*>(ctx_nullable->thermograph_cache_ptr);
    }

    inline static void set_thermograph_cache(serializer_ctx* ctx, thermograph_cache* cache)
    {
        assert(ctx != nullptr);
        ctx->thermograph_cache_ptr = cache;
    }
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

    inline static thermograph_cache* load_ptr(ibuffer& is, serializer_ctx* ctx)
    {
        thermograph_cache* cache = new thermograph_cache();
        serializer_load(is, cache->_graphs, ctx);
        serializer_load(is, cache->_hash_to_graph_id, ctx);
        return cache;
    }
};
