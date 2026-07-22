#pragma once

// NOLINTBEGIN(readability-identifier-naming)

namespace __exit_signal_impl {
extern bool _should_stop;
} // namespace __exit_signal_impl

// NOLINTEND(readability-identifier-naming)

namespace exit_signal {
inline bool mcgs_should_stop()
{
    return __exit_signal_impl::_should_stop;
}

inline void request_mcgs_exit()
{
    __exit_signal_impl::_should_stop = true;
}

void enable_handlers();
bool handlers_are_enabled();

} // namespace exit_signal
