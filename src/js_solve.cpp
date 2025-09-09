#include "js_solve.h"

#include "emscripten.h"
#include "file_parser.h"
#include <sstream>

using namespace std;

// TODO refactor so main.cpp and this function use the same logic...
std::string js_solve(const std::string& game_string)
{
    file_parser* fp = file_parser::from_string(game_string);
    game_case gc;

    stringstream stream;

    bool first = true;

    while (fp->parse_chunk(gc))
    {
        if (!first)
            stream << '\n';

        for (game* g : gc.games)
            stream << *g << '\n';

        search_result result = gc.run();

        stream << "Player: " << result.player_str() << '\n';
        stream << result.player_str() << '\n';
        stream << "Result: " << result.value_str() << '\n';

        first = false;
        gc.cleanup_games();
    }

    delete fp;

    return stream.str();
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS()
{
    emscripten::function("js_solve", &js_solve);
}
#endif
