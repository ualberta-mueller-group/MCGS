#include "sign_enum.h"

#include <cassert>

std::string sign_enum_to_string(sign_enum sign_type)
{
    switch (sign_type)
    {
        case SIGN_POSITIVE:
            return "POSITIVE";
        case SIGN_NEGATIVE:
            return "NEGATIVE";
    }

    assert(false);
}

