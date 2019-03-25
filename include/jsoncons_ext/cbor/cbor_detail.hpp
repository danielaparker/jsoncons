// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_DETAIL_HPP
#define JSONCONS_CBOR_CBOR_DETAIL_HPP

#include <string>
#include <vector>
#include <memory>
#include <iterator> // std::forward_iterator_tag
#include <limits> // std::numeric_limits
#include <utility> // std::move
#include <jsoncons/json.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/config/binary_detail.hpp>

namespace jsoncons { namespace cbor { namespace detail {

enum class cbor_major_type : uint8_t
{
    unsigned_integer = 0x00,
    negative_integer = 0x01,
    byte_string = 0x02,
    text_string = 0x03,
    array = 0x04,
    map = 0x05,   
    semantic_tag = 0x06,
    simple = 0x7
};

namespace additional_info
{
    const uint8_t indefinite_length = 0x1f;
}

}}}

#endif
