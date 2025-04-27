#include "global_options.h"
#include <vector>
#include <algorithm>
#include <sstream>
#include <cstddef>
#include <cstdint>

////////////////////////////////////////////////// Implementation details
namespace {

// This vector isn't a static member of global_option_base, to avoid including
// <vector> in the header
std::vector<global_option_base*> all_options;

} // namespace

global_option_base::global_option_base(const std::string& name, global_summary_enum print_summary)
    : _name(name)
{
    if (print_summary == GLOBAL_SUMMARY_INCLUDE)
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

// Preferred way to initialize global_option. "#" in macro treats value as a string
#define INIT_GLOBAL_WITH_SUMMARY(variable_name, value_type, default_value) \
global_option<value_type> variable_name(std::string(#variable_name), default_value, GLOBAL_SUMMARY_INCLUDE)

#define INIT_GLOBAL_WITHOUT_SUMMARY(variable_name, value_type, default_value) \
global_option<value_type> variable_name(std::string(#variable_name), default_value, GLOBAL_SUMMARY_EXCLUDE)


namespace global {

// These WILL be printed with ./MCGS --print-optimizations
INIT_GLOBAL_WITH_SUMMARY(random_table_seed, uint64_t, 7753);
INIT_GLOBAL_WITH_SUMMARY(subgame_split, bool, true);
INIT_GLOBAL_WITH_SUMMARY(simplify_basic_cgt, bool, true);
INIT_GLOBAL_WITH_SUMMARY(tt_sumgame_idx_bits, size_t, 28);

// These WILL NOT be printed with ./MCGS --print-optimizations
INIT_GLOBAL_WITHOUT_SUMMARY(silence_warnings, bool, false);

} // namespace global
