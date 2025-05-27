// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DECODE_JSON_HPP
#define JSONCONS_DECODE_JSON_HPP

#include <jsoncons/reflect/decode_json.hpp>

namespace jsoncons {

template <typename T, typename... Args>
T decode_json(Args&& ... args)
{
    return jsoncons::reflect::decode_json<T>(std::forward<Args>(args)...); 
}

} // namespace jsoncons

#endif // JSONCONS_DECODE_JSON_HPP

