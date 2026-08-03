#include "exit_signal.h"

#include <csignal>

namespace {
bool initialized = false;

extern "C" void signal_handler(int signal)
{
    /*
        NOTE: We can't print a message here -- called functions must be
        async-signal-safe!
    */
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
