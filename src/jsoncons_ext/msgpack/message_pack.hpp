// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BINARY_MESSAGE_PACK_HPP
#define JSONCONS_BINARY_MESSAGE_PACK_HPP

#include <jsoncons/detail/jsoncons_config.hpp>
#include <jsoncons_ext/msgpack/msgpack.hpp>

namespace jsoncons { namespace msgpack {
  
#if !defined(JSONCONS_NO_DEPRECATED)
template<class Json>
std::vector<uint8_t> encode_message_pack(const Json& j)
{
    return encode_msgpack(j);
}

template<class Json>
Json decode_message_pack(const std::vector<uint8_t>& v)
{
    return decode_msgpack<Json>(v);
}
#endif

}}

#endif
