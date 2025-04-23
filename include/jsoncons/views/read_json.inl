// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_READ_JSON_INL
#define JSONCONS_READ_JSON_INL

#include <jsoncons/views/json_ref.hpp>
#include <jsoncons/views/read_json.hpp>
#include <jsoncons/views/binary.hpp>
#include <jsoncons/views/floating_point.hpp>
#include <string_view>
#include <iterator>
#include <stdexcept>
#include <limits>
#include <cmath>
#include <vector>

namespace jsoncons {

    /*==============================================================================
     * Compile-time Options
     *============================================================================*/

     /*
      Define as 1 to disable the fast floating-point number conversion in yyjson,
      and use libc's `strtod/snprintf` instead.

      This will reduce the binary size by about 30%, but significantly slow down the
      floating-point read/write speed.
      */
#ifndef YYJSON_DISABLE_FAST_FP_CONV
#endif

      /*
       Define as 1 to disable non-standard JSON support at compile-time:
          - Reading and writing inf/nan literal, such as `NaN`, `-Infinity`.
          - Single line and multiple line comments.
          - Single trailing comma at the end of an object or array.
          - Invalid unicode in string value.

       This will also invalidate these run-time options:
          - read_json_flags::allow_inf_and_nan
          - read_json_flags::allow_comments
          - read_json_flags::allow_trailing_commas
          - read_json_flags::allow_invalid_unicode

       This will reduce the binary size by about 10%, and speed up the reading and
       writing speed by about 2% to 6%.
       */

        /*
        Define as 1 to disable UTF-8 validation at compile time.

        If all input strings are guaranteed to be valid UTF-8 encoding (for example,
        some language's String object has already validated the encoding), using this
        flag can avoid redundant UTF-8 validation in yyjson.

        This flag can speed up the reading and writing speed of non-ASCII encoded
        strings by about 3% to 7%.

        Note: If this flag is used while passing in illegal UTF-8 strings, the
        following errors may occur:
        - Escaped characters may be ignored when parsing JSON strings.
        - Ending quotes may be ignored when parsing JSON strings, causing the string
          to be concatenated to the next value.
        */
#ifndef YYJSON_DISABLE_UTF8_VALIDATION
#endif

        /*
         Define as 1 to indicate that the target architecture does not support unaligned
         memory access. Please refer to the comments in the C file for details.
         */
#ifndef YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
#endif

