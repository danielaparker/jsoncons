// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_TYPE_HPP
#define JSONCONS_JSON_TYPE_HPP

namespace jsoncons {

    enum class json_type : uint8_t 
    {
        null_value,
        bool_value,
        int64_value,
        uint64_value,
        half_value,
        double_value,
        string_value,
        byte_string_value,
        array_value,
        object_value
    };

} // jsoncons

#endif
