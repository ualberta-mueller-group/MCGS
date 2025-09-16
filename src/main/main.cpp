//---------------------------------------------------------------------------
// main.cpp - main loop of MCGS
//---------------------------------------------------------------------------

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "basic_player.h"
#include "cli_options.h"
#include "file_parser.h"
#include "autotests.h"
#include "gen_experiments.h"
#include "mcgs_init.h"
#include "hashing.h"
#include "global_options.h"
#include "random.h"
#include "search_utils.h"

#include "nogo_split_test.h"
#include "sumgame.h"
#include "throw_assert.h"

using std::cout, std::endl, std::string;

class print_on_exit
{
public:
    print_on_exit(const std::string& message)
        : _message(message)
    {
    }

    ~print_on_exit()
    {
        std::cout << _message << std::flush;
    }

private:
    std::string _message;
};

int main(int argc, char** argv)
{
    print_on_exit poe("Exiting main\n");

    cli_options opts = parse_args(argc, (const char**) argv, false);

    // i.e. ./MCGS --help
    if (opts.should_exit)
        return 0;

    cout << "Pre-init" << endl;
    mcgs_init_all(opts);
    cout << "Post-init" << endl;

    /*
    std::ofstream out_file("table.txt");

    THROW_ASSERT(out_file.is_open());

    random_table& rt = get_global_random_table(RANDOM_TABLE_DEFAULT);

    const size_t table_size = rt.current_size();
    out_file << "Size: " << table_size << "\n";

    for (size_t i = 0; i < table_size; i++)
    {
        for (int j = 0; j < 256; j++)
        {
            out_file << "(" << i << ", " << j << "): " << rt.get_zobrist_val(i, j)
                << "\n";
        }
    }

    out_file.close();
    */

    random_generator gen(31);
    for (int i = 0; i < 20; i++)
        cout << gen.get_u32() << endl;

    return 0;

    if (opts.use_player)
    {
        file_parser* parser = opts.parser.get();
        THROW_ASSERT(parser != nullptr, "No games specified for player");

        play_games(*parser, opts.play_log_name);
        return 0;
    }

    if (opts.nogo_test)
    {
        nogo_split_test();
        return 0;
    }

    if (opts.run_tests)
    {
        run_autotests(opts.test_directory, opts.outfile_name,
                      opts.test_timeout);
        return 0;
    }

    //if (opts.run_tests_stdin)
    //{
    //    run_autotests_stdin(opts.outfile_name, opts.test_timeout);
    //    return 0;
    //}

    if (opts.gen_experiments)
    {
        gen_experiments();
        return 0;
    }

    // Run sums from input
    if (opts.parser)
    {
        game_case gc;
        bool first_case = true;

        while (opts.parser->parse_chunk(gc))
        {
            if (!first_case)
                cout << endl;

            for (game* g : gc.games)
                cout << "\t" << *g << endl;

            if (gc.games.size() == 0)
                cout << "\t" << "<no games specified>" << endl;

            cout << "Player: " << player_name_bw_imp(gc.to_play) << endl;
            cout << "Expected: " << gc.expected_value.str() << endl;

            if (opts.dry_run)
                cout << "Not running search..." << endl;
            else
            {
                if (global::clear_tt() && !first_case)
                    sumgame::reset_ttable();

                search_result sr = gc.run(0);
                cout << "Got: " << sr.value_str() << endl;
                cout << "Time (ms): " << sr.duration_str() << endl;
                cout << "Status: " << sr.status_str() << endl;
            }

            if (gc.comments.size() > 0)
                cout << "\"" << gc.comments << "\"" << endl;

            gc.cleanup_games();
            first_case = false;
        }
    }

    if (random_table::did_resize_warning())
        random_table::print_resize_warning();


    return 0;
}
