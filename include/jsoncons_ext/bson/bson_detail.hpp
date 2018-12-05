// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BSON_BSON_DETAIL_HPP
#define JSONCONS_BSON_BSON_DETAIL_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <limits>
#include <cassert>
#include <jsoncons/config/binary_detail.hpp>

namespace jsoncons { namespace bson {

namespace bson_format
{
    const uint8_t double_cd = 0x01;
    const uint8_t string_cd = 0x02;
    const uint8_t document_cd = 0x03;
    const uint8_t array_cd = 0x04;
    const uint8_t binary_cd = 0x05;
    const uint8_t object_id_cd = 0x07;
    const uint8_t bool_cd = 0x08;
    const uint8_t datetime_cd = 0x09;
    const uint8_t null_cd = 0x0a;
    const uint8_t regex_cd = 0x0b;
    const uint8_t javascript_cd = 0x0d;
    const uint8_t javascript_with_scope_cd = 0x0f;
    const uint8_t int32_cd = 0x10;
    const uint8_t timestamp_cd = 0x11; // uint64
    const uint8_t int64_cd = 0x12;
    const uint8_t decimal128_cd = 0x13;
    const uint8_t min_key_cd = 0xff;
    const uint8_t max_key_cd = 0x7f;
}
 
class bson_error : public std::invalid_argument, public virtual json_exception
{
public:
    explicit bson_error(size_t pos) noexcept
        : std::invalid_argument("")
    {
        buffer_.append("Error decoding a message pack at position ");
        buffer_.append(std::to_string(pos));
    }
    ~bson_error() noexcept
    {
    }
    const char* what() const noexcept override
    {
        return buffer_.c_str();
    }
private:
    std::string buffer_;
};

#if !defined(JSONCONS_NO_DEPRECATED)
typedef bson_error bson_decode_error;
#endif

enum class bson_structure_type {document, array};

}}

#endif
