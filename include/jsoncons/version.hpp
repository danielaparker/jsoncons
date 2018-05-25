// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_VERSION_HPP
#define JSONCONS_VERSION_HPP

#include <iostream>

namespace jsoncons {
    
struct versioning_info
{
    versioning_info(unsigned int major,
                    unsigned int minor,
                    unsigned int patch)
       : major_(major),
         minor_(minor),
         patch_(patch)
    {}

    unsigned int const major_;
    unsigned int const minor_;
    unsigned int const patch_;

    friend std::ostream& operator<<(std::ostream& os, const versioning_info& ver)
    {
        os << ver.major_ << '.'
           << ver.minor_ << '.'
           << ver.patch_;
        return os;
    }

    versioning_info(const versioning_info&) = default;
    versioning_info() = delete;
    versioning_info& operator=(const versioning_info&) = delete;
}; 

inline
versioning_info version()
{
    static versioning_info ver(0, 105, 0);
    return ver;
}

}

#endif
