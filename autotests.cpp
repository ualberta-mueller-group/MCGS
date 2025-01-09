#include "autotests.h"
#include <csignal>
#include <cstdio>
#include <cstdlib>
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
#include <string>
#include <sys/poll.h>
#include <sys/wait.h>
#include <unistd.h>
#include "misc_constants.h"

using namespace std;

using filesystem::recursive_directory_iterator;

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

            cout << (outcome ? "Win" : "Loss") << "\n";
            cout << 5.12 << "\n";
            cout << flush;

            gc.cleanup_games();
            break;
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
    const int timeout = 2000;

    ofstream outfile("out.txt");
    assert(outfile.is_open());

    // Print NSV format
    outfile << "file" << newline;
    outfile << "case number" << newline;
    outfile << "to play" << newline;
    outfile << "expected value" << newline;
    outfile << "LINE COUNT" << newline;
    outfile << "human readable games" << newline;
    outfile << "no crash/crash" << newline;
    outfile << "completed/dnf" << newline;
    outfile << "search result" << newline;
    outfile << "time (ms)" << newline;

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
            outfile << newline;
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
                dup2(up_pipe[WRITE_END], STDOUT_FILENO);
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
            close(up_pipe[WRITE_END]);
            close(err_pipe[WRITE_END]);

            int flags = fcntl(up_pipe[READ_END], F_GETFL, 0);
            fcntl(up_pipe[READ_END], F_SETFL, flags | O_NONBLOCK);

            pollfd pfd;
            pfd.fd = up_pipe[READ_END];
            pfd.events = POLLIN;

            poll(&pfd, 1, timeout);

            bool timed_out = false;
            string data;

            if ((pfd.revents & POLLIN) == 0)
            {
                timed_out = true;
            } else
            {
                char buffer[2];
                int bytes_read;
                while ((bytes_read = read(up_pipe[READ_END], buffer, sizeof(buffer) - 1)) > 0)
                {
                    buffer[bytes_read] = '\0';
                    data += buffer;
                }
            }

            bool test_crashed = false;

            int middle_status;
            int returned_pid = waitpid(fork_id, &middle_status, WNOHANG);

            if (returned_pid == -1 || ( (returned_pid == fork_id) && (WEXITSTATUS(middle_status) != 0) ))
            {
                test_crashed = true;
            }

            outfile << (test_crashed ? "CRASHED" : "NO CRASH") << newline;
            outfile << (timed_out ? "TIMED OUT" : "COMPLETED") << newline;

            if (!timed_out && !test_crashed)
            {
                stringstream stream(data);

                string line;
                
                assert(getline(stream, line));
                outfile << line << newline;
                assert(getline(stream, line));
                outfile << line << newline;

            } else
            {
                outfile << "Unknown" << newline;
                outfile << "DNF" << newline;
            }


            close(up_pipe[READ_END]);
            close(err_pipe[READ_END]);

            kill(fork_id, SIGQUIT);
            int status;
            waitpid(fork_id, &status, 0); // avoid zombie process


            // End of loop
            case_number++;
            gc.cleanup_games();
        }

        delete parser;
    }

    outfile.close();
}
