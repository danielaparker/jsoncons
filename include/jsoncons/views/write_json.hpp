// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_WRITE_JSON_HPP
#define JSONCONS_WRITE_JSON_HPP

//#include <jsoncons/json_reader.hpp>
#include <jsoncons/views/binary.hpp>

namespace jsoncons {


enum class write_json_flags : uint32_t
{
    none = 0,
    pretty = 1 << 0,  // 4 space indent
    escape_unicode = 1 << 1,
    escape_slashes = 1 << 2,
    allow_inf_and_nan = 1 << 3,
    inf_and_nan_as_null = 1 << 4,
    allow_invalid_unicode = 1 << 5,
    pretty_two_spaces = 1 << 6,
    newline_at_end = 1 << 7
};

JSONCONS_FORCEINLINE write_json_flags operator~(write_json_flags a)
{
    return static_cast<write_json_flags>(~static_cast<uint32_t>(a));
}

JSONCONS_FORCEINLINE write_json_flags operator&(write_json_flags a, write_json_flags b)
{
    return static_cast<write_json_flags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

JSONCONS_FORCEINLINE write_json_flags operator^(write_json_flags a, write_json_flags b)
{
    return static_cast<write_json_flags>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
}

JSONCONS_FORCEINLINE write_json_flags operator|(write_json_flags a, write_json_flags b)
{
    return static_cast<write_json_flags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

JSONCONS_FORCEINLINE write_json_flags operator&=(write_json_flags& a, write_json_flags b)
{
    a = a & b;
    return a;
}

JSONCONS_FORCEINLINE write_json_flags operator^=(write_json_flags& a, write_json_flags b)
{
    a = a ^ b;
    return a;
}

JSONCONS_FORCEINLINE write_json_flags operator|=(write_json_flags& a, write_json_flags b)
{
    a = a | b;
    return a;
}

enum class write_json_errc
{
    success = 0,
    invalid_parameter,
    memory_allocation,
    invalid_value_type,
    nan_or_inf,
    file_open,
    file_close,
    invalid_utf8_encoding
};


class write_json_error_category_impl
   : public std::error_category
{
public:
    const char* name() const noexcept final
    {
        return "jsoncons/write_json";
    }
    std::string message(int ev) const final
    {
        switch (static_cast<write_json_errc>(ev))
        {
            case write_json_errc::memory_allocation:
                return "Memory allocation failed";
            case write_json_errc::invalid_parameter:
                return "Memory allocation failed";
            case write_json_errc::invalid_value_type:
                return "Invalid JSON value type";
            case write_json_errc::nan_or_inf:
                return "Memory allocation failed";
            case write_json_errc::file_open:
                return "File opening failed";
            case write_json_errc::file_close:
                return "File closing failed";
            case write_json_errc::invalid_utf8_encoding:
                return "Invalid utf-8 encoding in string";
            default:
                return "Unknown JSON write error";
            }
    }
};

JSONCONS_FORCEINLINE
const std::error_category& write_json_error_category()
{
  static write_json_error_category_impl instance;
  return instance;
}

JSONCONS_FORCEINLINE 
std::error_code make_error_code(write_json_errc result)
{
    return std::error_code(static_cast<int>(result),write_json_error_category());
}


struct write_json_result
{
    write_json_errc ec;

    constexpr explicit operator bool() const noexcept
    {
        return ec == write_json_errc{};
    }

    friend bool operator==(const write_json_result&,
        const write_json_result&) = default;
};

write_json_result write_json(const json_view& val, std::string& buffer, 
    write_json_flags flags = write_json_flags{});

/** Character encode type, if (type > char_enc_err_1) bytes = type / 2; */
using char_enc_type = uint8_t;

inline constexpr uint8_t char_enc_cpy_1 = 0; /* 1-byte UTF-8, copy. */
inline constexpr uint8_t char_enc_err_1 = 1; /* 1-byte UTF-8, error. */
inline constexpr uint8_t char_enc_esc_a = 2; /* 1-byte ASCII, escaped as '\x'. */
inline constexpr uint8_t char_enc_esc_1 = 3; /* 1-byte UTF-8, escaped as '\uXXXX'. */
inline constexpr uint8_t char_enc_cpy_2 = 4; /* 2-byte UTF-8, copy. */
inline constexpr uint8_t char_enc_esc_2 = 5; /* 2-byte UTF-8, escaped as '\uXXXX'. */
inline constexpr uint8_t char_enc_cpy_3 = 6; /* 3-byte UTF-8, copy. */
inline constexpr uint8_t char_enc_esc_3 = 7; /* 3-byte UTF-8, escaped as '\uXXXX'. */
inline constexpr uint8_t char_enc_cpy_4 = 8; /* 4-byte UTF-8, copy. */
inline constexpr uint8_t char_enc_esc_4 = 9; /* 4-byte UTF-8, escaped as '\uXXXX\uXXXX'. */

const char_enc_type* get_enc_table_with_flag(write_json_flags flags);

uint8_t *write_string(uint8_t *cur, bool esc, bool inv,
                               const uint8_t *str, std::size_t str_len,
                               const char_enc_type *enc_table); 

uint8_t *write_u64(uint64_t val, uint8_t *buf);

uint8_t *write_string_noesc(uint8_t *cur, const uint8_t *str, std::size_t str_len);

uint8_t *write_f64_raw(uint8_t *buf, uint64_t raw, write_json_flags flags);

uint8_t *write_null(uint8_t *cur);

uint8_t *write_bool(uint8_t *cur, bool val);

/** Write indent (requires level x 4 bytes buffer).
    Param spaces should not larger than 4. */
inline constexpr uint8_t *write_indent(uint8_t *cur, std::size_t level, std::size_t spaces) 
{
    while (level-- > 0) {
        utility::byte_copy_4(cur, "    ");
        cur += spaces;
    }
    return cur;
}

} // namespace jsoncons

#endif

