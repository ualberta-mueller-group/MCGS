#include "global_options.h"
#include <vector>
#include <algorithm>

////////////////////////////////////////////////// Implementation details
namespace {

// This vector isn't a static member of global_option_base, to avoid including
// <vector> in the header
std::vector<global_option_base*> all_options;

} // namespace

global_option_base::global_option_base()
{
    all_options.push_back(this);
}

std::string global_option_base::get_summary_all()
{
    std::string summary;

    // Sort options by their names
    std::sort(all_options.begin(), all_options.end(),
    [](const global_option_base* opt1, const global_option_base* opt2) -> bool
    {
        return opt1->name() < opt2->name();
    });

    // Now concatenate all of their summaries
    const size_t N = all_options.size();
    for (size_t i = 0; i < N; i++)
    {
        const global_option_base* option = all_options[i];
        summary += option->get_summary_single();

        if (i + 1 < N)
            summary += "\n";
    }

    return summary;
}

//////////////////////////////////////////////////////////// Options

// Preferred way to initialize global_option
#define INIT_GLOBAL(variable_name, value_type, default_value) \
global_option<value_type> variable_name(std::string(#variable_name), default_value)

namespace global_options {

INIT_GLOBAL(some_value1, int, 5);
INIT_GLOBAL(some_value3, double, 1.1);
INIT_GLOBAL(some_value2, float, 5.5);

} // namespace global_options
