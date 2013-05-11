#include <string>
#include <utility>
#include <vector>
#include <assert.h>
#include <cstdlib>
#include <cerrno>
#include "jsoncons/json_parser.hpp"
#include "jsoncons/json_variant.hpp"

namespace jsoncons {
 
void append_codepoint_to_string(unsigned int cp, std::string& s)
{
    if (cp <= 0x7f)
    {
        s.push_back(static_cast<char>(cp));
    }
    else if (cp <= 0x7FF)
    {
        s.push_back(static_cast<char>(0xC0 | (0x1f & (cp >> 6))));
        s.push_back(static_cast<char>(0x80 | (0x3f & cp)));
    }
    else if (cp <= 0xFFFF)
    {
        s.push_back(0xE0 | static_cast<char>((0xf & (cp >> 12))));
        s.push_back(0x80 | static_cast<char>((0x3f & (cp >> 6))));
        s.push_back(static_cast<char>(0x80 | (0x3f & cp)));
    }
    else if (cp <= 0x10FFFF)
    {
        s.push_back(static_cast<char>(0xF0 | (0x7 & (cp >> 18))));
        s.push_back(static_cast<char>(0x80 | (0x3f & (cp >> 12))));
        s.push_back(static_cast<char>(0x80 | (0x3f & (cp >> 6))));
        s.push_back(static_cast<char>(0x80 | (0x3f & cp)));
    }
}

}

