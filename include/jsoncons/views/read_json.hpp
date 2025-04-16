// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_READ_JSON_HPP
#define JSONCONS_READ_JSON_HPP

#include <jsoncons/views/json_ref.hpp>
#include <string_view>
#include <iterator>
#include <stdexcept>
#include <limits>
#include <cmath>
#include <vector>

namespace jsoncons {

/*==============================================================================
 * JSON Reader API
 *============================================================================*/

 /** Run-time options for JSON reader. */
enum class read_json_flags : uint32_t
{
    none = 0,
    insitu = 1 << 0,
    stop_when_done = 1 << 1,
    allow_trailing_commas = 1 << 2,
    allow_comments = 1 << 3,
    allow_inf_and_nan = 1 << 4,
    number_as_raw = 1 << 5,
    allow_invalid_unicode = 1 << 6,
    bignum_as_raw = 1 << 7
};

JSONCONS_FORCEINLINE read_json_flags operator!(read_json_flags a)
{
    return static_cast<read_json_flags>(!static_cast<uint32_t>(a));
}

JSONCONS_FORCEINLINE read_json_flags operator~(read_json_flags a)
{
    return static_cast<read_json_flags>(~static_cast<uint32_t>(a));
}

JSONCONS_FORCEINLINE read_json_flags operator&(read_json_flags a, read_json_flags b)
{
    return static_cast<read_json_flags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

JSONCONS_FORCEINLINE read_json_flags operator^(read_json_flags a, read_json_flags b)
{
    return static_cast<read_json_flags>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
}

JSONCONS_FORCEINLINE read_json_flags operator|(read_json_flags a, read_json_flags b)
{
    return static_cast<read_json_flags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

JSONCONS_FORCEINLINE read_json_flags operator&=(read_json_flags& a, read_json_flags b)
{
    a = a & b;
    return a;
}

JSONCONS_FORCEINLINE read_json_flags operator^=(read_json_flags& a, read_json_flags b)
{
    a = a ^ b;
    return a;
}

JSONCONS_FORCEINLINE read_json_flags operator|=(read_json_flags& a, read_json_flags b)
{
    a = a | b;
    return a;
}

/** Default option (RFC 8259 compliant):
    - Read positive integer as uint64_t.
    - Read negative integer as int64_t.
    - Read floating-point number as double with round-to-nearest mode.
    - Read integer which cannot fit in uint64_t or int64_t as double.
    - Report error if double number is infinity.
    - Report error if string contains invalid UTF-8 character or BOM.
    - Report error on trailing commas, comments, inf and nan literals. */

    /** Read the input data in-situ.
        This option allows the reader to modify and use input data to store string
        values, which can increase reading speed slightly.
        The caller should hold the input data before free the document.
        The input data must be padded by at least `buffer_padding_size` bytes.
        For example: `[1,2]` should be `[1,2]\0\0\0\0`, input length should be 5. */

        /** Stop when done instead of issuing an error if there's additional content
            after a JSON document. This option may be used to parse small pieces of JSON
            in larger data, such as `NDJSON`. */

            /** Allow single trailing comma at the end of an object or array,
                such as `[1,2,3,]`, `{"a":1,"b":2,}` (non-standard). */

                /** Allow C-style single line and multiple line comments (non-standard). */

                /** Allow inf/nan number and literal, case-insensitive,
                    such as 1e999, NaN, inf, -Infinity (non-standard). */

                    /** Read all numbers as raw strings (value with `uint8_t(json_type::raw)` type),
                        inf/nan literal is also read as raw with `ALLOW_INF_AND_NAN` flag. */

                        /** Allow reading invalid unicode when parsing string values (non-standard).
                            Invalid characters will be allowed to appear in the string values, but
                            invalid escape sequences will still be reported as errors.
                            This flag does not affect the performance of correctly encoded strings.

                            @warning Strings in JSON values may contain incorrect encoding when this
                            option is used, you need to handle these strings carefully to avoid security
                            risks. */

                            /** Read big numbers as raw strings. These big numbers include integers that
                                cannot be represented by `int64_t` and `uint64_t`, and floating-point
                                numbers that cannot be represented by finite `double`.
                                The flag will be overridden by `read_json_flags::number_as_raw` flag. */


                                /** Invalid parameter, such as nullptr input string or 0 input length. */

                                /** Memory allocation failure occurs. */

                                /** Input JSON string is empty. */

                                /** Unexpected content after document, such as `[123]abc`. */

                                /** Unexpected ending, such as `[123`. */

                                /** Unexpected character inside the document, such as `[abc]`. */

                                /** Invalid JSON structure, such as `[1,]`. */

                                /** Invalid comment, such as unclosed multi-line comment. */

                                /** Invalid number, such as `123.e12`, `000`. */

                                /** Invalid string, such as invalid escaped character inside a string. */

                                /** Invalid JSON literal, such as `truu`. */

                                /** Failed to open a file. */

                                /** Failed to read a file. */

                                /** Error information for JSON reader. */

/**
 Read JSON with options.

 This function is thread-safe when:
 1. The `dat` is not modified by other threads.
 2. The `alc` is thread-safe or nullptr.

 @param dat The JSON data (UTF-8 without BOM), null-terminator is not required.
    If this parameter is nullptr, the function will fail and return nullptr.
    The `dat` will not be modified without the flag `read_json_flags::insitu`, so you
    can pass a `const char *` string and case it to `char *` if you don't use
    the `read_json_flags::insitu` flag.
 @param len The length of JSON data in bytes.
    If this parameter is 0, the function will fail and return nullptr.
 @param flg The JSON read options.
    Multiple options can be combined with `|` operator. 0 means no options.
 @param alc The memory allocator used by JSON reader.
    Pass nullptr to use the libc's default allocator.
 @param err A pointer to receive error information.
    Pass nullptr if you don't need error information.
 @return A new JSON document, or nullptr if an error occurs.
    When it's no longer needed, it should be freed with `yyjson_doc_free()`.
 */

struct read_json_result
{
    uint8_t* ptr;
    read_json_errc ec;
    
    constexpr explicit operator bool() const noexcept
    {
        return ec == read_json_errc{};
    }
    
    friend bool operator==(const read_json_result&,
        const read_json_result&) = default;
};

} // namespace jsoncons

#endif // JSONCONS_READ_JSON_HPP

