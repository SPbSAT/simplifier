#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace csat::utils::string_utils
{

/**
 * Trims spaces from start and end of a `str`.
 * @param str -- string view, to be trimmed.
 */
inline void trimSpaces(std::string_view& str)
{
    size_t const _b = str.find_first_not_of(' ');
    size_t const _e = str.find_last_not_of(' ');
    if (_b != std::string::npos)
    {
        str = str.substr(_b, _e - _b + 1);
    }
    else if (_e != std::string::npos)
    {
        str = str.substr(0, _e);
    }
    else
    {
        str = str.substr(0, 0);
    }
}

} // csat::utils::string_utils namespace