         /* Define as 1 to export symbols when building this library as Windows DLL. */
#ifndef YYJSON_EXPORTS
#endif

/* Define as 1 to import symbols when using this library as Windows DLL. */
#ifndef YYJSON_IMPORTS
#endif

/* Define as 1 to include <stdint.h> for compiler which doesn't support C99. */
#ifndef YYJSON_HAS_STDINT_H
#endif

/* Define as 1 to include <stdbool.h> for compiler which doesn't support C99. */
#ifndef YYJSON_HAS_STDBOOL_H
#endif

/*==============================================================================
 * JSON Character Matcher
 *============================================================================*/

/** Character type */

/** Whitespace character: ' ', '\\t', '\\n', '\\r'. */
inline constexpr uint8_t char_type_space      = 1 << 0;

/** Number character: '-', [0-9]. */
inline constexpr uint8_t char_type_number     = 1 << 1;

/** JSON Escaped character: '"', '\', [0x00-0x1F]. */
inline constexpr uint8_t char_type_esc_ascii  = 1 << 2;

/** Non-ASCII character: [0x80-0xFF]. */
inline constexpr uint8_t char_type_non_ascii  = 1 << 3;

/** JSON container character: '{', '['. */
inline constexpr uint8_t char_type_container  = 1 << 4;

/** Comment character: '/'. */
inline constexpr uint8_t char_type_comment    = 1 << 5;

/** Line end character: '\\n', '\\r', '\0'. */
inline constexpr uint8_t char_type_line_end   = 1 << 6;

/** Hexadecimal numeric character: [0-9a-fA-F]. */
inline constexpr uint8_t char_type_hex        = 1 << 7;

/** Character type table (generate with misc/make_tables.c) */
inline constexpr uint8_t char_table[256] = {
    0x44, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x05, 0x45, 0x04, 0x04, 0x45, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x20,
    0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82,
    0x82, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x10, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};

/** Match a character with specified type. */
JSONCONS_FORCE_INLINE bool char_is_type(uint8_t c, uint8_t type) {
    return (char_table[c] & type) != 0;
}

/** Match a whitespace: ' ', '\\t', '\\n', '\\r'. */
JSONCONS_FORCE_INLINE bool char_is_space(uint8_t c) {
    return char_is_type(c, (uint8_t)char_type_space);
}

/** Match a whitespace or comment: ' ', '\\t', '\\n', '\\r', '/'. */
JSONCONS_FORCE_INLINE bool char_is_space_or_comment(uint8_t c) {
    return char_is_type(c, (uint8_t)(char_type_space | char_type_comment));
}

/** Match a JSON number: '-', [0-9]. */
JSONCONS_FORCE_INLINE bool char_is_number(uint8_t c) {
    return char_is_type(c, (uint8_t)char_type_number);
}

/** Match a JSON container: '{', '['. */
JSONCONS_FORCE_INLINE bool char_is_container(uint8_t c) {
    return char_is_type(c, (uint8_t)char_type_container);
}

/** Match a stop character in ASCII string: '"', '\', [0x00-0x1F,0x80-0xFF]. */
JSONCONS_FORCE_INLINE bool char_is_ascii_stop(uint8_t c) {
    return char_is_type(c, (uint8_t)(char_type_esc_ascii |
                                       char_type_non_ascii));
}

/** Match a line end character: '\\n', '\\r', '\0'. */
JSONCONS_FORCE_INLINE bool char_is_line_end(uint8_t c) {
    return char_is_type(c, (uint8_t)char_type_line_end);
}

/** Match a hexadecimal numeric character: [0-9a-fA-F]. */
JSONCONS_FORCE_INLINE bool char_is_hex(uint8_t c) {
    return char_is_type(c, (uint8_t)char_type_hex);
}

/**
 Skips spaces and comments as many as possible.
 
 It will return false in these cases:
    1. No character is skipped. The 'end' pointer is set as input cursor.
    2. A multiline comment is not closed. The 'end' pointer is set as the head
       of this comment block.
 */

inline read_json_result skip_spaces_and_comments(uint8_t* hdr, jsoncons::read_json_errc& ec) 
{
    uint8_t *cur = hdr;
    while (true) {
        if (utility::byte_match_2(cur, "/*")) {
            hdr = cur;
            cur += 2;
            while (true) {
                if (utility::byte_match_2(cur, "*/")) {
                    cur += 2;
                    break;
                }
                if (*cur == 0) {
                    return read_json_result{hdr, read_json_errc::unclosed_multiline_comment};
                }
                cur++;
            }
            continue;
        }
        if (utility::byte_match_2(cur, "//")) {
            cur += 2;
            while (!char_is_line_end(*cur)) cur++;
            continue;
        }
        if (char_is_space(*cur)) {
            cur += 1;
            while (char_is_space(*cur)) cur++;
            continue;
        }
        break;
    }
    return read_json_result{cur, read_json_errc{}};
}

/**
 Check truncated string.
 Returns true if `cur` match `str` but is truncated.
 */
JSONCONS_FORCE_INLINE bool is_truncated_str(uint8_t *cur, uint8_t *end,
    const char *str,
    bool case_sensitive) 
{
    std::size_t len = strlen(str);
    if (cur + len <= end || end <= cur) return false;
    if (case_sensitive) {
        return memcmp(cur, str, (std::size_t)(end - cur)) == 0;
    }
    for (; cur < end; cur++, str++) {
        if ((*cur != (uint8_t)*str) && (*cur != (uint8_t)*str - 'a' + 'A')) {
            return false;
        }
    }
    return true;
}

/**
 Check truncated JSON on parsing errors.
 Returns true if the input is valid but truncated.
 */
JSONCONS_FORCE_INLINE bool is_truncated_end(uint8_t *hdr, uint8_t *cur, uint8_t *end,
    read_json_errc code,
    read_json_flags flags) 
{
    if (cur >= end) return true;
    if (code == read_json_errc::invalid_literal) {
        if (is_truncated_str(cur, end, "true", true) ||
            is_truncated_str(cur, end, "false", true) ||
            is_truncated_str(cur, end, "null", true)) {
            return true;
        }
    }
    if (code == read_json_errc::unexpected_character ||
        code == read_json_errc::invalid_number ||
        code == read_json_errc::invalid_literal) {
        if ((flags & read_json_flags::allow_inf_and_nan) != read_json_flags{}) {
            if (*cur == '-') cur++;
            if (is_truncated_str(cur, end, "infinity", false) ||
                is_truncated_str(cur, end, "nan", false)) {
                return true;
            }
        }
    }
    if (code == read_json_errc::unexpected_content) {
        if ((flags & read_json_flags::allow_inf_and_nan) != read_json_flags{}) {
            if (hdr + 3 <= cur &&
                is_truncated_str(cur - 3, end, "infinity", false)) {
                return true; /* e.g. infin would be read as inf + in */
            }
        }
    }
    if (code == read_json_errc::invalid_string) {
        std::size_t len = (std::size_t)(end - cur);
        
        /* unicode escape sequence */
        if (*cur == '\\') {
            if (len == 1) return true;
            if (len <= 5) {
                if (*++cur != 'u') return false;
                for (++cur; cur < end; cur++) {
                    if (!char_is_hex(*cur)) return false;
                }
                return true;
            }
            return false;
        }
        
        /* 2 to 4 bytes UTF-8, see `read_string()` for details. */
        if (*cur & 0x80) {
            uint8_t c0 = cur[0], c1 = cur[1], c2 = cur[2];
            if (len == 1) {
                /* 2 bytes UTF-8, truncated */
                if ((c0 & 0xE0) == 0xC0 && (c0 & 0x1E) != 0x00) return true;
                /* 3 bytes UTF-8, truncated */
                if ((c0 & 0xF0) == 0xE0) return true;
                /* 4 bytes UTF-8, truncated */
                if ((c0 & 0xF8) == 0xF0 && (c0 & 0x07) <= 0x04) return true;
            }
            if (len == 2) {
                /* 3 bytes UTF-8, truncated */
                if ((c0 & 0xF0) == 0xE0 &&
                    (c1 & 0xC0) == 0x80) {
                    uint8_t pat = (uint8_t)(((c0 & 0x0F) << 1) | ((c1 & 0x20) >> 5));
                    return 0x01 <= pat && pat != 0x1B;
                }
                /* 4 bytes UTF-8, truncated */
                if ((c0 & 0xF8) == 0xF0 &&
                    (c1 & 0xC0) == 0x80) {
                    uint8_t pat = (uint8_t)(((c0 & 0x07) << 2) | ((c1 & 0x30) >> 4));
                    return 0x01 <= pat && pat <= 0x10;
                }
            }
            if (len == 3) {
                /* 4 bytes UTF-8, truncated */
                if ((c0 & 0xF8) == 0xF0 &&
                    (c1 & 0xC0) == 0x80 &&
                    (c2 & 0xC0) == 0x80) {
                    uint8_t pat = (uint8_t)(((c0 & 0x07) << 2) | ((c1 & 0x30) >> 4));
                    return 0x01 <= pat && pat <= 0x10;
                }
            }
        }
    }
    return false;
}

/**
 Read a JSON string.
 @param ptr The head pointer of string before '"' prefix (inout).
 @param lst JSON last position.
 @param inv Allow invalid unicode.
 @param val The string value to be written.
 @param msg The error message pointer.
 @return Whether success.
 */

/** Read 'true' literal, '*cur' should be 't'. */
inline read_json_result read_true(uint8_t* ptr, json_ref* val, 
    jsoncons::read_json_errc& ec) 
{
    uint8_t *cur = ptr;
    if (JSONCONS_LIKELY(utility::byte_match_4(cur, "true"))) {
        std::construct_at(val, true);
        //val.info = uint8_t(json_type::bool_value);
        //val.uni.bool_val = true; 
        return read_json_result{cur + 4, read_json_errc{}};
    }
    ec = read_json_errc::invalid_literal;
    return read_json_result{cur, read_json_errc::invalid_literal};
}

/** Read 'false' literal, '*cur' should be 'f'. */
inline read_json_result read_false(uint8_t *ptr, json_ref* val,
    jsoncons::read_json_errc& ec) 
{
    uint8_t *cur = ptr;
    if (JSONCONS_LIKELY(utility::byte_match_4(cur + 1, "alse"))) {
        std::construct_at(val, false);
        //val.info = uint8_t(json_type::bool_value);
        //val.uni.bool_val = false; 
        return read_json_result{cur + 5, read_json_errc{}};
    }
    ec = read_json_errc::invalid_literal;
    return read_json_result{cur, read_json_errc::invalid_literal};
}

/** Read 'null' literal, '*cur' should be 'n'. */
inline read_json_result read_null(uint8_t *ptr, json_ref* val,
    jsoncons::read_json_errc& ec) 
{
    uint8_t *cur = ptr;
    if (JSONCONS_LIKELY(utility::byte_match_4(cur, "null"))) {
        std::construct_at(val);
        //val.info = uint8_t(json_type::null_value);
        return read_json_result{cur + 4, read_json_errc{}};
    }
    ec = read_json_errc::invalid_literal;
    return read_json_result{cur, read_json_errc::invalid_literal};
}

/** Read 'Inf' or 'Infinity' literal (ignoring case). */
inline read_json_result read_inf(bool sign, uint8_t *ptr, read_json_flags flags, json_ref* val,
    jsoncons::read_json_errc& ec) {
    
    uint8_t *hdr = ptr - sign;
    uint8_t *cur = ptr;
    if ((cur[0] == 'I' || cur[0] == 'i') &&
        (cur[1] == 'N' || cur[1] == 'n') &&
        (cur[2] == 'F' || cur[2] == 'f')) {
        if ((cur[3] == 'I' || cur[3] == 'i') &&
            (cur[4] == 'N' || cur[4] == 'n') &&
            (cur[5] == 'I' || cur[5] == 'i') &&
            (cur[6] == 'T' || cur[6] == 't') &&
            (cur[7] == 'Y' || cur[7] == 'y')) {
            cur += 8;
        } else {
            cur += 3;
        }
        if (JSONCONS_UNLIKELY(((flags & read_json_flags::number_as_raw) != read_json_flags{}))) {
            std::construct_at(val, raw_json_arg, (const char *)hdr, (std::size_t)(cur - hdr)); 
        } else {
            std::construct_at(val, utility::f64_raw_get_inf(sign)); 
            //val.info = uint8_t(json_type::double_value);
            //val.uni.f64_val = utility::f64_raw_get_inf(sign);
        }
        return read_json_result{ cur, read_json_errc{} };
    }
    ec = read_json_errc::no_digit_after_minus_sign;
    return read_json_result{ptr, read_json_errc::no_digit_after_minus_sign}; 
}

/** Read 'NaN' literal (ignoring case). */
inline read_json_result read_nan(bool sign, uint8_t* ptr, read_json_flags flags, json_ref* val,
    jsoncons::read_json_errc& ec) 
{

    uint8_t *hdr = ptr - sign;
    uint8_t *cur = ptr;
    if ((cur[0] == 'N' || cur[0] == 'n') &&
        (cur[1] == 'A' || cur[1] == 'a') &&
        (cur[2] == 'N' || cur[2] == 'n')) {
        cur += 3;
        if (JSONCONS_UNLIKELY(((flags & read_json_flags::number_as_raw) != read_json_flags{}))) 
        {
            std::construct_at(val, raw_json_arg, (const char *)hdr, (std::size_t)(cur - hdr)); 
        } 
        else 
        {
            std::construct_at(val, std::nan("")); 
            //val.info = uint8_t(json_type::double_value);
            //val.uni.f64_val = std::nan("");
        }
        return read_json_result{ cur, read_json_errc{} };
    }
    ec = read_json_errc::no_digit_after_minus_sign;
    return read_json_result{ptr, read_json_errc::no_digit_after_minus_sign}; 
}

/** Read 'Inf', 'Infinity' or 'NaN' literal (ignoring case). */
inline read_json_result read_inf_or_nan(bool sign, uint8_t* ptr, read_json_flags flags, json_ref* val,
    jsoncons::read_json_errc& ec) 
{
    auto result = read_inf(sign, ptr, flags, val, ec);
    return result? result : read_nan(sign, ptr, flags, val, ec);
}

} // namespace jsoncons

#endif // JSONCONS_READ_JSON_HPP

