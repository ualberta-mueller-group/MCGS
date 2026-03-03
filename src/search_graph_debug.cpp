#include "search_graph_debug.h"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <cstdint>
#include <type_traits>
#include <vector>

#include "cgt_basics.h"
#include "cgt_dyadic_rational.h"
#include "cgt_integer_game.h"
#include "cgt_switch.h"
#include "cgt_up_star.h"
#include "file_iterator.h"
#include "string_to_int.h"
#include "sumgame.h"
#include "throw_assert.h"
#include "utilities.h"
#include "all_game_headers.h"

using namespace std;

////////////////////////////////////////////////// Implementation details
/*
    Function pointer type for constructing `game` objects from parameter lists
    and game "boards".

    "Parameter list" refers to options for parameterized games i.e. gen_toads.
    For other games, the parameter list will be empty.
*/
typedef game* (*make_game_fn_ptr_t)(const vector<int>&, const string&);

optional<search_graph_printer> _sgraph_impl::graph_printer;

namespace {
optional<string> graph_dir; // output directory
uint64_t next_graph_file_number = 0;

optional<unordered_map<string, search_node_type_t>> color_to_node_type_opt;
optional<unordered_map<string, make_game_fn_ptr_t>> game_name_to_make_game_fn_opt;

} // namespace

////////////////////////////////////////////////// Helpers

