// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version
    
#ifndef JSONCONS_GENERIC_TOKENIZER_HPP
#define JSONCONS_GENERIC_TOKENIZER_HPP

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_error.hpp>
#include <system_error>

namespace jsoncons {

enum class generic_event_kind : uint8_t
{
    string_value = 1,       // 0001
    byte_string_value = 2,  // 0010
    null_value = 3,         // 0011
    bool_value = 4,         // 0100
    int64_value = 5,        // 0101
    uint64_value = 6,       // 0110
    half_value = 8,         // 1000
    double_value = 9,       // 1001
    begin_map = 13,         // 1101
    end_map = 7,            // 0111    
    begin_array = 14,       // 1110
    end_array = 15          // 1111
};

inline bool is_begin_container(generic_event_kind event_kind) noexcept
{
    static const uint8_t mask{ uint8_t(generic_event_kind::begin_map) & uint8_t(generic_event_kind::begin_array) };
    return (uint8_t(event_kind) & mask) == mask;
}

inline bool is_end_container(generic_event_kind event_kind) noexcept
{
    static const uint8_t mask{ uint8_t(generic_event_kind::end_map) & uint8_t(generic_event_kind::end_array) };
    return (uint8_t(event_kind) & mask) == mask;
}

template <typename CharT>
std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, generic_event_kind tag)
{
    static constexpr const CharT* begin_array_name = JSONCONS_CSTRING_CONSTANT(CharT, "begin_array");
    static constexpr const CharT* end_array_name = JSONCONS_CSTRING_CONSTANT(CharT, "end_array");
    static constexpr const CharT* begin_object_name = JSONCONS_CSTRING_CONSTANT(CharT, "begin_map");
    static constexpr const CharT* end_object_name = JSONCONS_CSTRING_CONSTANT(CharT, "end_map");
    static constexpr const CharT* key_name = JSONCONS_CSTRING_CONSTANT(CharT, "key");
    static constexpr const CharT* string_value_name = JSONCONS_CSTRING_CONSTANT(CharT, "string_value");
    static constexpr const CharT* byte_string_value_name = JSONCONS_CSTRING_CONSTANT(CharT, "byte_string_value");
    static constexpr const CharT* null_value_name = JSONCONS_CSTRING_CONSTANT(CharT, "null_value");
    static constexpr const CharT* bool_value_name = JSONCONS_CSTRING_CONSTANT(CharT, "bool_value");
    static constexpr const CharT* uint64_value_name = JSONCONS_CSTRING_CONSTANT(CharT, "uint64_value");
    static constexpr const CharT* int64_value_name = JSONCONS_CSTRING_CONSTANT(CharT, "int64_value");
    static constexpr const CharT* half_value_name = JSONCONS_CSTRING_CONSTANT(CharT, "half_value");
    static constexpr const CharT* double_value_name = JSONCONS_CSTRING_CONSTANT(CharT, "double_value");

    switch (tag)
    {
        case generic_event_kind::begin_array:
        {
            os << begin_array_name;
            break;
        }
        case generic_event_kind::end_array:
        {
            os << end_array_name;
            break;
        }
        case generic_event_kind::begin_map:
        {
            os << begin_object_name;
            break;
        }
        case generic_event_kind::end_map:
        {
            os << end_object_name;
            break;
        }
        case generic_event_kind::string_value:
        {
            os << string_value_name;
            break;
        }
        case generic_event_kind::byte_string_value:
        {
            os << byte_string_value_name;
            break;
        }
        case generic_event_kind::null_value:
        {
            os << null_value_name;
            break;
        }
        case generic_event_kind::bool_value:
        {
            os << bool_value_name;
            break;
        }
        case generic_event_kind::int64_value:
        {
            os << int64_value_name;
            break;
        }
        case generic_event_kind::uint64_value:
        {
            os << uint64_value_name;
            break;
        }
        case generic_event_kind::half_value:
        {
            os << half_value_name;
            break;
        }
        case generic_event_kind::double_value:
        {
            os << double_value_name;
            break;
        }
    }
    return os;
}

template <typename CharT>
class basic_generic_cursor
{
public:
    virtual bool done() const = 0;

    virtual void next() = 0;

    virtual void next(std::error_code& ec) = 0;

    virtual generic_event_kind event_kind() = 0;

    //virtual void next(std::error_code& ec) = 0;
};

struct from_json_result
{
    json_errc ec{};

    explicit operator bool() const noexcept
    {
        return ec == json_errc{};
    }

    bool operator==(const from_json_result& other)
    {
        return ec == other.ec;
    }

    friend bool operator==(const from_json_result& lhs, const from_json_result& rhs)
    {
        return lhs == rhs;
    }
};

} // namespace jsoncons

#endif // JSONCONS_GENERIC_TOKENIZER_HPP

