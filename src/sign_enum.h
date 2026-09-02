#pragma once

#include <string>

enum sign_enum
{
    SIGN_POSITIVE,
    SIGN_NEGATIVE
};

std::string sign_enum_to_string(sign_enum sign_type);