namespace {
string node_type_to_color(search_node_type_t node_type)
{
    switch(node_type)
    {
        case SEARCH_NODE_TYPE_SIMPLIFICATION:
            return "aqua";
        case SEARCH_NODE_TYPE_PRUNED:
            return "purple";
        case SEARCH_NODE_TYPE_WIN:
            return "green";
        case SEARCH_NODE_TYPE_LOSS:
            return "red";
        case SEARCH_NODE_TYPE_ANNOTATED_LOSS:
            return "darkred";
        case SEARCH_NODE_TYPE_ANNOTATED_WIN:
            return "darkgreen";
        case SEARCH_NODE_TYPE_ANNOTATED_PRUNED_LOSS:
            return "tomato";
        case SEARCH_NODE_TYPE_ANNOTATED_PRUNED_WIN:
            return "olivedrab";
        default:
            THROW_ASSERT(false); // Not implemented!
    }
}

search_node_type_t color_to_node_type(const string& color_name)
{
    if (!color_to_node_type_opt.has_value())
    {
        color_to_node_type_opt.emplace();

        unordered_map<string, search_node_type_t>&
            color_to_type = *color_to_node_type_opt;

        color_to_type["aqua"] = SEARCH_NODE_TYPE_SIMPLIFICATION;
        color_to_type["purple"] = SEARCH_NODE_TYPE_PRUNED;
        color_to_type["green"] = SEARCH_NODE_TYPE_WIN;
        color_to_type["red"] = SEARCH_NODE_TYPE_LOSS;
        color_to_type["darkred"] = SEARCH_NODE_TYPE_ANNOTATED_LOSS;
        color_to_type["darkgreen"] = SEARCH_NODE_TYPE_ANNOTATED_WIN;
        color_to_type["tomato"] = SEARCH_NODE_TYPE_ANNOTATED_PRUNED_LOSS;

        color_to_type["olivedrab"] =
            SEARCH_NODE_TYPE_ANNOTATED_PRUNED_WIN;
    }

    THROW_ASSERT(color_to_node_type_opt.has_value());

    unordered_map<string, search_node_type_t>&
        color_to_type = *color_to_node_type_opt;

    const auto result = color_to_type.find(color_name);
    THROW_ASSERT(result != color_to_type.end());
    return result->second;
}

// Node read from `.dot` file
struct node_input_t
{
    uint64_t node_id;
    string label;
    search_node_type_t node_type;
};

// Represents a single game extracted from the label of a node_input_t
struct game_token_t
{
    string name;
    vector<int> parameter_list;
    string content;
};

void consume_whitespace(size_t& idx, const string& label)
{
    const size_t N_CHARS = label.size();

    while (idx < N_CHARS)
    {
        const char c = label[idx];

        if (isspace(c))
            idx++;
        else
            break;
    }
}

string match_title(size_t& idx, const string& label)
{
    const size_t N_CHARS = label.size();

    string title;

    while (idx < N_CHARS)
    {
        const char c = label[idx];

        THROW_ASSERT(!isspace(c));

        // Start of parameter list or end of title?
        if (c == '<' || c == ':')
            break;

        title.push_back(c);
        idx++;
    }

    return title;
}

vector<int> match_parameter_list(size_t& idx, const string& label)
{
    const size_t N_CHARS = label.size();

    if (!(idx < N_CHARS) || label[idx] != '<')
        return {};
    idx++;

    vector<int> parameter_list;
    string chunk;

    auto consume_chunk = [&]() -> void
    {
        if (!chunk.empty())
        {
            parameter_list.emplace_back(str_to_i(chunk));
            chunk.clear();
        }
    };

    while (idx < N_CHARS)
    {
        const char c = label[idx];
        idx++;

        if (c == '>')
            break;

        if (c == '-' || isdigit(c))
        {
            chunk.push_back(c);
            continue;
        }

        THROW_ASSERT(c == ',' || isspace(c));
        consume_chunk();
    }

    consume_chunk();

    return parameter_list;
}

string match_contents(size_t& idx, const string& label)
{
    string contents;
    const size_t N_CHARS = label.size();

    THROW_ASSERT(idx < N_CHARS && label[idx] == ':');
    idx++;
    if (!(idx < N_CHARS))
        return contents;

    string chunk;
    size_t idx_rollback = idx;

    bool prev_is_space = isspace(label[idx]);

    while (idx < N_CHARS)
    {
        const char c = label[idx];

        if (c == ':')
        {
            /*
                `chunk` contains next game title and we need to roll back.
                This handles the edge case for `sheep` (whose output may contain
                spaces).
            */
            chunk.clear();
            idx = idx_rollback;
            break;
        }

        const bool current_is_space = isspace(c);

        idx++;
        chunk.push_back(c);

        if (current_is_space && !prev_is_space)
        {
            contents += chunk;
            chunk.clear();

            idx_rollback = idx;
        }

        prev_is_space = current_is_space;
    }

    contents += chunk;

    while (!contents.empty() && isspace(contents.back()))
        contents.pop_back();

    return contents;
}

template <class T>
game* make_game_no_params(const vector<int>& params, const string& contents)
{
    static_assert(std::is_base_of_v<game, T>);
    THROW_ASSERT(params.empty());
    return new T(contents);
}

template <class T>
game* make_game_with_params(const vector<int>& params, const string& contents)
{
    static_assert(std::is_base_of_v<game, T>);
    return new T(params, contents);
}

game* make_integer_game(const vector<int>& params, const string& contents)
{
    THROW_ASSERT(params.empty());
    const int val = str_to_i(contents);
    return new integer_game(val);
}

game* make_nimber(const vector<int>& params, const string& contents)
{
    THROW_ASSERT(params.empty());
    THROW_ASSERT(!contents.empty() && contents[0] == '*');

    const int val = str_to_i(contents.substr(1));
    return new nimber(val);
}

game* make_dyadic_rational(const vector<int>& params, const string& contents)
{
    THROW_ASSERT(params.empty());

    static regex rational_pattern(
            R"END(^(-?[0-9]+)\/(-?[0-9]+)$)END",
            regex::ECMAScript);

    smatch match;

    regex_match(contents, match, rational_pattern);
    THROW_ASSERT(match.size() == 3);

    const int p = str_to_i(match[1]);
    const int q = str_to_i(match[2]);

    return new dyadic_rational(p, q);
}

game* make_up_star(const vector<int>& params, const string& contents)
{
    THROW_ASSERT(params.empty());

    static regex up_star_pattern(R"END(^(-?[0-9]+)(\*?)$)END",
                                 regex::ECMAScript);

    smatch match;
    regex_match(contents, match, up_star_pattern);

    THROW_ASSERT(match.size() == 3);

    const int ups = str_to_i(match[1]);
    const bool has_star = !match[2].str().empty();

    return new up_star(ups, has_star);
}

game* make_kayles(const vector<int>& params, const string& contents)
{
    THROW_ASSERT(params.empty());
    const int val = str_to_i(contents);
    return new kayles(val);
}

game* make_game(const game_token_t& token)
{
    if (!game_name_to_make_game_fn_opt.has_value())
    {
        game_name_to_make_game_fn_opt.emplace();
        unordered_map<string, make_game_fn_ptr_t>& make_game_fn_map =
            *game_name_to_make_game_fn_opt;

        make_game_fn_map["dyadic_rational"] = make_dyadic_rational;
        make_game_fn_map["integer"] = make_integer_game;
        make_game_fn_map["nimber"] = make_nimber;
        //switch_game
        make_game_fn_map["up_star"] = make_up_star;

        make_game_fn_map["clobber_1xn"] = make_game_no_params<clobber_1xn>;
        make_game_fn_map["nogo_1xn"] = make_game_no_params<nogo_1xn>;
        make_game_fn_map["elephants"] = make_game_no_params<elephants>;
        make_game_fn_map["toppling_dominoes"] = make_game_no_params<toppling_dominoes>;
        make_game_fn_map["gen_toads"] = make_game_with_params<gen_toads>;

        make_game_fn_map["amazons"] = make_game_no_params<amazons>;
        make_game_fn_map["nogo"] = make_game_no_params<nogo>;
        make_game_fn_map["clobber"] = make_game_no_params<clobber>;
        make_game_fn_map["cannibal_clobber"] = make_game_no_params<cannibal_clobber>;
        make_game_fn_map["domineering"] = make_game_no_params<domineering>;
        make_game_fn_map["fission"] = make_game_no_params<fission>;
        make_game_fn_map["sheep"] = make_game_no_params<sheep>;

        make_game_fn_map["kayles"] = make_kayles;
    }

    assert(game_name_to_make_game_fn_opt.has_value());

    unordered_map<string, make_game_fn_ptr_t>& make_game_fn_map =
        *game_name_to_make_game_fn_opt;

    const auto result = make_game_fn_map.find(token.name);
    THROW_ASSERT(result != make_game_fn_map.end());
    return (result->second)(token.parameter_list, token.content);
}

void check_sum_equals_label(const sumgame& sum, const string& label)
{
    stringstream str;
    sum.print_simple(str);
    const string& sum_printed = str.str();

    THROW_ASSERT(sum_printed == label, "Sum not equal to label. Got \"" +
                                           sum_printed + "\" but should be \"" +
                                           label + "\"");
}

void make_sum_from_node_input(sumgame& sum, const node_input_t& node_input)
{
    assert(sum.num_total_games() == 0);

    const string& label = node_input.label;

    ebw player = EMPTY;
    if (!label.empty())
        player = player_char_to_color(label[0]);
    THROW_ASSERT(is_black_white(player));
    sum.set_to_play(player);

    size_t idx = 1;
    const size_t N_CHARS = label.size();

    while (true)
    {
        consume_whitespace(idx, label);
        if (!(idx < N_CHARS))
            break;

        game_token_t token;
        token.name = match_title(idx, label);
        token.parameter_list = match_parameter_list(idx, label);
        token.content = match_contents(idx, label);

        game* g = make_game(token);
        sum.add(g);
    }

    check_sum_equals_label(sum, label);
}

optional<node_input_t> parse_node_input(const string& line)
{
    static regex node_input_pattern(
            R"END(^([0-9]+) *\[label="([^"]+)", *fillcolor="([^"]+)", *color="[^"]+"\];.*$)END", 
            regex::ECMAScript);

