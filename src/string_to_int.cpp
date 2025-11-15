#include "string_to_int.h"

#include <limits>
#include <string>
#include <optional>
#include "cgt_basics.h"
#include "utilities.h"

////////////////////////////////////////////////// short
short str_to_sh(const std::string& str)
{
    THROW_ASSERT(is_int(str));

    const int val = std::stoi(str);

    THROW_ASSERT(
            std::numeric_limits<short>::min() <= val && //
            val <= std::numeric_limits<short>::max() //
            );

    return static_cast<short>(val);
}

std::optional<short> str_to_sh_opt(const std::string& str)
{
    if (!is_int(str))
        return {};

    const int val = std::stoi(str);

    THROW_ASSERT(
            std::numeric_limits<short>::min() <= val && //
            val <= std::numeric_limits<short>::max() //
            );

    return static_cast<short>(val);
}

////////////////////////////////////////////////// unsigned short
unsigned short str_to_ush(const std::string& str)
{
    THROW_ASSERT(is_unsigned_int(str));

    const unsigned long val = std::stoul(str);

    THROW_ASSERT(val <= std::numeric_limits<unsigned short>::max());

    return static_cast<unsigned short>(val);
}

std::optional<unsigned short> str_to_ush_opt(const std::string& str)
{
    if (!is_unsigned_int(str))
        return {};

    const unsigned long val = std::stoul(str);

    THROW_ASSERT(val <= std::numeric_limits<unsigned short>::max());

    return static_cast<unsigned short>(val);
}


