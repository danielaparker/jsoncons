// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONFWD_HPP
#define JSONCONS_JSONFWD_HPP

#include <memory>

namespace jsoncons {

struct sorted_policy;
                        
template <class CharT, 
          class ImplementationPolicy = sorted_policy, 
          class Allocator = std::allocator<CharT>>
class basic_json;

}

#endif