    smatch match;

    regex_match(line, match, node_input_pattern);

    if (match.size() != 4)
        return {};

    node_input_t node_input;
    node_input.node_id = str_to_ull(match[1]);
    node_input.label = match[2];
    node_input.node_type = color_to_node_type(match[3]);

    return node_input;
}

void cleanup_sum(sumgame& sum)
{
    int num_games = sum.num_total_games();

    while (num_games > 0)
    {
        game* g = sum.subgame(num_games - 1);
        num_games--;

        sum.pop(g);
        delete g;
    }
}

inline search_node_type_t get_new_node_type(search_node_type_t old_type,
                                            bool win)
{
    switch (old_type)
    {
        case SEARCH_NODE_TYPE_WIN:
            return (win ? SEARCH_NODE_TYPE_WIN
                        : SEARCH_NODE_TYPE_ANNOTATED_LOSS);

        case SEARCH_NODE_TYPE_LOSS:
            return (!win ? SEARCH_NODE_TYPE_LOSS
                         : SEARCH_NODE_TYPE_ANNOTATED_WIN);

        case SEARCH_NODE_TYPE_PRUNED:
            return (win ? SEARCH_NODE_TYPE_ANNOTATED_PRUNED_WIN
                        : SEARCH_NODE_TYPE_ANNOTATED_PRUNED_LOSS);

        //case SEARCH_NODE_TYPE_SIMPLIFICATION:
        //case SEARCH_NODE_TYPE_ANNOTATED_LOSS:
        //case SEARCH_NODE_TYPE_ANNOTATED_WIN:
        //case SEARCH_NODE_TYPE_ANNOTATED_PRUNED_LOSS:
        //case SEARCH_NODE_TYPE_ANNOTATED_PRUNED_WIN:
        default:
            THROW_ASSERT(false); // Not implemented!
    }
}

