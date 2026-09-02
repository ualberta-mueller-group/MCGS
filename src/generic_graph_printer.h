#pragma once

#include <string>
#include <cstdint>
#include <set>
#include <map>
#include <unordered_map>

class generic_graph_printer
{
public:
    generic_graph_printer(): _next_id(1) {}

    void add_vertex(const void* v1_ptr, const std::string& vertex_label);
    void add_edge(const void* v1_ptr, const void* v2_ptr, const std::string& edge_label);

    void print_to_file(const std::string& file_name, const std::string& title) const;

private:
    typedef uintptr_t vertex_id_t;

    struct edge_t
    {
        edge_t(vertex_id_t v2_id, std::string edge_label)
            : v2_id(v2_id), edge_label(edge_label)
        {
        }

        bool operator<(const edge_t& rhs) const { return v2_id < rhs.v2_id; }

        bool operator==(const edge_t& rhs) const { return v2_id == rhs.v2_id; }

        vertex_id_t v2_id;
        std::string edge_label;
    };

    struct vertex_t
    {
        vertex_t(std::string vertex_label) : vertex_label(vertex_label) {}

        std::string vertex_label;
        std::set<edge_t> out_edges;
    };

    vertex_id_t _ptr_to_vertex_id(const void* ptr);

    vertex_id_t _next_id;
    std::unordered_map<uintptr_t, vertex_id_t> _id_map;
    std::map<vertex_id_t, vertex_t> _vertex_map;
};

