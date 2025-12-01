#include "js_solve.h"

#include <sstream>
#include <string>

#include "emscripten.h" // IWYU pragma: keep
#include "file_parser.h"
#include "search_utils.h"

using namespace std;

// TODO refactor so main.cpp and this function use the same logic...
js_struct js_solve(const std::string& game_string)
{
    file_parser* fp = file_parser::from_string(game_string);
    game_case gc;

    stringstream stream;

    bool first = true;

    int n_cases = 0;

    while (fp->parse_chunk(gc))
    {
        n_cases++;
        if (!first)
            stream << '\n';

        for (game* g : gc.games)
            stream << *g << '\n';

        search_result result = gc.run();

        stream << "Player: " << result.player_str() << '\n';
        stream << "Result: " << result.value_str() << '\n';

        first = false;
        gc.cleanup_games();
    }

    delete fp;

    return {stream.str(), n_cases};

    //return stream.str();
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS()
{
    emscripten::class_<js_struct>("js_struct")
        .property("result", &js_struct::result)
        .property("n_cases", &js_struct::n_cases);

    emscripten::function("js_solve", &js_solve);
}
#endif
