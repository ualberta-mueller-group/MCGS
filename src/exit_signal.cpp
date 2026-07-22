#include "exit_signal.h"

#include <cstring>
#include <iostream>
#include <csignal>

namespace {
bool initialized = false;

extern "C" void signal_handler(int signal)
{
    if (!exit_signal::mcgs_should_stop())
    {
        std::cerr << "Received signal \"" << strsignal(signal)
                  << "\". Attempting safe shutdown..." << std::endl;
    }

    exit_signal::request_mcgs_exit();
}
} // namespace

namespace __exit_signal_impl {
bool _should_stop = false;
} // namespace __exit_signal_impl

namespace exit_signal {
void enable_handlers()
{
    if (initialized)
        return;
    initialized = true;

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
}

bool handlers_are_enabled()
{
    return initialized;
}

} // namespace exit_signal
