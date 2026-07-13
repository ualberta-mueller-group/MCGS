#include "exit_signal.h"

#include <iostream>
#include <csignal>

namespace {
extern "C" void signal_handler(int signal)
{
    std::cout << "GOT SIGNAL: " << signal << std::endl;
    exit_signal::request_mcgs_exit();
}
} // namespace

namespace __exit_signal_impl {
bool _should_stop = false;
} // namespace __exit_signal_impl

namespace exit_signal {
void init_signal_handler()
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
}

} // namespace exit_signal
