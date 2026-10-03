#include "cgt_environment.h"
#include "integral_conversion.h"
#include "n_bit_int.h"
#include "safe_arithmetic.h"
#include "search_graph_debug.h"
#include "sign_enum.h"
#include "string_to_int.h"
#include "throw_assert.h"
#include "utilities.h"
#include "generic_graph_printer.h"

#include <cctype>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <type_traits>
#include <unistd.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <memory>
#include <fstream>
#include <optional>
#include <cassert>
#include <variant>

#include "ast2_token.h"
#include "ast2_node.h"
#include "ast2_parser.h"
#include "ast2_lexer.h"

using namespace std;

////////////////////////////////////////////////// Exported functions
cgt_environment::cgt_environment(shared_ptr<const ast2_sum> sum_node)
    : _sum_node(sum_node)
{
    assert(_sum_node);
}

game* cgt_environment::make_game() const
{
    THROW_ASSERT(_sum_node);
    return _sum_node->make_game(false);
}

void test_cgt_environment(const string& env_string, size_t line_start, size_t column_start)
{
    cout << "Input string (";
    cout << "L" << line_start << " C" << column_start;
    cout << "):" << endl;

    cout << "`" << env_string << "`" << endl;

    cout << endl;

    ast2_lexer lexer;
    const vector<ast2_token> tokens =
        lexer.get_tokens(&env_string, line_start, column_start);

    cout << "Token stream (after lexer rules):" << endl;
    cout << "[" << endl;
    for (const ast2_token& tok : tokens)
        cout << "\t" << tok << endl;
    cout << "]" << endl;

    cout << endl;

    //const ast2_token_scope ts(0, tokens.size());
    //size_t idx = 0;
    ast2_parser parser;
    unique_ptr<ast2_sum> s = parser.parse(&tokens);

    cout << "Parse tree:" << endl;
    assert(s);
    s->print(cout, 0);
    cout << endl;

    static unsigned int graph_number = 0;

    const string file_name = "graphs/" + to_string(graph_number++) + ".dot";
    generic_graph_printer graph;
    s->print_graph(graph);
    graph.print_to_file(file_name, env_string);

    //THROW_ASSERT(idx == tokens.size(), "Parsing incomplete!");
}

cgt_environment parse_cgt_environment(const string& env_string, size_t line_start, size_t column_start)
{
    ast2_lexer lexer;
    const vector<ast2_token> tokens =
        lexer.get_tokens(&env_string, line_start, column_start);

    ast2_parser parser;
    unique_ptr<ast2_sum> sum = parser.parse(&tokens);

    return cgt_environment(shared_ptr<const ast2_sum>(sum.release()));
}