void handle_node_input(ofstream& outfile, sumgame& sum, node_input_t& node_input)
{
    assert(sum.num_total_games() == 0);
    make_sum_from_node_input(sum, node_input);

    const bool result = sum.solve();
    const search_node_type_t new_node_type = get_new_node_type(node_input.node_type, result);
    const string& color = node_type_to_color(new_node_type);

    outfile << node_input.node_id;
    outfile << " [label=\"" << node_input.label << "\"";
    outfile << ", fillcolor=\"" << color << "\"";
    outfile << ", color=\"" << color << "\"";
    outfile << "];" << endl;

    cleanup_sum(sum);
}

// i.e. `XYZ.dot` -> `XYZ_annotated.dot`
filesystem::path append_suffix_to_path_name(const filesystem::path& path, const string& suffix)
{
    filesystem::path result = path;

    const filesystem::path extension = path.extension();
    result.replace_extension("");
    result.replace_filename(result.filename().string() + suffix);
    result.replace_extension(extension);

    return result;
}

void annotate_single(const filesystem::path& path, sumgame& sum)
{
    assert(sum.num_total_games() == 0);

    ifstream infile(path.string());
    THROW_ASSERT(infile.is_open());

    ofstream outfile(append_suffix_to_path_name(path, "_annotated"));
    THROW_ASSERT(outfile.is_open());

    string line;
    while (getline(infile, line))
    {
        optional<node_input_t> node_input = parse_node_input(line);
        if (node_input.has_value())
        {
            handle_node_input(outfile, sum, *node_input);
            continue;
        }

        outfile << line << endl;
    }

    infile.close();
    outfile.close();

}
} // namespace

////////////////////////////////////////////////// class search_graph_printer
search_graph_printer::search_graph_printer(const string& file_name)
    : _outfile_path(file_name), _outfile(_outfile_path), _next_node_id(0)
{
    THROW_ASSERT(_outfile.is_open());
    _start_graph();
}

search_graph_printer::~search_graph_printer()
{
    close();
}

void search_graph_printer::push(const sumgame& sum)
{
    ostringstream str;
    sum.print_simple(str);

    push(str.str());
}

void search_graph_printer::push(const string& node_string)
{
    _node_stack.emplace_back(_next_node_id, node_string);
    _next_node_id++;
}

