/*
    Utilities for creating and annotating `.dot` graph files of search trees
    i.e. during partisan search.

    See `sgraph` namespace near bottom of this file
*/
#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>

#include "sumgame.h"

////////////////////////////////////////////////// Node types
// Determines color of node
enum search_node_type_t
{
    SEARCH_NODE_TYPE_SIMPLIFICATION, // unused
    SEARCH_NODE_TYPE_PRUNED, // pruned node (outcome unknown)
    SEARCH_NODE_TYPE_WIN,
    SEARCH_NODE_TYPE_LOSS,

    SEARCH_NODE_TYPE_ANNOTATED_LOSS, // correction: should be loss
    SEARCH_NODE_TYPE_ANNOTATED_WIN, // correction: should be win
    SEARCH_NODE_TYPE_ANNOTATED_PRUNED_LOSS, // pruned node which should be loss
    SEARCH_NODE_TYPE_ANNOTATED_PRUNED_WIN, // pruned node which should be win
};

////////////////////////////////////////////////// class search_graph_printer
// Prints nodes to `.dot` file. 
class search_graph_printer
{
public:
    search_graph_printer(const std::string& file_name);
    ~search_graph_printer();

    void push(const sumgame& sum);
    void push(const std::string& node_string);
    void pop(search_node_type_t node_type);

    void close(); // successful completion of search
    void abort(); // deletes file, i.e. when search times out

private:
    struct search_node
    {
        search_node(uint64_t node_id, std::string node_string)
            : node_id(node_id), node_string(node_string)
        {
        }

        uint64_t node_id;
        std::string node_string;
    };

    void _start_graph();
    void _end_graph();

    void _print_node(const search_node& node, search_node_type_t node_type);
    void _print_edge(uint64_t from_id, uint64_t to_id);

    std::filesystem::path _outfile_path;
    std::ofstream _outfile;
    uint64_t _next_node_id;
    std::vector<search_node> _node_stack;
};

////////////////////////////////////////////////// Implementation details
// NOLINTNEXTLINE(readability-identifier-naming)
namespace _sgraph_impl {
extern std::optional<search_graph_printer> graph_printer;
} // namespace _sgraph_impl

////////////////////////////////////////////////// sgraph API
namespace sgraph {
// Set output directory for INITIAL printing of search graphs
void set_dir(const std::string& graph_file_dir);

inline bool is_recording()
{
    return _sgraph_impl::graph_printer.has_value();
}

// Start/end recording of search graph. Set success=false IFF search times out
void start();
void end(bool success);

inline void push(const sumgame& sum)
{
    if (!is_recording())
        return;

    _sgraph_impl::graph_printer.value().push(sum);
}

inline void push(const std::string& node_string)
{
    if (!is_recording())
        return;

    _sgraph_impl::graph_printer.value().push(node_string);
}

inline void pop(search_node_type_t node_type)
{
    if (!is_recording())
        return;

    _sgraph_impl::graph_printer.value().pop(node_type);
}

inline void pop_winloss(bool win)
{
    if (!is_recording())
        return;

    _sgraph_impl::graph_printer.value().pop(win ? SEARCH_NODE_TYPE_WIN
                                                : SEARCH_NODE_TYPE_LOSS);
}

/*
    For each `XYZ.dot` file in the given directory (whose filename doesn't
    contain "_annotated"), create a `XYZ_annotated.dot` file with corrections
    and additional info about nodes
*/
void annotate_graphs(const std::string& input_dir);

} // namespace sgraph
