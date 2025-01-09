#include "autotests.h"
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <cassert>
#include <iostream>
#include "cgt_basics.h"
#include "file_parser.h"
#include "game.h"
#include "sumgame.h"
#include "utilities.h"
#include <sstream>
#include <poll.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>

using namespace std;

using filesystem::recursive_directory_iterator;

constexpr char newline = '\n';

#define READ_END 0
#define WRITE_END 1


void reaper(int argc, char** argv)
{
    int parent_pid;

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "--reaper") == 0)
        {
            int argnext = i + 1;
            if (argnext >= argc || !is_int(argv[argnext]))
            {
                cerr << "Bad or missing PID" << endl;
                exit(-1);
            }

            parent_pid = atoi(argv[argnext]);
        }
    }

    // Start child
    int up_pipe[2];
    int err_pipe[2];

    if (pipe(up_pipe) != 0 || pipe(err_pipe) != 0)
    {
        cerr << "Failed to create pipe" << endl;
        exit(-1);
    }

    int fork_id = fork();

    if (fork_id == 0) // bottom process
    {
        close(up_pipe[READ_END]);
        close(err_pipe[READ_END]);
        dup2(up_pipe[WRITE_END], STDIN_FILENO);
        dup2(err_pipe[WRITE_END], STDERR_FILENO);

        // no --reaper or PID, but must have NULL
        int arg_len = argc - 2 + 1;

        char* args[arg_len];
        int argidx = 0;

        for (int i = 0; i < argc; i++)
        {
            if (strcmp(argv[i], "--reaper") == 0)
            {
                // skip 2
                i++;
                continue;
            }

            args[argidx] = argv[i];
            argidx++;
        }

        assert(argidx == arg_len - 1);
        args[argidx] = NULL;

        execv(argv[0], args);

        // Should be unreachable
        cerr << "Failed execv()" << endl;
        exit(-1);
    }

    // middle process

}


string human_readable_game_string(const vector<game *>& games, int* line_count)
{
    stringstream stream;

    const size_t N = games.size();

    for (size_t i = 0; i < N; i++)
    {
        const game* g = games[i];

        g->print(stream);

        if (i + 1 < N)
        {
            stream << newline;
        }

    }

    string result = stream.str();
    *line_count = result.size() > 0 ? 1 : 0;

    for (const char& c : result)
    {
        if (c == '\n')
        {
            (*line_count)++;
        }
    }

    return result;
}


void run_one_case(cli_options& opts)
{
    assert(opts.parser);
    assert(opts.case_number >= 0);

    int current_case = 0;
    file_parser* parser = opts.parser.get();
    game_case gc;

    bool did_run = false;

    while (parser->parse_chunk(gc))
    {
        if (current_case == opts.case_number)
        {
            // Run this case
            did_run = true;

            assert(is_black_white(gc.to_play));
            sumgame sum(gc.to_play);

            for (game* g : gc.games)
            {
                sum.add(g);
            }

            bool outcome = sum.solve();

            cout << outcome << "\n";
            cout << 5.12 << "\n";
        }

        current_case++;
        gc.cleanup_games();
    }

    if (!did_run)
    {
        cerr << "Invalid case number" << endl;
        exit(-1);
    }
}

/*
       file
       case number
       to play
       expected value
   got value
   time
   test case hash
       LINE COUNT
       human readable game representation
   LINE COUNT
   comments


*/
void run_autotests()
{
    // test timeout (ms)
    const int timeout = 500;

    ofstream outfile("out.txt");
    assert(outfile.is_open());

    // Print NSV format
    outfile << "file" << newline;
    outfile << "case number" << newline;
    outfile << "to play" << newline;
    outfile << "expected value" << newline;
    outfile << "LINE COUNT" << newline;
    outfile << "human readable games" << newline;
    outfile << newline;



    for (const filesystem::directory_entry& entry : recursive_directory_iterator("test/input/autotests"))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        const filesystem::path& p = entry.path();

        if (p.extension() != ".test")
        {
            continue;
        }

        const string file_name = p.string();

        cout << entry << endl;

        // Open the file, parse the games
        file_parser* parser = file_parser::from_file(file_name);
        game_case gc;

        int case_number = 0;
        while (parser->parse_chunk(gc))
        {
            outfile << file_name << newline;
            outfile << case_number << newline;
            outfile << color_char(gc.to_play) << newline;
            outfile << test_outcome_to_string(gc.expected_outcome) << newline;

            int line_count = 0;
            string games = human_readable_game_string(gc.games, &line_count);

            outfile << line_count << newline;
            outfile << games << newline;

            // Build command
            string command = "./MCGS --silent --case ";
            command += to_string(case_number) + " --file \"";
            command += file_name + "\"";

            // Create new process

            int up_pipe[2];
            int err_pipe[2];

            if (pipe(up_pipe) != 0 || pipe(err_pipe) != 0)
            {
                cerr << "Failed to create pipes" << endl;
                exit(-1);
            }

            int parent_pid = getpid();
            int fork_id = fork();

            if (fork_id == 0) // middle process
            {
                close(up_pipe[READ_END]);
                close(err_pipe[READ_END]);
                dup2(up_pipe[WRITE_END], STDIN_FILENO);
                dup2(err_pipe[WRITE_END], STDERR_FILENO);

                execl("./MCGS",

                    "./MCGS",

                    "--reaper",
                    to_string(parent_pid).c_str(),

                    "--silent",

                    "--case",
                    to_string(case_number).c_str(),

                    "--file",
                    file_name.c_str(),

                    NULL
                );

                // This should be unreachable
                cerr << "exec() failed" << endl;
                exit(-1);
            }

            // top process




            // End of loop
            outfile << newline;
            case_number++;
            gc.cleanup_games();
        }



        delete parser;

    }




    outfile.close();
}
