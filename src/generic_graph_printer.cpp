#include "generic_graph_printer.h"

#include <fstream>
#include "throw_assert.h"

using namespace std;

void generic_graph_printer::add_vertex(const void* v1_ptr,
                                       const string& vertex_label)
{
    THROW_ASSERT(vertex_label.find('\"') == string::npos);
    THROW_ASSERT(!vertex_label.empty());

    const vertex_id_t v1_id = _ptr_to_vertex_id(v1_ptr);
    const auto inserted = _vertex_map.try_emplace(v1_id, vertex_label);
    THROW_ASSERT(inserted.second);
}

void generic_graph_printer::add_edge(const void* v1_ptr, const void* v2_ptr,
                                     const string& edge_label)
{
    THROW_ASSERT(edge_label.find('\"') == string::npos);

    const vertex_id_t v1_id = _ptr_to_vertex_id(v1_ptr);
    const vertex_id_t v2_id = _ptr_to_vertex_id(v2_ptr);

    auto v1_it = _vertex_map.find(v1_id);
    THROW_ASSERT(v1_it != _vertex_map.end());

    const auto v2_it = _vertex_map.find(v2_id);
    THROW_ASSERT(v2_it != _vertex_map.end());

    vertex_t& v1 = v1_it->second;
    const auto inserted = v1.out_edges.emplace(v2_id, edge_label);
    THROW_ASSERT(inserted.second);
}

void generic_graph_printer::print_to_file(const string& file_name, const string& title) const
{
    ofstream of(file_name);
    THROW_ASSERT(of.is_open());

    // Write header
    of << "digraph G {\n";
    of << "\tordering=\"out\";\n";
    of << "\tlabelloc=\"top\";\n";
    of << "\tlabel=\"" << title << "\";\n";

    // Write vertices
    for (const pair<const vertex_id_t, vertex_t>& p : _vertex_map)
    {
        of << "\t" << p.first;
        of << "[label=\"" << p.second.vertex_label << "\"];\n";
    }

    of << "\n";

    // Write edges
    for (const pair<const vertex_id_t, vertex_t>& p : _vertex_map)
    {
        const vertex_id_t v1_id = p.first;
        for (const edge_t& edge : p.second.out_edges)
        {
            of << "\t" << v1_id << " -> " << edge.v2_id;

            const string& edge_label = edge.edge_label;
            if (!edge_label.empty())
                of << "[label=\"" << edge_label << "\"]";

            of << ";\n";
        }
    }

    // Write footer
    of << "}\n";

    of.close();
}

generic_graph_printer::vertex_id_t generic_graph_printer::_ptr_to_vertex_id(
    const void* ptr)
{
    const uintptr_t ptr_int = reinterpret_cast<uintptr_t>(ptr);

    vertex_id_t& v_id = _id_map[ptr_int];

    if (v_id == 0)
        v_id = _next_id++;

    return v_id;
}

