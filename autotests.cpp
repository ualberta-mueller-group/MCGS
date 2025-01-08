#include "autotests.h"
#include "file_parser.h"
#include "cgt_basics.h"

#include <cerrno>
#include <csignal>
#include <poll.h>
#include <iostream>
#include <fstream>
#include <sys/poll.h>
#include <sys/wait.h>
#include <filesystem>
#include <cassert>
#include "game.h"
#include <string>
#include <vector>
#include <stdio.h>

#include <unistd.h>

#define READ_END 0
#define WRITE_END 1


using namespace std;

constexpr char newline = '\n';


using filesystem::recursive_directory_iterator;
using filesystem::directory_entry;
using filesystem::path;

/*

    file name
    case number
    LINE COUNT
    human readable game form
    to play
    expected outcome

    actual outcome
    time



    LINE COUNT
    included comments

    input hash
*/


string human_readable_representation(const vector<game *>& games, int* line_count)
{
    assert(line_count != nullptr);

    stringstream stream;

    size_t N = games.size();
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

/*
    TODO make this relative to the executable's path, not current working directory...

    or for now, just exit when the first arg isn't ./MCGS

    should print format at the top of the file, like a CSV file
*/
void run_autotests()
{

    ofstream outfile("out.txt");
    assert(outfile.is_open());

    outfile << "file name" << newline;
    outfile << "case number" << newline;
    outfile << "LINE COUNT" << newline;
    outfile << "human readable game representation" << newline;
    outfile << "to play" << newline;
    outfile << "expected outcome" << newline;
    outfile << "got outcome" << newline;
    outfile << "time" << newline;
    outfile << newline;

    // Iterate over all files in input directory
    for (const directory_entry& entry : recursive_directory_iterator("test/input/autotests"))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        const path& file_path = entry.path();

        if (file_path.extension() != ".test")
        {
            continue;
        }

        // Run tests on this file...
        cout << entry << endl;

        game_case gc;
        file_parser* parser = file_parser::from_file(file_path);

        int case_number = 0;
        while (parser->parse_chunk(gc))
        {
            // Print info...
            outfile << file_path << newline;

            outfile << case_number << newline;

            int gamestring_lines = 0;
            string gamestring = human_readable_representation(gc.games, &gamestring_lines);
            outfile << gamestring_lines << newline;

            outfile << gamestring << newline;

            outfile << color_char(gc.to_play) << newline;

            outfile << test_outcome_to_string(gc.expected_outcome) << newline;


            // Invoke subprocess...

            /*
            string command_string = "./MCGS --case " + to_string(case_number);
            FILE* proc = popen(command_string.c_str(), "r");
            cout << "[" << (proc == nullptr) << "]" << newline;

            string proc_result = "";

            char buffer[2];
            while (fgets(buffer, sizeof(buffer), proc))
            {
                proc_result += buffer;
            }

            if (proc_result.size() > 0 && proc_result.back() == '\n')
            {
                proc_result.pop_back();
            }

            pclose(proc);

            outfile << proc_result << newline;
            */


            int down_pipe[2];
            int up_pipe[2];

            if (pipe(down_pipe) != 0 || pipe(up_pipe) != 0)
            {
                throw "Failed to create subprocess pipe...";
            }

            int pid = fork();

            if (pid == 0) // child
            {
                close(down_pipe[WRITE_END]);
                close(up_pipe[READ_END]);

                dup2(down_pipe[READ_END], STDIN_FILENO);
                dup2(up_pipe[WRITE_END], STDOUT_FILENO);

                execlp("./MCGS", "./MCGS", "--case", to_string(case_number).c_str(), NULL);
                throw "execl() call failed";
            }

            // parent
            close(down_pipe[READ_END]);
            close(up_pipe[WRITE_END]);

            // Wait for exit...
            /*
                int exit_status = 0;
                int got_pid = waitpid(pid, &exit_status, 0);
                assert(got_pid == pid);
                assert(exit_status == 0);
            */

            pollfd fds;
            fds.fd = up_pipe[READ_END];
            fds.events = POLLIN;

            poll(&fds, 1, 500);

            if (!(fds.revents & POLLIN))
            {
                cout << "TIMEOUT" << endl;
            }

            string result;

            char buffer[512];
            int bytes_read = read(up_pipe[READ_END], buffer, sizeof(buffer) - 1);
            assert(bytes_read >= 0);
            buffer[bytes_read] = '\0';

            result = buffer;
            if (result.size() > 0 && result.back() == '\n')
            {
                result.pop_back();
            }
            outfile << result << newline;

            close(down_pipe[WRITE_END]);
            close(up_pipe[READ_END]);

            outfile << newline;
            gc.cleanup_games();
            case_number++;
        }

        delete parser;
    }


    outfile << flush;
    outfile.close();
}
