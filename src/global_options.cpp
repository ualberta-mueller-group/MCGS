#include "global_options.h"
#include <vector>
#include <algorithm>

////////////////////////////////////////////////// Implementation details
namespace {

// This vector isn't a static member of global_option_base, to avoid including
// <vector> in the header
std::vector<global_option_base*> all_options;

} // namespace

global_option_base::global_option_base(const std::string& name)
    : _name(name)
{
    all_options.push_back(this);
}

global_option_base::~global_option_base()
{

}

std::string global_option_base::flag() const
{
    return "--" + _name_with_dashes();
}

std::string global_option_base::no_flag() const
{
    return "--no-" + _name_with_dashes();
}

std::string global_option_base::get_summary_single() const
{
    std::stringstream str;
    str << name() << ": " << get_str();
    return str.str();
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

inline std::string global_option_base::_name_with_dashes() const
{
    std::string name_modified;
    name_modified.reserve(_name.size());

    for (const char& c : _name)
    {
        if (c == '_')
            name_modified.push_back('-');
        else
            name_modified.push_back(c);
    }

    return name_modified;
}

//////////////////////////////////////////////////////////// Options

// Preferred way to initialize global_option
#define INIT_GLOBAL(variable_name, value_type, default_value) \
global_option<value_type> variable_name(std::string(#variable_name), default_value)

namespace global {

INIT_GLOBAL(random_table_seed, uint64_t, 7753);
INIT_GLOBAL(subgame_split, bool, true);
INIT_GLOBAL(simplify_basic_cgt, bool, true);
INIT_GLOBAL(tt_sumgame_idx_bits, size_t, 28);

} // namespace global_options