void search_graph_printer::pop(search_node_type_t node_type)
{
    THROW_ASSERT(!_node_stack.empty());
    search_node& node = _node_stack.back();

    //if (node_type == SEARCH_NODE_TYPE_SIMPLIFICATION)
    //{
    //    THROW_ASSERT(_node_stack.size() >= 2);
    //    search_node& node_prev = _node_stack[_node_stack.size() - 2];

    //    if (node.node_string == node_prev.node_string)
    //    {
    //        node_prev.node_id = node.node_id;
    //        _node_stack.pop_back();
    //        return;
    //    }
    //}

    _print_node(node, node_type);

    if (_node_stack.size() >= 2)
    {
        search_node& node_prev = _node_stack[_node_stack.size() - 2];
        _print_edge(node_prev.node_id, node.node_id);
    }

    _node_stack.pop_back();
}

void search_graph_printer::close()
{
    if (_outfile.is_open())
    {
        assert(_node_stack.empty());

        _end_graph();
        _outfile.close();
    }
}

void search_graph_printer::abort()
{
    if (_outfile.is_open())
    {
        _outfile.close();
        filesystem::remove(_outfile_path);
    }
}

void search_graph_printer::_start_graph()
{
    THROW_ASSERT(_outfile.is_open());
    _outfile << "digraph G {" << endl;
    //_outfile << "node [style=filled];" << endl;
}

void search_graph_printer::_end_graph()
{
    THROW_ASSERT(_outfile.is_open());
    _outfile << "}" << flush;
}

void search_graph_printer::_print_node(const search_node& node,
                                       search_node_type_t node_type)
{
    THROW_ASSERT(_outfile.is_open());

    _outfile << node.node_id;
    _outfile << " [label=\"" << node.node_string << "\"";
    _outfile << ", fillcolor=\"" << node_type_to_color(node_type) << "\"";
    _outfile << ", color=\"" << node_type_to_color(node_type) << "\"";
    _outfile << "];" << endl;
}

void search_graph_printer::_print_edge(uint64_t from_id, uint64_t to_id)
{
    THROW_ASSERT(_outfile.is_open());
    _outfile << from_id << " -> " << to_id << ";" << endl;
}

////////////////////////////////////////////////// sgraph API
namespace sgraph {
void set_dir(const string& graph_file_dir)
{
    assert(!graph_dir.has_value());
    graph_dir = graph_file_dir;
}

void start()
{
    assert(!_sgraph_impl::graph_printer.has_value());
    if (!graph_dir.has_value())
        return;

    const filesystem::path dir(*graph_dir);

    if (!filesystem::exists(dir))
        filesystem::create_directory(dir);

    THROW_ASSERT(filesystem::is_directory(dir));

    const string filename = to_string(next_graph_file_number) + ".dot";

    _sgraph_impl::graph_printer.emplace(dir / filename);
    next_graph_file_number++;
}

void end(bool success)
{
    assert(logical_iff(graph_dir.has_value(),
                       _sgraph_impl::graph_printer.has_value()));

    if (_sgraph_impl::graph_printer.has_value())
    {
        if (success)
            _sgraph_impl::graph_printer->close();
        else
            _sgraph_impl::graph_printer->abort();

        _sgraph_impl::graph_printer.reset();
    }
}

void annotate_graphs(const string& input_dir)
{
    THROW_ASSERT(!is_recording());
    graph_dir.reset(); // don't generate more graphs as we verify them

    sumgame sum(BLACK);

    for (file_iterator_alphabetical iter(input_dir); iter; ++iter)
    {
        const filesystem::directory_entry& entry = iter.gen_entry();

        if (!entry.is_regular_file())
            continue;

        const filesystem::path& file_path = entry.path();

        if (file_path.extension() != ".dot")
            continue;

        const string& stem = file_path.stem().string();

        if (stem.find("_annotated") != string::npos)
            continue;

        annotate_single(file_path, sum);
    }
}

} // namespace sgraph
