#include "reaper.h"
#include "utilities.h"
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <iostream>
#include "misc_constants.h"
#include <cassert>

#include <csignal>
#include <sys/poll.h>
#include <sys/wait.h>


using namespace std;

int kill_pid = 0;

void kill_bottom()
{
    if (kill_pid != 0)
    {
        kill(kill_pid, SIGKILL);
        kill_pid = 0;
    }
}

void sigquit_handler(int sig)
{
    kill_bottom();
}

void reaper(int argc, char** argv)
{
    int parent_pid = 0;

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
            break;
        }
    }

    if (parent_pid == 0) 
    {
        cerr << "Invalid reaper() args" << endl;
        exit(-1);
    }

    // Start bottom process
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
        dup2(up_pipe[WRITE_END], STDOUT_FILENO);
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
    kill_pid = fork_id;
    atexit(kill_bottom);
    signal(SIGQUIT, sigquit_handler);

    close(up_pipe[WRITE_END]);
    close(err_pipe[WRITE_END]);

    // Don't block when reading pipes...
    {
        int flags = fcntl(up_pipe[READ_END], F_GETFL, 0);
        fcntl(up_pipe[READ_END], F_SETFL, flags | O_NONBLOCK);
    }

    {
        int flags = fcntl(err_pipe[READ_END], F_GETFL, 0);
        fcntl(err_pipe[READ_END], F_SETFL, flags | O_NONBLOCK);
    }


    // Forward data upwards
    while (true)
    {
        pollfd pfds[2];

        pfds[0].fd = up_pipe[READ_END];
        pfds[0].events = POLLIN;

        pfds[1].fd = err_pipe[READ_END];
        pfds[1].events = POLLIN;

        poll(pfds, sizeof(pfds) / sizeof(pfds[0]), 1000);

        if ((pfds[0].revents & POLLIN) != 0)
        {
            char buffer[2];
            int bytes_read;
            while ((bytes_read = read(up_pipe[READ_END], buffer, sizeof(buffer) - 1)) > 0)
            {
                buffer[bytes_read] = '\0';
                cout << buffer;
            }
            cout << flush;
        }

        if ((pfds[1].revents & POLLIN) != 0)
        {
            char buffer[2];
            int bytes_read;
            while ((bytes_read = read(err_pipe[READ_END], buffer, sizeof(buffer) - 1)) > 0)
            {
                buffer[bytes_read] = '\0';
                cerr << buffer;
            }
            cerr << flush;
        }

        // Check if parent exists
        if (kill(parent_pid, 0) == -1)
        {
            kill_bottom();
            exit(0);
        }
    
        int bottom_status;
        int returned_pid = waitpid(fork_id, &bottom_status, WNOHANG);

        // returned_pid being 0 means process still running

        // returned_pid being -1 means error with waitpid()
        if (returned_pid == -1)
        {
            kill_bottom();
            cerr << "waitpid() error" << endl;
            exit(-1);
        }

        // returned_pid == fork_id means process state changed
        if (returned_pid == fork_id)
        {
            if (WIFEXITED(bottom_status))
            {
                int exit_status = WEXITSTATUS(bottom_status);
                exit(exit_status);
            } else
            {
                exit(-1);
            }

        }
    }
}

