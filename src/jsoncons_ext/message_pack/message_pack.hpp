// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_MESSAGE_PACK_MESSAGE_PACK_HPP
#define JSONCONS_MESSAGE_PACK_MESSAGE_PACK_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <jsoncons/json.hpp>

namespace jsoncons { namespace message_pack {

template<class Json>
Json encode_message_pack(std::vector<uint8_t>)
{
}

}}

#endif
