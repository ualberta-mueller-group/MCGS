#include "js_solve.h"

#include <memory>
#include <sstream>
#include <string>

#include "csv_row.h"
#include "emscripten.h" // IWYU pragma: keep
#include "file_parser.h"
#include "test_case.h"

using namespace std;

// TODO refactor so main.cpp and this function use the same logic...
js_struct js_solve(const std::string& game_string)
{
    file_parser* fp = file_parser::from_string(game_string);

    stringstream stream;

    bool first = true;
    int global_test_case_count = 0;

    while (fp->parse_chunk())
    {
        const int n_test_cases = fp->n_test_cases();

        for (int test_case_idx = 0; test_case_idx < n_test_cases; test_case_idx++)
        {
            global_test_case_count++;
            if (!first)
                stream << '\n';

            std::shared_ptr<i_test_case> test_case = fp->get_test_case(test_case_idx);
            const std::vector<game*>& games = test_case->get_games();

            for (game* g : games)
                stream << *g << '\n';

            test_case->run(0);
            //search_result result = gc.run();

            const csv_row& row = test_case->get_csv_row();

            stream << "Player: " << print_optional(row.player, CSV_MISSING_TEXT) << '\n';
            stream << "Result: " << print_optional(row.result, CSV_MISSING_TEXT) << '\n';

            first = false;
        }
    }

    delete fp;

    return {stream.str(), global_test_case_count};

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
