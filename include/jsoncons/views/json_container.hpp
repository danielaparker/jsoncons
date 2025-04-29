// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_VIEWS_JSON_CONTAINER_HPP
#define JSONCONS_VIEWS_JSON_CONTAINER_HPP

#include <jsoncons/views/json_view.hpp>
#include <jsoncons/views/read_json.hpp>
#include <jsoncons/views/floating_point.hpp>
#include <jsoncons/views/fileio.hpp>
#include <jsoncons/views/unicode.hpp>
#include <jsoncons/views/parse_json_result.hpp>
#include <string_view>
#include <iterator>
#include <stdexcept>
#include <istream>

namespace jsoncons {

template <class Allocator>
class string_pool
{    
    using allocator_type = typename std::allocator_traits<Allocator>:: template rebind_alloc<uint8_t>;;

    uint8_t* data_{nullptr};
    std::size_t length_{0};
    allocator_type alloc_;
public:
    string_pool(const allocator_type& alloc = allocator_type{})
        : alloc_(alloc)
    {
    }
    
    string_pool(uint8_t* data, std::size_t length, const allocator_type& alloc)
        : data_(data), length_(length), alloc_(alloc)
    {
    }

    string_pool(const string_pool& other) = delete;
    
    string_pool(string_pool&& other) noexcept
        : data_(other.data_), length_(other.length_), alloc_(other.alloc_)
    {        
        other.data_ = nullptr;
        other.length_ = 0;
    }

    ~string_pool()
    {
        if (data_ != nullptr)
        {
            std::allocator_traits<allocator_type>::deallocate(alloc_, data_, length_);
        }
    }

    string_pool& operator=(const string_pool& other) = delete;
    string_pool& operator=(string_pool&& other) = delete;

    void swap(string_pool& other)
    {
        std::swap(data_, other.data_);
        std::swap(length_, other.length_);
        if (alloc_ != other.alloc_)
        {
            auto temp = alloc_;
            alloc_ = other.alloc_;
            other.alloc_ = temp;
        }
    }

    uint8_t* data() 
    {
        return data_;
    }
    
    std::size_t length() const
    {
        return length_;
    }
};

template <typename Allocator = std::allocator<uint8_t>>
class json_container
{
public:
    using allocator_type = Allocator;
private:
    using char_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<char>;
    using u8_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<uint8_t>;
    using view_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<json_ref>;
    using buffer_type = std::basic_string<char,std::char_traits<char>,char_allocator_type>;
    
    json_ref* root_{nullptr};
    std::size_t root_capacity_{ 0 };
    string_pool<Allocator> str_pool_;
    allocator_type alloc_;
public:
    json_container(json_ref* root, std::size_t root_capacity,
        string_pool<Allocator>&& str_pool, 
        const allocator_type& alloc)
        : root_(root), root_capacity_(root_capacity), str_pool_{std::move(str_pool)}, 
        alloc_{alloc}
    {
    }
    json_container(const json_container& other) = delete;
    json_container(json_container&& other) noexcept
        : root_(other.root_), str_pool_(std::move(other.str_pool_))
    {
        other.root_ = nullptr;
    }

    ~json_container()
    {
        if (root_ != nullptr)
        {
            view_allocator_type view_alloc{ alloc_ };
            std::allocator_traits<view_allocator_type>::deallocate(view_alloc, root_, root_capacity_);
            root_ = nullptr;
        }
    }

    json_container& operator=(const json_container& other) = delete;

    json_container& operator=(json_container&& other) noexcept
    {
        if (this != &other)
        {
            std::swap(root_, other.root_);
            str_pool_.swap(other.str_pool_);;
        }
        return *this;
    }

    json_view root()
    {
        return json_view(root_);
    }

    static parse_json_result<json_container<Allocator>> parse(std::string_view sv, read_json_flags flg = read_json_flags::none)
    {
        allocator_type alloc{};
        flg &= ~read_json_flags::insitu; /* const string cannot be modified */
        return parse(const_cast<char*>(sv.data()), sv.size(), flg, alloc);
    }
    
    static parse_json_result<json_container<Allocator>> parse(std::istream is,
        read_json_flags flags, const allocator_type& alloc);

    static parse_json_result<json_container<Allocator>> parse_file(std::string_view sv, read_json_flags flg = read_json_flags::none)
    {
        allocator_type alloc{};
        flg &= ~read_json_flags::insitu; /* const string cannot be modified */
        return yyjson_read_file(const_cast<char*>(sv.data()), flg, alloc);
    }

private:
    static parse_json_result<json_container<Allocator>> parse(char* dat, size_t len, read_json_flags flg,
        const allocator_type& alloc);

    static parse_json_result<json_container<Allocator>> yyjson_read_opts(char* dat,
        std::size_t len,
        read_json_flags flg,
        const allocator_type& alloc);

    static parse_json_result<json_container<Allocator>> yyjson_read_file(const char* path,
        read_json_flags flg,
        const allocator_type& alloc);

    static parse_json_result<json_container<Allocator>> yyjson_read_fp(FILE* file,
        read_json_flags flg,
        const allocator_type& alloc);
    
    static parse_json_result<json_container<Allocator>> read_root_single(string_pool<Allocator>&& str_pool,
        uint8_t *cur,
        uint8_t *end,
        read_json_flags flags,
        const allocator_type& alloc);
    
    static parse_json_result<json_container<Allocator>> read_root_minify(string_pool<Allocator>&& str_pool,
        uint8_t *cur,
        uint8_t *end,
        read_json_flags flags,
        const allocator_type& alloc);
    
    static parse_json_result<json_container<Allocator>> read_root_pretty(string_pool<Allocator>&& str_pool,
        uint8_t *cur,
        uint8_t *end,
        read_json_flags flags,
        const allocator_type& alloc);
};

/*==============================================================================
 * Compile-time Options
 *============================================================================*/

 /*
  Define as 1 to disable the fast floating-point number conversion in yyjson,
  and use libc's `strtod/snprintf` instead.

  This will reduce the binary size by about 30%, but significantly slow down the
  floating-point read/write speed.
  */

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

JSONCONS_FORCE_INLINE read_json_result skip_spaces_and_comments(uint8_t* hdr) 
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
JSONCONS_FORCE_INLINE read_json_result read_true(uint8_t* ptr, json_ref* val) 
{
    uint8_t *cur = ptr;
    if (JSONCONS_LIKELY(utility::byte_match_4(cur, "true"))) {
        ::new(val)json_ref(true);
        //val.info = uint8_t(json_type::bool_value);
        //val.uni.bool_val = true; 
        return read_json_result{cur + 4, read_json_errc{}};
    }
    return read_json_result{cur, read_json_errc::invalid_literal};
}

/** Read 'false' literal, '*cur' should be 'f'. */
JSONCONS_FORCE_INLINE read_json_result read_false(uint8_t *ptr, json_ref* val) 
{
    uint8_t *cur = ptr;
    if (JSONCONS_LIKELY(utility::byte_match_4(cur + 1, "alse"))) {
        ::new(val) json_ref(false);
        //val.info = uint8_t(json_type::bool_value);
        //val.uni.bool_val = false; 
        return read_json_result{cur + 5, read_json_errc{}};
    }
    return read_json_result{cur, read_json_errc::invalid_literal};
}

/** Read 'null' literal, '*cur' should be 'n'. */
JSONCONS_FORCE_INLINE read_json_result read_null(uint8_t *ptr, json_ref* val) 
{
    uint8_t *cur = ptr;
    if (JSONCONS_LIKELY(utility::byte_match_4(cur, "null"))) {
        std::construct_at(val);
        //val.info = uint8_t(json_type::null_value);
        return read_json_result{cur + 4, read_json_errc{}};
    }
    return read_json_result{cur, read_json_errc::invalid_literal};
}

/** Read 'Inf' or 'Infinity' literal (ignoring case). */
JSONCONS_FORCE_INLINE read_json_result read_inf(bool sign, uint8_t *ptr, read_json_flags flags, json_ref* val) {
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
            ::new(val) json_ref(raw_json_arg, (const char *)hdr, (std::size_t)(cur - hdr)); 
        } else {
            ::new(val)json_ref(utility::f64_raw_get_inf(sign)); 
            //val.info = uint8_t(json_type::double_value);
            //val.uni.f64_val = utility::f64_raw_get_inf(sign);
        }
        return read_json_result{ cur, read_json_errc{} };
    }
    return read_json_result{ptr, read_json_errc::no_digit_after_minus_sign}; 
}

/** Read 'NaN' literal (ignoring case). */
JSONCONS_FORCE_INLINE read_json_result read_nan(bool sign, uint8_t* ptr, read_json_flags flags, json_ref* val) {
    uint8_t *hdr = ptr - sign;
    uint8_t *cur = ptr;
    if ((cur[0] == 'N' || cur[0] == 'n') &&
        (cur[1] == 'A' || cur[1] == 'a') &&
        (cur[2] == 'N' || cur[2] == 'n')) {
        cur += 3;
        if (JSONCONS_UNLIKELY(((flags & read_json_flags::number_as_raw) != read_json_flags{}))) 
        {
            ::new(val) json_ref(raw_json_arg, (const char *)hdr, (std::size_t)(cur - hdr)); 
        } 
        else 
        {
            ::new(val) json_ref(std::nan("")); 
            //val.info = uint8_t(json_type::double_value);
            //val.uni.f64_val = std::nan("");
        }
        return read_json_result{ cur, read_json_errc{} };
    }
    return read_json_result{ptr, read_json_errc::no_digit_after_minus_sign}; 
}

/** Read 'Inf', 'Infinity' or 'NaN' literal (ignoring case). */
JSONCONS_FORCE_INLINE read_json_result read_inf_or_nan(bool sign, uint8_t* ptr, read_json_flags flags, json_ref* val) 
{
    auto result = read_inf(sign, ptr, flags, val);
    return result? result : read_nan(sign, ptr, flags, val);
}
    
/*==============================================================================
 * Warning Suppress
 *============================================================================*/

#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wunused-function"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#   pragma clang diagnostic ignored "-Wunused-label"
#   pragma clang diagnostic ignored "-Wunused-macros"
#   pragma clang diagnostic ignored "-Wunused-variable"
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wunused-function"
#   pragma GCC diagnostic ignored "-Wunused-parameter"
#   pragma GCC diagnostic ignored "-Wunused-label"
#   pragma GCC diagnostic ignored "-Wunused-macros"
#   pragma GCC diagnostic ignored "-Wunused-variable"
#elif defined(_MSC_VER)
#   pragma warning(disable:4100) /* unreferenced formal parameter */
#   pragma warning(disable:4101) /* unreferenced variable */
#   pragma warning(disable:4102) /* unreferenced label */
#   pragma warning(disable:4127) /* conditional expression is constant */
#   pragma warning(disable:4706) /* assignment within conditional expression */
#endif

/*
 Estimated initial ratio of the JSON data (data_size / value_count).
 For example:
    
    data:        {"id":12345678,"name":"Harry"}
    data_size:   30
    value_count: 5
    ratio:       6
    
 yyjson uses dynamic memory with a growth factor of 1.5 when reading and writing
 JSON, the ratios below are used to determine the initial memory size.
 
 A too large ratio will waste memory, and a too small ratio will cause multiple
 memory growths and degrade performance. Currently, these ratios are generated
 with some commonly used JSON datasets.
 */

/* The minimum size of the dynamic allocator's chunk. */

/* Default value for compile-time options. */
#ifndef YYJSON_DISABLE_FAST_FP_CONV
#define YYJSON_DISABLE_FAST_FP_CONV 0
#endif
#ifndef YYJSON_DISABLE_UTF8_VALIDATION
#define YYJSON_DISABLE_UTF8_VALIDATION 0
#endif

/*==============================================================================
 * Macros
 *============================================================================*/

/* Used to write uint64_t literal for C89 which doesn't support "ULL" suffix. */
#undef  U64
#define U64(hi, lo) ((((uint64_t)hi##UL) << 32U) + lo##UL)

/*==============================================================================
 * Bits Utils
 * These functions are used by the floating-point number reader and writer.
 *============================================================================*/

/** Returns the number of leading 0-bits in value (input should not be 0). */
JSONCONS_FORCE_INLINE static uint32_t u64_lz_bits(uint64_t v) {
#if GCC_HAS_CLZLL
    return (uint32_t)__builtin_clzll(v);
#elif MSC_HAS_BIT_SCAN_64
    unsigned long r;
    _BitScanReverse64(&r, v);
    return (uint32_t)63 - (uint32_t)r;
#elif MSC_HAS_BIT_SCAN
    unsigned long hi, lo;
    bool hi_set = _BitScanReverse(&hi, (uint32_t)(v >> 32)) != 0;
    _BitScanReverse(&lo, (uint32_t)v);
    hi |= 32;
    return (uint32_t)63 - (uint32_t)(hi_set ? hi : lo);
#else
    /*
     branchless, use de Bruijn sequences
     see: https://www.chessprogramming.org/BitScan
     */
    const uint8_t table[64] = {
        63, 16, 62,  7, 15, 36, 61,  3,  6, 14, 22, 26, 35, 47, 60,  2,
         9,  5, 28, 11, 13, 21, 42, 19, 25, 31, 34, 40, 46, 52, 59,  1,
        17,  8, 37,  4, 23, 27, 48, 10, 29, 12, 43, 20, 32, 41, 53, 18,
        38, 24, 49, 30, 44, 33, 54, 39, 50, 45, 55, 51, 56, 57, 58,  0
    };
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    return table[(v * U64(0x03F79D71, 0xB4CB0A89)) >> 58];
#endif
}

/** Returns the number of trailing 0-bits in value (input should not be 0). */
JSONCONS_FORCE_INLINE static uint32_t u64_tz_bits(uint64_t v) {
#if GCC_HAS_CTZLL
    return (uint32_t)__builtin_ctzll(v);
#elif MSC_HAS_BIT_SCAN_64
    unsigned long r;
    _BitScanForward64(&r, v);
    return (uint32_t)r;
#elif MSC_HAS_BIT_SCAN
    unsigned long lo, hi;
    bool lo_set = _BitScanForward(&lo, (uint32_t)(v)) != 0;
    _BitScanForward(&hi, (uint32_t)(v >> 32));
    hi += 32;
    return lo_set ? lo : hi;
#else
    /*
     branchless, use de Bruijn sequences
     see: https://www.chessprogramming.org/BitScan
     */
    const uint8_t table[64] = {
         0,  1,  2, 53,  3,  7, 54, 27,  4, 38, 41,  8, 34, 55, 48, 28,
        62,  5, 39, 46, 44, 42, 22,  9, 24, 35, 59, 56, 49, 18, 29, 11,
        63, 52,  6, 26, 37, 40, 33, 47, 61, 45, 43, 21, 23, 58, 17, 10,
        51, 25, 36, 32, 60, 20, 57, 16, 50, 31, 19, 15, 30, 14, 13, 12
    };
    return table[((v & (~v + 1)) * U64(0x022FDD63, 0xCC95386D)) >> 58];
#endif
}



/*==============================================================================
 * 128-bit Integer Utils
 * These functions are used by the floating-point number reader and writer.
 *============================================================================*/

/** Multiplies two 64-bit unsigned integers (a * b),
    returns the 128-bit result as 'hi' and 'lo'. */
JSONCONS_FORCE_INLINE static void u128_mul(uint64_t a, uint64_t b, uint64_t *hi, uint64_t *lo) {
#if YYJSON_HAS_INT128
    utility::u128 m = (utility::u128)a * b;
    *hi = (uint64_t)(m >> 64);
    *lo = (uint64_t)(m);
#elif MSC_HAS_UMUL128
    *lo = _umul128(a, b, hi);
#else
    uint32_t a0 = (uint32_t)(a), a1 = (uint32_t)(a >> 32);
    uint32_t b0 = (uint32_t)(b), b1 = (uint32_t)(b >> 32);
    uint64_t p00 = (uint64_t)a0 * b0, p01 = (uint64_t)a0 * b1;
    uint64_t p10 = (uint64_t)a1 * b0, p11 = (uint64_t)a1 * b1;
    uint64_t m0 = p01 + (p00 >> 32);
    uint32_t m00 = (uint32_t)(m0), m01 = (uint32_t)(m0 >> 32);
    uint64_t m1 = p10 + m00;
    uint32_t m10 = (uint32_t)(m1), m11 = (uint32_t)(m1 >> 32);
    *hi = p11 + m01 + m11;
    *lo = ((uint64_t)m10 << 32) | (uint32_t)p00;
#endif
}

/** Multiplies two 64-bit unsigned integers and add a value (a * b + c),
    returns the 128-bit result as 'hi' and 'lo'. */
JSONCONS_FORCE_INLINE static void u128_mul_add(uint64_t a, uint64_t b, uint64_t c, uint64_t *hi, uint64_t *lo) {
#if YYJSON_HAS_INT128
    utility::u128 m = (utility::u128)a * b + c;
    *hi = (uint64_t)(m >> 64);
    *lo = (uint64_t)(m);
#else
    uint64_t h, l, t;
    u128_mul(a, b, &h, &l);
    t = l + c;
    h += (uint64_t)(((t < l) | (t < c)));
    *hi = h;
    *lo = t;
#endif
}

/*==============================================================================
 * Power10 Lookup Table
 * These data are used by the floating-point number reader and writer.
 *============================================================================*/


/*==============================================================================
 * Digit Character Matcher
 *============================================================================*/

/** Digit type */
typedef uint8_t digi_type;

/** Digit: '0'. */
static constexpr digi_type DIGI_TYPE_ZERO       = 1 << 0;

/** Digit: [1-9]. */
static constexpr digi_type DIGI_TYPE_NONZERO    = 1 << 1;

/** Plus sign (positive): '+'. */
static constexpr digi_type DIGI_TYPE_POS        = 1 << 2;

/** Minus sign (negative): '-'. */
static constexpr digi_type DIGI_TYPE_NEG        = 1 << 3;

/** Decimal point: '.' */
static constexpr digi_type DIGI_TYPE_DOT        = 1 << 4;

/** Exponent sign: 'e, 'E'. */
static constexpr digi_type DIGI_TYPE_EXP        = 1 << 5;

/** Digit type table (generate with misc/make_tables.c) */
static constexpr digi_type digi_table[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x08, 0x10, 0x00,
    0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/** Match a character with specified type. */
JSONCONS_FORCE_INLINE static bool digi_is_type(uint8_t d, digi_type type) {
    return (digi_table[d] & type) != 0;
}

/** Match a sign: '+', '-' */
JSONCONS_FORCE_INLINE static bool digi_is_sign(uint8_t d) {
    return digi_is_type(d, (digi_type)(DIGI_TYPE_POS | DIGI_TYPE_NEG));
}

/** Match a none zero digit: [1-9] */
JSONCONS_FORCE_INLINE static bool digi_is_nonzero(uint8_t d) {
    return digi_is_type(d, (digi_type)DIGI_TYPE_NONZERO);
}

/** Match a digit: [0-9] */
JSONCONS_FORCE_INLINE static bool digi_is_digit(uint8_t d) {
    return digi_is_type(d, (digi_type)(DIGI_TYPE_ZERO | DIGI_TYPE_NONZERO));
}

/** Match an exponent sign: 'e', 'E'. */
JSONCONS_FORCE_INLINE static bool digi_is_exp(uint8_t d) {
    return digi_is_type(d, (digi_type)DIGI_TYPE_EXP);
}

/** Match a floating point indicator: '.', 'e', 'E'. */
JSONCONS_FORCE_INLINE static bool digi_is_fp(uint8_t d) {
    return digi_is_type(d, (digi_type)(DIGI_TYPE_DOT | DIGI_TYPE_EXP));
}

/** Match a digit or floating point indicator: [0-9], '.', 'e', 'E'. */
JSONCONS_FORCE_INLINE static bool digi_is_digit_or_fp(uint8_t d) {
    return digi_is_type(d, (digi_type)(DIGI_TYPE_ZERO | DIGI_TYPE_NONZERO |
                                       DIGI_TYPE_DOT | DIGI_TYPE_EXP));
}

/*==============================================================================
 * Hex Character Reader
 * This function is used by JSON reader to read escaped characters.
 *============================================================================*/

/**
 This table is used to convert 4 hex character sequence to a number.
 A valid hex character [0-9A-Fa-f] will mapped to it's raw number [0x00, 0x0F],
 an invalid hex character will mapped to [0xF0].
 (generate with misc/make_tables.c)
 */
static constexpr uint8_t hex_conv_table[256] = {
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0
};

/**
 Scans an escaped character sequence as a UTF-16 code unit (branchless).
 e.g. "\\u005C" should pass "005C" as `cur`.
 
 This requires the string has 4-byte zero padding.
 */
JSONCONS_FORCE_INLINE static bool read_hex_u16(const uint8_t *cur, uint16_t *val) {
    uint16_t c0, c1, c2, c3, t0, t1;
    c0 = hex_conv_table[cur[0]];
    c1 = hex_conv_table[cur[1]];
    c2 = hex_conv_table[cur[2]];
    c3 = hex_conv_table[cur[3]];
    t0 = (uint16_t)((c0 << 8) | c2);
    t1 = (uint16_t)((c1 << 8) | c3);
    *val = (uint16_t)((t0 << 4) | t1);
    return ((t0 | t1) & (uint16_t)0xF0F0) == 0;
}



/*==============================================================================
 * JSON Reader Utils
 * These functions are used by JSON reader to read literals and comments.
 *============================================================================*/

/** Read a JSON number as raw string. */
JSONCONS_FORCE_INLINE static read_json_result read_number_raw(uint8_t* cur,
    read_json_flags flags,
    json_ref* val) 
{
    
    uint8_t* hdr = cur;
    
    /* skip sign */
    cur += (*cur == '-');
    
    /* read first digit, check leading zero */
    if (JSONCONS_UNLIKELY(!digi_is_digit(*cur))) {
        if ((flags & read_json_flags::allow_inf_and_nan) != read_json_flags{}) {
            auto result = read_inf_or_nan(*hdr == '-', cur, flags, val);
            cur = result.ptr;
            if (result)
            {
                ::new(val) json_ref(raw_json_arg, (const char *)hdr, (std::size_t)(cur - hdr));
                return read_json_result{cur, read_json_errc{}}; 
            }
        }
        return read_json_result{cur, read_json_errc::no_digit_after_minus_sign};
    }
    
    /* read integral part */
    if (*cur == '0') {
        cur++;
        if (JSONCONS_UNLIKELY(digi_is_digit(*cur))) {
            return read_json_result(cur - 1, read_json_errc::leading_zero);
        }
        if (!digi_is_fp(*cur)) 
        {
            ::new(val) json_ref(raw_json_arg, (const char *)hdr, (std::size_t)(cur - hdr), semantic_tag::bigint); 
            return read_json_result{cur, read_json_errc{}}; 
        }
    } else {
        while (digi_is_digit(*cur)) cur++;
        if (!digi_is_fp(*cur)) 
        {
            ::new(val) json_ref(raw_json_arg, (const char *)hdr, (std::size_t)(cur - hdr), semantic_tag::bigint); 
            return read_json_result{cur, read_json_errc{}}; 
        }
    }
    
    /* read fraction part */
    if (*cur == '.') {
        cur++;
        if (!digi_is_digit(*cur++)) {
            return read_json_result(cur, read_json_errc::no_digit_after_decimal_point);
        }
        while (digi_is_digit(*cur)) cur++;
    }
    
    /* read exponent part */
    if (digi_is_exp(*cur)) {
        cur += 1 + digi_is_sign(cur[1]);
        if (!digi_is_digit(*cur++)) {
            return read_json_result(cur, read_json_errc::no_digit_after_exponent_sign);
        }
        while (digi_is_digit(*cur)) cur++;
    }
    
    ::new(val) json_ref(raw_json_arg, (const char *)hdr, (std::size_t)(cur - hdr)); 
    return read_json_result{cur, read_json_errc{}}; 
}

/**
 Skips spaces and comments as many as possible.
 
 It will return false in these cases:
    1. No character is skipped. The 'end' pointer is set as input cursor.
    2. A multiline comment is not closed. The 'end' pointer is set as the head
       of this comment block.
 */

/**
 Scans an escaped character sequence as a UTF-16 code unit (branchless).
 e.g. "\\u005C" should pass "005C" as `cur`.
 
 This requires the string has 4-byte zero padding.
 */

/**
 Read a JSON number.
 This is a fallback function if the custom number reader is disabled.
 This function use libc's strtod() to read floating-point number.
 */
read_json_result read_number(uint8_t* ptr,
    read_json_flags flags,
    json_ref* val) 
{   
    uint64_t sig, num;
    uint8_t *hdr = ptr;
    uint8_t *cur = ptr;
    uint8_t *dot = nullptr;
    bool sign;
    
    /* read number as raw string if has `read_json_flags::number_as_raw` flag */
    if (JSONCONS_UNLIKELY(((flags & read_json_flags::number_as_raw) != read_json_flags{}))) {
        auto result = read_number_raw(cur, flags, val);
        return result;
    }
    
    sign = (*hdr == '-');
    cur += sign;
    sig = (uint8_t)(*cur - '0');
    
    /* read first digit, check leading zero */
    if (JSONCONS_UNLIKELY(!digi_is_digit(*cur))) {
        if ((flags & read_json_flags::allow_inf_and_nan) != read_json_flags{}) {
            auto result = read_inf_or_nan(sign, cur, flags, val);
            if (result)
            {
                cur = result.ptr;
                return result;
            }
        }
        return read_json_result(cur, read_json_errc::no_digit_after_minus_sign);
    }
    if (*cur == '0') {
        cur++;
        if (JSONCONS_UNLIKELY(digi_is_digit(*cur))) {
            return read_json_result(cur - 1, read_json_errc::leading_zero);
        }
        if (!digi_is_fp(*cur))
        {
            ::new(val) json_ref(zero_arg); 
            return read_json_result{cur, read_json_errc{}}; 
        }
        goto read_double;
    }
    
    /* read continuous digits, up to 19 characters */
#define expr_intg(i) \
    if (JSONCONS_LIKELY((num = (uint64_t)(cur[i] - (uint8_t)'0')) <= 9)) sig = num + sig * 10; \
    else { cur += i; goto intg_end; }
    repeat_in_1_18(expr_intg)
#undef expr_intg
    
    /* here are 19 continuous digits, skip them */
    cur += 19;
    if (digi_is_digit(cur[0]) && !digi_is_digit_or_fp(cur[1])) {
        /* this number is an integer consisting of 20 digits */
        num = (uint8_t)(*cur - '0');
        if ((sig < (U64_MAX / 10)) ||
            (sig == (U64_MAX / 10) && num <= (U64_MAX % 10))) {
            sig = num + sig * 10;
            cur++;
            if (sign) 
            {
                if (((flags & read_json_flags::bignum_as_raw) != read_json_flags{}))
                {
                    ::new(val) json_ref(raw_json_arg, (const char *)hdr, std::size_t(cur - hdr)); 
                    return read_json_result{cur, read_json_errc{}}; 
                }
                ::new(val) json_ref(-static_cast<int64_t>(sig)); 
                return read_json_result{cur, read_json_errc{}}; 
            }
            ::new(val) json_ref(sig);
            return read_json_result{cur, read_json_errc{}}; 
        }
    }
    
intg_end:
    /* continuous digits ended */
    if (!digi_is_digit_or_fp(*cur)) {
        // this number is an integer consisting of 1 to 19 digits 
        if (sign && (sig > ((uint64_t)1 << 63))) {
            if (((flags & read_json_flags::bignum_as_raw) != read_json_flags{}))
            {
                ::new(val) json_ref(raw_json_arg, (const char *)hdr, std::size_t(cur - hdr)); 
                return read_json_result{cur, read_json_errc{}}; 
            }
            ::new(val) json_ref(-static_cast<double>(sig));
            return read_json_result{cur, read_json_errc{}}; \
        }
        if (!sign) 
        { 
            ::new(val) json_ref(sig); 
        } 
        else 
        { 
            ::new(val) json_ref(-std::bit_cast<int64_t,uint64_t>(sig)); 
        } 
        return read_json_result{cur, read_json_errc{}}; \
    }
    
read_double:
    /* this number should be read as double */
    while (digi_is_digit(*cur)) cur++;
    if (!digi_is_fp(*cur) && ((flags & read_json_flags::bignum_as_raw) != read_json_flags{})) {
        ::new(val) json_ref(raw_json_arg, (const char *)hdr, std::size_t(cur - hdr)); // it's a large integer
        return read_json_result{cur, read_json_errc{}}; 
    }
    if (*cur == '.') {
        /* skip fraction part */
        dot = cur;
        cur++;
        if (!digi_is_digit(*cur)) {
            return read_json_result(cur, read_json_errc::no_digit_after_decimal_point);
        }
        cur++;
        while (digi_is_digit(*cur)) cur++;
    }
    if (digi_is_exp(*cur)) {
        /* skip exponent part */
        cur += 1 + digi_is_sign(cur[1]);
        if (!digi_is_digit(*cur)) {
            return read_json_result(cur, read_json_errc::no_digit_after_exponent_sign);
        }
        cur++;
        while (digi_is_digit(*cur)) cur++;
    }
    
    double value;
    auto [tmpp, ec] = std::from_chars((const char *)hdr, (const char *)cur, value);
    if (JSONCONS_UNLIKELY(ec != std::errc{})) 
    {
        if (ec == std::errc::result_out_of_range)
        {
            if (((flags & read_json_flags::bignum_as_raw) != read_json_flags{})) 
            { 
                ::new(val) json_ref(raw_json_arg, (const char *)hdr, std::size_t(cur - hdr)); 
                return read_json_result{cur, read_json_errc{}}; 
            } 
            if ((flags & read_json_flags::allow_inf_and_nan) != read_json_flags{}) 
            { 
                ::new(val) json_ref(std::bit_cast<double,uint64_t>(((uint64_t)sign << 63) | (uint64_t)(F64_RAW_INF))); 
                return read_json_result{cur, read_json_errc{}}; 
            } 
            else 
            {
                return read_json_result(hdr, read_json_errc::inf_or_nan);
            }
        }
        return read_json_result(cur, read_json_errc::invalid_number);
    }
    ::new(val) json_ref(value);
    return read_json_result{cur, read_json_errc{}}; 
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

/**
 Read a JSON string.
 @param ptr The head pointer of string before '"' prefix (inout).
 @param lst JSON last position.
 @param inv Allow invalid unicode.
 @param val The string value to be written.
 @param msg The error message pointer.
 @return Whether success.
 */
JSONCONS_FORCE_INLINE read_json_result read_string(uint8_t* ptr,
    uint8_t *lst,
    bool inv,
    json_ref* val) 
{   
    uint8_t *cur = ptr;
    uint8_t *src = ++cur, *dst, *pos;
    uint16_t hi, lo;
    uint32_t uni, tmp;
    
skip_ascii:
    /* Most strings have no escaped characters, so we can jump them quickly. */
    
skip_ascii_begin:
    /*
     We want to make loop unrolling, as shown in the following code. Some
     compiler may not generate instructions as expected, so we rewrite it with
     explicit goto statements. We hope the compiler can generate instructions
     like this: https://godbolt.org/z/8vjsYq
     
         while (true) repeat16({
            if (JSONCONS_LIKELY(!(char_is_ascii_stop(*src)))) src++;
            else break;
         })
     */
#define expr_jump(i) \
    if (JSONCONS_LIKELY(!char_is_ascii_stop(src[i]))) {} \
    else goto skip_ascii_stop##i;
    
#define expr_stop(i) \
    skip_ascii_stop##i: \
    src += i; \
    goto skip_ascii_end;
    
    repeat16_incr(expr_jump)
    src += 16;
    goto skip_ascii_begin;
    repeat16_incr(expr_stop)
    
#undef expr_jump
#undef expr_stop
    
skip_ascii_end:
    
    /*
     GCC may store src[i] in a register at each line of expr_jump(i) above.
     These instructions are useless and will degrade performance.
     This inline asm is a hint for gcc: "the memory has been modified,
     do not cache it".
     
     MSVC, Clang, ICC can generate expected instructions without this hint.
     */
#if YYJSON_IS_REAL_GCC
    __asm__ volatile("":"=m"(*src));
#endif
    if (JSONCONS_LIKELY(*src == '"')) {
        ::new(val) json_ref(noesc_arg, (const char *)cur, (std::size_t)(src - cur));
        *src = '\0';
        return read_json_result{src+1, read_json_errc{}};
    }
    
skip_utf8:
    if (*src & 0x80) { /* non-ASCII character */
        /*
         Non-ASCII character appears here, which means that the text is JSONCONS_LIKELY
         to be written in non-English or emoticons. According to some common
         data set statistics, byte sequences of the same length may appear
         consecutively. We process the byte sequences of the same length in each
         loop, which is more friendly to branch prediction.
         */
        pos = src;
#if YYJSON_DISABLE_UTF8_VALIDATION
        while (true) repeat8({
            if (JSONCONS_LIKELY((*src & 0xF0) == 0xE0)) src += 3;
            else break;
        })
        if (*src < 0x80) goto skip_ascii;
        while (true) repeat8({
            if (JSONCONS_LIKELY((*src & 0xE0) == 0xC0)) src += 2;
            else break;
        })
        while (true) repeat8({
            if (JSONCONS_LIKELY((*src & 0xF8) == 0xF0)) src += 4;
            else break;
        })
#else
        uni = utility::byte_load_4(src);
        while (utility::is_valid_seq_3(uni)) {
            src += 3;
            uni = utility::byte_load_4(src);
        }
        if (utility::is_valid_seq_1(uni)) goto skip_ascii;
        while (utility::is_valid_seq_2(uni)) {
            src += 2;
            uni = utility::byte_load_4(src);
        }
        while (utility::is_valid_seq_4(uni)) {
            src += 4;
            uni = utility::byte_load_4(src);
        }
#endif
        if (JSONCONS_UNLIKELY(pos == src)) {
            if (!inv) return read_json_result(src, read_json_errc::invalid_utf8);
            ++src;
        }
        goto skip_ascii;
    }
    
    /* The escape character appears, we need to copy it. */
    dst = src;
copy_escape:
    if (JSONCONS_LIKELY(*src == '\\')) {
        switch (*++src) {
            case '"':  *dst++ = '"';  src++; break;
            case '\\': *dst++ = '\\'; src++; break;
            case '/':  *dst++ = '/';  src++; break;
            case 'b':  *dst++ = '\b'; src++; break;
            case 'f':  *dst++ = '\f'; src++; break;
            case 'n':  *dst++ = '\n'; src++; break;
            case 'r':  *dst++ = '\r'; src++; break;
            case 't':  *dst++ = '\t'; src++; break;
            case 'u':
                if (JSONCONS_UNLIKELY(!read_hex_u16(++src, &hi))) {
                    return read_json_result(src - 2, read_json_errc::invalid_escaped_sequence);
                }
                src += 4;
                if (JSONCONS_LIKELY((hi & 0xF800) != 0xD800)) {
                    /* a BMP character */
                    if (hi >= 0x800) {
                        *dst++ = (uint8_t)(0xE0 | (hi >> 12));
                        *dst++ = (uint8_t)(0x80 | ((hi >> 6) & 0x3F));
                        *dst++ = (uint8_t)(0x80 | (hi & 0x3F));
                    } else if (hi >= 0x80) {
                        *dst++ = (uint8_t)(0xC0 | (hi >> 6));
                        *dst++ = (uint8_t)(0x80 | (hi & 0x3F));
                    } else {
                        *dst++ = (uint8_t)hi;
                    }
                } else {
                    /* a non-BMP character, represented as a surrogate pair */
                    if (JSONCONS_UNLIKELY((hi & 0xFC00) != 0xD800)) {
                        return read_json_result(src - 6, read_json_errc::invalid_high_surrogate);
                    }
                    if (JSONCONS_UNLIKELY(!utility::byte_match_2(src, "\\u"))) {
                        return read_json_result(src, read_json_errc::no_low_surrogate);
                    }
                    if (JSONCONS_UNLIKELY(!read_hex_u16(src + 2, &lo))) {
                        return read_json_result(src, read_json_errc::invalid_escaped_character);
                    }
                    if (JSONCONS_UNLIKELY((lo & 0xFC00) != 0xDC00)) {
                        return read_json_result(src, read_json_errc::invalid_low_surrogate);
                    }
                    uni = ((((uint32_t)hi - 0xD800) << 10) |
                            ((uint32_t)lo - 0xDC00)) + 0x10000;
                    *dst++ = (uint8_t)(0xF0 | (uni >> 18));
                    *dst++ = (uint8_t)(0x80 | ((uni >> 12) & 0x3F));
                    *dst++ = (uint8_t)(0x80 | ((uni >> 6) & 0x3F));
                    *dst++ = (uint8_t)(0x80 | (uni & 0x3F));
                    src += 6;
                }
                break;
            default: return read_json_result(src, read_json_errc::invalid_escaped_character);
        }
    } else if (JSONCONS_LIKELY(*src == '"')) {
        ::new(val) json_ref((const char *)cur, std::size_t(dst - cur));
        *dst = '\0';
        return read_json_result{src+1, read_json_errc{}};
    } else {
        if (!inv) return read_json_result(src, read_json_errc::unexpected_control_character);
        if (src >= lst) return read_json_result(src, read_json_errc::unclosed_string);
        *dst++ = *src++;
    }
    
copy_ascii:
    /*
     Copy continuous ASCII, loop unrolling, same as the following code:
     
         while (true) repeat16({
            if (JSONCONS_UNLIKELY(char_is_ascii_stop(*src))) break;
            *dst++ = *src++;
         })
     */
#if YYJSON_IS_REAL_GCC
#   define expr_jump(i) \
    if (JSONCONS_LIKELY(!(char_is_ascii_stop(src[i])))) {} \
    else { __asm__ volatile("":"=m"(src[i])); goto copy_ascii_stop_##i; }
#else
#   define expr_jump(i) \
    if (JSONCONS_LIKELY(!(char_is_ascii_stop(src[i])))) {} \
    else { goto copy_ascii_stop_##i; }
#endif
    repeat16_incr(expr_jump)
#undef expr_jump
    
    utility::byte_move_16(dst, src);
    src += 16;
    dst += 16;
    goto copy_ascii;
    
    /*
     The memory will be moved forward by at least 1 byte. So the `byte_move`
     can be one byte more than needed to reduce the number of instructions.
     */
copy_ascii_stop_0:
    goto copy_utf8;
copy_ascii_stop_1:
    utility::byte_move_2(dst, src);
    src += 1;
    dst += 1;
    goto copy_utf8;
copy_ascii_stop_2:
    utility::byte_move_2(dst, src);
    src += 2;
    dst += 2;
    goto copy_utf8;
copy_ascii_stop_3:
    utility::byte_move_4(dst, src);
    src += 3;
    dst += 3;
    goto copy_utf8;
copy_ascii_stop_4:
    utility::byte_move_4(dst, src);
    src += 4;
    dst += 4;
    goto copy_utf8;
copy_ascii_stop_5:
    utility::byte_move_4(dst, src);
    utility::byte_move_2(dst + 4, src + 4);
    src += 5;
    dst += 5;
    goto copy_utf8;
copy_ascii_stop_6:
    utility::byte_move_4(dst, src);
    utility::byte_move_2(dst + 4, src + 4);
    src += 6;
    dst += 6;
    goto copy_utf8;
copy_ascii_stop_7:
    utility::byte_move_8(dst, src);
    src += 7;
    dst += 7;
    goto copy_utf8;
copy_ascii_stop_8:
    utility::byte_move_8(dst, src);
    src += 8;
    dst += 8;
    goto copy_utf8;
copy_ascii_stop_9:
    utility::byte_move_8(dst, src);
    utility::byte_move_2(dst + 8, src + 8);
    src += 9;
    dst += 9;
    goto copy_utf8;
copy_ascii_stop_10:
    utility::byte_move_8(dst, src);
    utility::byte_move_2(dst + 8, src + 8);
    src += 10;
    dst += 10;
    goto copy_utf8;
copy_ascii_stop_11:
    utility::byte_move_8(dst, src);
    utility::byte_move_4(dst + 8, src + 8);
    src += 11;
    dst += 11;
    goto copy_utf8;
copy_ascii_stop_12:
    utility::byte_move_8(dst, src);
    utility::byte_move_4(dst + 8, src + 8);
    src += 12;
    dst += 12;
    goto copy_utf8;
copy_ascii_stop_13:
    utility::byte_move_8(dst, src);
    utility::byte_move_4(dst + 8, src + 8);
    utility::byte_move_2(dst + 12, src + 12);
    src += 13;
    dst += 13;
    goto copy_utf8;
copy_ascii_stop_14:
    utility::byte_move_8(dst, src);
    utility::byte_move_4(dst + 8, src + 8);
    utility::byte_move_2(dst + 12, src + 12);
    src += 14;
    dst += 14;
    goto copy_utf8;
copy_ascii_stop_15:
    utility::byte_move_16(dst, src);
    src += 15;
    dst += 15;
    goto copy_utf8;
    
copy_utf8:
    if (*src & 0x80) { /* non-ASCII character */
        pos = src;
        uni = utility::byte_load_4(src);
#if YYJSON_DISABLE_UTF8_VALIDATION
        while (true) repeat4({
            if ((uni & b3_mask) == b3_patt) {
                utility::byte_copy_4(dst, &uni);
                dst += 3;
                src += 3;
                uni = utility::byte_load_4(src);
            } else break;
        })
        if ((uni & b1_mask) == b1_patt) goto copy_ascii;
        while (true) repeat4({
            if ((uni & b2_mask) == b2_patt) {
                utility::byte_copy_2(dst, &uni);
                dst += 2;
                src += 2;
                uni = utility::byte_load_4(src);
            } else break;
        })
        while (true) repeat4({
            if ((uni & b4_mask) == b4_patt) {
                utility::byte_copy_4(dst, &uni);
                dst += 4;
                src += 4;
                uni = utility::byte_load_4(src);
            } else break;
        })
#else
        while (utility::is_valid_seq_3(uni)) {
            utility::byte_copy_4(dst, &uni);
            dst += 3;
            src += 3;
            uni = utility::byte_load_4(src);
        }
        if (utility::is_valid_seq_1(uni)) goto copy_ascii;
        while (utility::is_valid_seq_2(uni)) {
            utility::byte_copy_2(dst, &uni);
            dst += 2;
            src += 2;
            uni = utility::byte_load_4(src);
        }
        while (utility::is_valid_seq_4(uni)) {
            utility::byte_copy_4(dst, &uni);
            dst += 4;
            src += 4;
            uni = utility::byte_load_4(src);
        }
#endif
        if (JSONCONS_UNLIKELY(pos == src)) {
            if (!inv) return read_json_result(src, read_json_errc::invalid_utf8);
            goto copy_ascii_stop_1;
        }
        goto copy_ascii;
    }
    goto copy_escape;
}

/*
 Estimated initial ratio of the JSON data (data_size / value_count).
 For example:
    
    data:        {"id":12345678,"name":"Harry"}
    data_size:   30
    value_count: 5
    ratio:       6
    
 yyjson uses dynamic memory with a growth factor of 1.5 when reading and writing
 JSON, the ratios below are used to determine the initial memory size.
 
 A too large ratio will waste memory, and a too small ratio will cause multiple
 memory growths and degrade performance. Currently, these ratios are generated
 with some commonly used JSON datasets.
 */
static constexpr uint8_t  read_estimated_pretty_ratio = 16;
static constexpr uint8_t  read_estimated_minify_ratio = 6;

/* The minimum size of the dynamic allocator's chunk. */
static constexpr std::size_t YYJSON_ALC_DYN_MIN_SIZE = 0x1000;

/*==============================================================================
 * Size Utils
 * These functions are used for memory allocation.
 *============================================================================*/

/** Returns whether the size is power of 2 (size should not be 0). */
JSONCONS_FORCE_INLINE static bool size_is_pow2(std::size_t size) {
    return (size & (size - 1)) == 0;
}

/** Align size upwards (may overflow). */
JSONCONS_FORCE_INLINE static std::size_t size_align_up(std::size_t size, std::size_t align) {
    if (size_is_pow2(align)) {
        return (size + (align - 1)) & ~(align - 1);
    } else {
        return size + align - (size + align - 1) % align - 1;
    }
}

/** Align size downwards. */
JSONCONS_FORCE_INLINE static std::size_t size_align_down(std::size_t size, std::size_t align) {
    if (size_is_pow2(align)) {
        return size & ~(align - 1);
    } else {
        return size - (size % align);
    }
}

/** Align address upwards (may overflow). */
JSONCONS_FORCE_INLINE static void *mem_align_up(void *mem, std::size_t align) {
    std::size_t size;
    memcpy(&size, &mem, sizeof(std::size_t));
    size = size_align_up(size, align);
    memcpy(&mem, &size, sizeof(std::size_t));
    return mem;
}


/*==============================================================================
 * File Utils
 * These functions are used to read and write JSON files.
 *============================================================================*/

namespace utility {

FILE *fopen_safe(const char *path, const char *mode) {
//#if YYJSON_MSC_VER >= 1400
    FILE *file = nullptr;
    if (fopen_s(&file, path, mode) != 0) return nullptr;
    return file;
//#else
//    return fopen(path, mode);
//#endif
}

FILE *fopen_readonly(const char *path) {
    return fopen_safe(path, "rb" YYJSON_FOPEN_EXT);
}

FILE *fopen_writeonly(const char *path) {
    return fopen_safe(path, "wb" YYJSON_FOPEN_EXT);
}

std::size_t fread_safe(void *buf, std::size_t size, FILE *file) {
#if YYJSON_MSC_VER >= 1400
    return fread_s(buf, size, 1, size, file);
#else
    return fread(buf, 1, size, file);
#endif
}

}

// json_container

/** Read single value JSON document. */
template <typename Allocator>
parse_json_result<json_container<Allocator>> json_container<Allocator>::read_root_single(string_pool<Allocator>&& str_pool,
    uint8_t *cur,
    uint8_t *end,
    read_json_flags flags,
    const allocator_type& alloc) 
{
    
#define return_err(_pos, _code, _msg) do { \
    if (val_hdr) std::allocator_traits<view_allocator_type>::deallocate(view_alloc, val_hdr, alc_len); \
    return is_truncated_end(str_pool.data(), _pos, end, _code, flags) ? parse_json_result<json_container<Allocator>>{read_json_errc::unexpected_end_of_input} : \
        parse_json_result<json_container<Allocator>>{_code}; \
} while (false)
    
    view_allocator_type view_alloc{alloc};
    std::error_code ec{};
    const char *msg; /* error message */
    
    std::size_t alc_len = 1; /* single value */
    
    bool raw = ((flags & read_json_flags::number_as_raw) != read_json_flags{}) || ((flags & read_json_flags::bignum_as_raw) != read_json_flags{});
    bool inv = (flags & read_json_flags::allow_invalid_unicode) != read_json_flags{};
    uint8_t *raw_end = nullptr;
    uint8_t** pre = raw ? &raw_end : nullptr;
    json_ref* val_hdr = std::allocator_traits<view_allocator_type>::allocate(view_alloc, alc_len); 
    json_ref* val = val_hdr;

    if (char_is_number(*cur)) {
        auto result = read_number(cur, flags, val);
        if (result)
        {
            cur = result.ptr;
            goto doc_end;
        }
        return_err(cur, result.ec, "");
    }
    if (*cur == '"') {
        auto result = read_string(cur, end, inv, val);
        cur = result.ptr;
        if (JSONCONS_UNLIKELY(!result))
        {
            return_err(cur, result.ec, "");
        }
        goto doc_end;
    }
    if (*cur == 't') {
        auto result = jsoncons::read_true(cur, val);
        cur = result.ptr;       
        if (JSONCONS_UNLIKELY(!result))
        {
            ec = result.ec;
            return_err(cur, result.ec, "");
        }
        goto doc_end;
    }
    if (*cur == 'f') {
        auto result = jsoncons::read_false(cur, val);
        cur = result.ptr;       
        if (JSONCONS_UNLIKELY(!result))
        {
            ec = result.ec;
            return_err(cur, result.ec, "");
        }
        goto doc_end;
    }
    if (*cur == 'n') {
        auto result = jsoncons::read_null(cur, val);
        cur = result.ptr;
        if (JSONCONS_UNLIKELY(!result))
        {
            if ((flags & read_json_flags::allow_inf_and_nan) != read_json_flags{}) 
            {
                auto result2 = read_nan(false, cur, flags, val);
                cur = result2.ptr;
                if (result2)
                {
                    goto doc_end;
                }
            }
            return_err(cur, result.ec, "");
        }
        goto doc_end;
    }
    if ((flags & read_json_flags::allow_inf_and_nan) != read_json_flags{}) 
    {
        auto result = read_inf_or_nan(false, cur, flags, val);
        cur = result.ptr;
        if (result)
        {
            goto doc_end;
        }
    }
    goto fail_character;
    
doc_end:
    /* check invalid contents after json document */
    if (JSONCONS_UNLIKELY(cur < end) && (flags & read_json_flags::stop_when_done) == read_json_flags{} ) {
        if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
            auto result = jsoncons::skip_spaces_and_comments(cur);
            if (!result)
            {
                return_err(cur, result.ec, "unclosed multiline comment");
            }
            cur = result.ptr;
        } else {
            while (char_is_space(*cur)) cur++;
        }
        if (JSONCONS_UNLIKELY(cur < end)) goto fail_garbage;
    }
    
    //if (pre && *pre) **pre = '\0';
    return json_container{val_hdr, alc_len, std::move(str_pool), alloc};
    
fail_comment:
    return_err(cur, read_json_errc::unclosed_multiline_comment, "unclosed multiline comment");
fail_character:
    return_err(cur, read_json_errc::unexpected_character, "unexpected character");
fail_garbage:
    return_err(cur, read_json_errc::unexpected_content, "unexpected content after document");
    
#undef return_err
}

/** Read JSON document (accept all style, but optimized for minify). */
template <typename Allocator>
parse_json_result<json_container<Allocator>> json_container<Allocator>::read_root_minify(string_pool<Allocator>&& str_pool,
    uint8_t *cur,
    uint8_t *end,
    read_json_flags flags,
    const allocator_type& alloc) {
    
#define return_err(_pos, _code, _msg) do { \
    if (val_hdr) std::allocator_traits<view_allocator_type>::deallocate(view_alloc, val_hdr, alc_len); \
    return is_truncated_end(str_pool.data(), _pos, end, _code, flags) ? parse_json_result<json_container<Allocator>>{read_json_errc::unexpected_end_of_input} : \
        parse_json_result<json_container<Allocator>>{_code}; \
} while (false)
    
#define val_incr() do { \
    val++; \
    if (JSONCONS_UNLIKELY(val >= val_end)) { \
        std::size_t alc_old = alc_len; \
        alc_len += alc_len / 2; \
        if ((sizeof(std::size_t) < 8) && (alc_len >= alc_max)) goto fail_alloc; \
        val_tmp = std::allocator_traits<view_allocator_type>::allocate(view_alloc, alc_len); \
        if (alc_old > 0) memcpy(val_tmp, val_hdr, alc_old*sizeof(json_ref)); \
        if (val_hdr) std::allocator_traits<view_allocator_type>::deallocate(view_alloc, val_hdr, alc_old); \
        if ((!val_tmp)) goto fail_alloc; \
        val = val_tmp + (std::size_t)(val - val_hdr); \
        ctn = val_tmp + (std::size_t)(ctn - val_hdr); \
        val_hdr = val_tmp; \
        val_end = val_tmp + (alc_len - 2); \
    } \
} while (false)

    view_allocator_type view_alloc{alloc};

    std::cout << "read_root_minify\n"; 
    std::size_t dat_len; /* data length in bytes, hint for allocator */
    std::size_t alc_len; /* value count allocated */
    std::size_t alc_max; /* maximum value count for allocator */
    std::size_t ctn_len; /* the number of elements in current container */
    json_ref *val_hdr; /* the head of allocated values */
    json_ref *val_end; /* the end of allocated values */
    json_ref *val_tmp; /* temporary pointer for realloc */
    json_ref *val; /* current JSON value */
    json_ref *ctn; /* current container */
    json_ref *ctn_parent; /* parent of current container */
    const char *msg; /* error message */
    std::error_code ec{};
    
    bool raw; /* read number as raw */
    bool inv; /* allow invalid unicode */
    uint8_t *raw_end; /* raw end for null-terminator */
    uint8_t **pre; /* previous raw end pointer */
    
    dat_len = (flags & read_json_flags::stop_when_done) != read_json_flags{} ? 256 : (std::size_t)(end - cur);
    alc_max = USIZE_MAX / sizeof(json_ref);
    alc_len = (dat_len / read_estimated_minify_ratio) + 4;
    alc_len = (std::min)(alc_len, alc_max);
    
    val_hdr = std::allocator_traits<view_allocator_type>::allocate(view_alloc, alc_len); 
    if (JSONCONS_UNLIKELY(!val_hdr)) goto fail_alloc;
    val_end = val_hdr + (alc_len - 2); /* padding for key-value pair reading */
    val = val_hdr;
    ctn = val;
    ctn_len = 0;
    raw = ((flags & read_json_flags::number_as_raw) != read_json_flags{}) || ((flags & read_json_flags::bignum_as_raw) != read_json_flags{});
    inv = ((flags & read_json_flags::allow_invalid_unicode) != read_json_flags{}) ;
    raw_end = nullptr;
    pre = raw ? &raw_end : nullptr;
    
    if (*cur++ == '{') {
        ctn->info = uint8_t(json_type::object_value);
        ctn->uni.index = 0;
        goto obj_key_begin;
    } else {
        ctn->info = uint8_t(json_type::array_value);
        ctn->uni.index = 0;
        goto arr_val_begin;
    }
    
arr_begin:
    /* save current container */
    ctn->info = (((uint64_t)ctn_len + 1) << tag_bit) |
               (ctn->info & tag_mask);
    
    /* create a new array value, save parent container offset */
    val_incr();
    val->info = uint8_t(json_type::array_value);
    val->uni.index = std::size_t(val - ctn);
    
    /* push the new array value as current container */
    ctn = val;
    ctn_len = 0;
    
arr_val_begin:
    if (*cur == '{') {
        cur++;
        goto obj_begin;
    }
    if (*cur == '[') {
        cur++;
        goto arr_begin;
    }
    if (char_is_number(*cur)) {
        val_incr();
        ctn_len++;
        auto result = jsoncons::read_number(cur, flags, val);
        if (result)
        {
            cur = result.ptr;
            goto arr_val_end;
        }
        return_err(cur, result.ec, "");
    }
    if (*cur == '"') {
        val_incr();
        ctn_len++;
        auto result = jsoncons::read_string(cur, end, inv, val);
        cur = result.ptr;
        if (JSONCONS_UNLIKELY(!result))
        {
            return_err(cur, result.ec, "");
        }
        goto arr_val_end;
    }
    if (*cur == 't') {
        val_incr();
        ctn_len++;
        auto result = jsoncons::read_true(cur, val);
        cur = result.ptr;       
        if (JSONCONS_UNLIKELY(!result))
        {
            ec = result.ec;
            return_err(cur, result.ec, "");
        }
        goto arr_val_end;
    }
    if (*cur == 'f') {
        val_incr();
        ctn_len++;
        auto result = jsoncons::read_false(cur, val);
        cur = result.ptr;       
        if (JSONCONS_UNLIKELY(!result))
        {
            ec = result.ec;
            return_err(cur, result.ec, "");
        }
        goto arr_val_end;
    }
    if (*cur == 'n') {
        val_incr();
        ctn_len++;
        auto result = jsoncons::read_null(cur, val);
        cur = result.ptr;
        if (JSONCONS_UNLIKELY(!result))
        {
            if ((flags & read_json_flags::allow_inf_and_nan) != read_json_flags{}) 
            {
                auto result2 = read_nan(false, cur, flags, val);
                cur = result2.ptr;
                if (result2)
                {
                    goto arr_val_end;
                }
            }
            return_err(cur, result.ec, "");
        }
        goto arr_val_end;
    }
    if (*cur == ']') {
        cur++;
        if (JSONCONS_LIKELY(ctn_len == 0)) goto arr_end;
        if ((flags & read_json_flags::allow_trailing_commas) != read_json_flags{}) goto arr_end;
        while (*cur != ',') cur--;
        goto fail_trailing_comma;
    }
    if (char_is_space(*cur)) {
        while (char_is_space(*++cur));
        goto arr_val_begin;
    }
    if ((flags & read_json_flags::allow_inf_and_nan) != read_json_flags{} &&
        (*cur == 'i' || *cur == 'I' || *cur == 'N')) {
        val_incr();
        ctn_len++;
        auto result = read_inf_or_nan(false, cur, flags, val);
        cur = result.ptr;
        if (result)
        {
            goto arr_val_end;
        }

        goto fail_character;
    }
    if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
        auto result = jsoncons::skip_spaces_and_comments(cur);
        if (!result)
        {
            return_err(cur, result.ec, "unclosed multiline comment");
        }
        cur = result.ptr;
        goto arr_val_begin;
    }
    goto fail_character;
    
arr_val_end:
    if (*cur == ',') {
        cur++;
        goto arr_val_begin;
    }
    if (*cur == ']') {
        cur++;
        goto arr_end;
    }
    if (char_is_space(*cur)) {
        while (char_is_space(*++cur));
        goto arr_val_end;
    }
    if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
        auto result = skip_spaces_and_comments(cur);
        if (!result)
        {
            return_err(cur, result.ec, "unclosed multiline comment");
        }
        cur = result.ptr;
        goto arr_val_end;
    }
    goto fail_character;
    
arr_end:
    /* get parent container */
    ctn_parent = ctn - ctn->uni.index;
    
    /* save the next sibling value offset */
    ctn->uni.index = (std::size_t)(val - ctn) + 1;
    ctn->info = ((ctn_len) << tag_bit) | uint8_t(json_type::array_value);
    if (JSONCONS_UNLIKELY(ctn == ctn_parent)) goto doc_end;
    
    /* pop parent as current container */
    ctn = ctn_parent;
    ctn_len = (std::size_t)(ctn->info >> tag_bit);
    if ((ctn->info & type_mask) == uint8_t(json_type::object_value)) {
        goto obj_val_end;
    } else {
        goto arr_val_end;
    }
    
obj_begin:
    /* push container */
    ctn->info = (((uint64_t)ctn_len + 1) << tag_bit) |
               (ctn->info & tag_mask);
    val_incr();
    val->info = uint8_t(json_type::object_value);
    /* offset to the parent */
    val->uni.index = (std::size_t)(val - ctn);
    ctn = val;
    ctn_len = 0;

obj_key_begin:
    if (JSONCONS_LIKELY(*cur == '"')) {
        val_incr();
        ctn_len++;
        auto result = jsoncons::read_string(cur, end, inv, val);
        cur = result.ptr;
        if (JSONCONS_UNLIKELY(!result))
        {
            return_err(cur, result.ec, "");
        }
        goto obj_key_end;
    }
    if (JSONCONS_LIKELY(*cur == '}')) {
        cur++;
        if (JSONCONS_LIKELY(ctn_len == 0)) goto obj_end;
        if ((flags & read_json_flags::allow_trailing_commas) != read_json_flags{}) goto obj_end;
        while (*cur != ',') cur--;
        goto fail_trailing_comma;
    }
    if (char_is_space(*cur)) {
        while (char_is_space(*++cur));
        goto obj_key_begin;
    }
    if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
        auto result = skip_spaces_and_comments(cur);
        if (!result)
        {
            return_err(cur, result.ec, "unclosed multiline comment");
        }
        cur = result.ptr;
        goto obj_key_begin;
    }
    goto fail_character;
    
obj_key_end:
    if (*cur == ':') {
        cur++;
        goto obj_val_begin;
    }
    if (char_is_space(*cur)) {
        while (char_is_space(*++cur));
        goto obj_key_end;
    }
    if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
        auto result = skip_spaces_and_comments(cur);
        if (!result)
        {
            return_err(cur, result.ec, "unclosed multiline comment");
        }
        cur = result.ptr;
        goto obj_key_end;
    }
    goto fail_character;
    
obj_val_begin:
    if (*cur == '"') {
        val++;
        ctn_len++;
        auto result = jsoncons::read_string(cur, end, inv, val);
        cur = result.ptr;
        if (JSONCONS_UNLIKELY(!result))
        {
            return_err(cur, result.ec, "");
        }
        goto obj_val_end;
    }
    if (char_is_number(*cur)) {
        val++;
        ctn_len++;
        auto result = jsoncons::read_number(cur, flags, val);
        if (result)
        {
            cur = result.ptr;
            goto obj_val_end;           
        }
        return_err(cur, result.ec, "");
    }
    if (*cur == '{') {
        cur++;
        goto obj_begin;
    }
    if (*cur == '[') {
        cur++;
        goto arr_begin;
    }
    if (*cur == 't') {
        val++;
        ctn_len++;
        auto result = jsoncons::read_true(cur, val);
        cur = result.ptr;       
        if (JSONCONS_UNLIKELY(!result))
        {
            ec = result.ec;
            return_err(cur, result.ec, "");
        }
        goto obj_val_end;
    }
    if (*cur == 'f') {
        val++;
        ctn_len++;
        auto result = jsoncons::read_false(cur, val);
        cur = result.ptr;       
        if (JSONCONS_UNLIKELY(!result))
        {
            ec = result.ec;
            return_err(cur, result.ec, "");
        }
        goto obj_val_end;
    }
    if (*cur == 'n') {
        val++;
        ctn_len++;
        auto result = jsoncons::read_null(cur, val);
        cur = result.ptr;
        if (JSONCONS_UNLIKELY(!result))
        {
            if ((flags & read_json_flags::allow_inf_and_nan) != read_json_flags{}) 
            {
                auto result2 = read_nan(false, cur, flags, val);
                cur = result2.ptr;
                if (result2)
                {
                    goto obj_val_end;
                }
            }
            return_err(cur, result.ec, "");
        }
        goto obj_val_end;
    }
    if (char_is_space(*cur)) {
        while (char_is_space(*++cur));
        goto obj_val_begin;
    }
    if ((flags & read_json_flags::allow_inf_and_nan) != read_json_flags{} &&
        (*cur == 'i' || *cur == 'I' || *cur == 'N')) {
        val++;
        ctn_len++;
        auto result = read_inf_or_nan(false, cur, flags, val);
        cur = result.ptr;
        if (result)
        {
            goto obj_val_end;
        }
        goto fail_character;
    }
    if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
        auto result = skip_spaces_and_comments(cur);
        if (!result)
        {
            return_err(cur, result.ec, "unclosed multiline comment");
        }
        cur = result.ptr;
        goto obj_val_begin;
    }
    goto fail_character;
    
obj_val_end:
    if (JSONCONS_LIKELY(*cur == ',')) {
        cur++;
        goto obj_key_begin;
    }
    if (JSONCONS_LIKELY(*cur == '}')) {
        cur++;
        goto obj_end;
    }
    if (char_is_space(*cur)) {
        while (char_is_space(*++cur));
        goto obj_val_end;
    }
    if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
        auto result = skip_spaces_and_comments(cur);
        if (!result)
        {
            return_err(cur, result.ec, "unclosed multiline comment");
        }
        cur = result.ptr;
        goto obj_val_end;
    }
    goto fail_character;
    
obj_end:
    /* pop container */
    ctn_parent = ctn - ctn->uni.index;
    /* point to the next value */
    ctn->uni.index = (std::size_t)(val - ctn) + 1;
    ctn->info = (ctn_len << (tag_bit - 1)) | uint8_t(json_type::object_value);

    if (JSONCONS_UNLIKELY(ctn == ctn_parent)) goto doc_end;
    ctn = ctn_parent;
    ctn_len = (std::size_t)(ctn->info >> tag_bit);
    if ((ctn->info & type_mask) == uint8_t(json_type::object_value)) {
        goto obj_val_end;
    } else {
        goto arr_val_end;
    }
    
doc_end:
    /* check invalid contents after json document */
    if (JSONCONS_UNLIKELY(cur < end) && (flags & read_json_flags::stop_when_done) == read_json_flags{} ) {
        if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
            auto result = skip_spaces_and_comments(cur);
            if (!result)
            {
                return_err(cur, result.ec, "unclosed multiline comment");
            }
            cur = result.ptr;
        } else {
            while (char_is_space(*cur)) cur++;
        }
        if (JSONCONS_UNLIKELY(cur < end)) goto fail_garbage;
    }
    
    //if (pre && *pre) **pre = '\0';

    return json_container{val_hdr, alc_len, std::move(str_pool), alloc};
    
fail_alloc:
    return_err(cur, read_json_errc::memory_allocation, "memory allocation failed");
fail_trailing_comma:
    return_err(cur, read_json_errc::invalid_structure, "trailing comma is not allowed");
fail_comment:
    return_err(cur, read_json_errc::unclosed_multiline_comment, "unclosed multiline comment");
fail_character:
    return_err(cur, read_json_errc::unexpected_character, "unexpected character");
fail_garbage:
    return_err(cur, read_json_errc::unexpected_content, "unexpected content after document");
    
#undef val_incr
#undef return_err
}

/** Read JSON document (accept all style, but optimized for pretty). */
template <typename Allocator>
parse_json_result<json_container<Allocator>> json_container<Allocator>::read_root_pretty(string_pool<Allocator>&& str_pool,
    uint8_t *cur,
    uint8_t *end,
    read_json_flags flags,
    const allocator_type& alloc) 
{
    
#define return_err(_pos, _code, _msg) do { \
    if (val_hdr) std::allocator_traits<view_allocator_type>::deallocate(view_alloc, val_hdr, alc_len); \
    return is_truncated_end(str_pool.data(), _pos, end, _code, flags) ? parse_json_result<json_container<Allocator>>{read_json_errc::unexpected_end_of_input} : \
        parse_json_result<json_container<Allocator>>{_code}; \
} while (false)
    
#define val_incr() { \
    val++; \
    if (JSONCONS_UNLIKELY(val >= val_end)) { \
        std::size_t alc_old = alc_len; \
        alc_len += alc_len / 2; \
        if ((sizeof(std::size_t) < 8) && (alc_len >= alc_max)) goto fail_alloc; \
        val_tmp = std::allocator_traits<view_allocator_type>::allocate(view_alloc, alc_len); \
        if (alc_old > 0) memcpy(val_tmp, val_hdr, alc_old*sizeof(json_ref)); \
        if (val_hdr) std::allocator_traits<view_allocator_type>::deallocate(view_alloc, val_hdr, alc_old); \
        if ((!val_tmp)) goto fail_alloc; \
        val = val_tmp + (std::size_t)(val - val_hdr); \
        ctn = val_tmp + (std::size_t)(ctn - val_hdr); \
        val_hdr = val_tmp; \
        val_end = val_tmp + (alc_len - 2); \
    } \
}
    view_allocator_type view_alloc{alloc};

    std::cout << "read_root_pretty\n"; 
    std::size_t dat_len; /* data length in bytes, hint for allocator */
    std::size_t alc_len; /* value count allocated */
    std::size_t alc_max; /* maximum value count for allocator */
    std::size_t ctn_len; /* the number of elements in current container */
    json_ref *val_hdr; /* the head of allocated values */
    json_ref *val_end; /* the end of allocated values */
    json_ref *val_tmp; /* temporary pointer for realloc */
    json_ref *val; /* current JSON value */
    json_ref *ctn; /* current container */
    json_ref *ctn_parent; /* parent of current container */
    const char *msg; /* error message */
    std::error_code ec{};
    
    bool raw; /* read number as raw */
    bool inv; /* allow invalid unicode */
    uint8_t *raw_end; /* raw end for null-terminator */
    uint8_t **pre; /* previous raw end pointer */
    
    dat_len = (flags & read_json_flags::stop_when_done) != read_json_flags{} ? 256 : (std::size_t)(end - cur);
    alc_max = USIZE_MAX / sizeof(json_ref);
    alc_len = (dat_len / read_estimated_pretty_ratio) + 4;
    alc_len = (std::min)(alc_len, alc_max);
    
    val_hdr = std::allocator_traits<view_allocator_type>::allocate(view_alloc, alc_len); 
    if (JSONCONS_UNLIKELY(!val_hdr)) goto fail_alloc;
    val_end = val_hdr + (alc_len - 2); /* padding for key-value pair reading */
    val = val_hdr;
    ctn = val;
    ctn_len = 0;
    raw = ((flags & read_json_flags::number_as_raw) != read_json_flags{}) || ((flags & read_json_flags::bignum_as_raw) != read_json_flags{});
    inv = ((flags & read_json_flags::allow_invalid_unicode) != read_json_flags{}) ;
    raw_end = nullptr;
    pre = raw ? &raw_end : nullptr;
    
    if (*cur++ == '{') {
        ctn->info = uint8_t(json_type::object_value);
        ctn->uni.index = 0;
        if (*cur == '\n') cur++;
        goto obj_key_begin;
    } else {
        ctn->info = uint8_t(json_type::array_value);
        ctn->uni.index = 0;
        if (*cur == '\n') cur++;
        goto arr_val_begin;
    }
    
arr_begin:
    /* save current container */
    ctn->info = (((uint64_t)ctn_len + 1) << tag_bit) |
               (ctn->info & tag_mask);
    
    /* create a new array value, save parent container offset */
    val_incr();
    val->info = uint8_t(json_type::array_value);
    val->uni.index = (std::size_t)(val - ctn);
    
    /* push the new array value as current container */
    ctn = val;
    ctn_len = 0;
    if (*cur == '\n') cur++;
    
arr_val_begin:
#if YYJSON_IS_REAL_GCC
    while (true) repeat16({
        if (utility::byte_match_2(cur, "  ")) cur += 2;
        else break;
    })
#else
    while (true) repeat16({
        if (JSONCONS_LIKELY(utility::byte_match_2(cur, "  "))) cur += 2;
        else break;
    })
#endif
    
    if (*cur == '{') {
        cur++;
        goto obj_begin;
    }
    if (*cur == '[') {
        cur++;
        goto arr_begin;
    }
    if (char_is_number(*cur)) {
        val_incr();
        ctn_len++;
        auto result = jsoncons::read_number(cur, flags, val);
        if (result)
        {
            cur = result.ptr;
            goto arr_val_end;
        }
        return_err(cur, result.ec, "");
    }
    if (*cur == '"') {
        val_incr();
        ctn_len++;
        auto result = jsoncons::read_string(cur, end, inv, val);
        cur = result.ptr;
        if (JSONCONS_UNLIKELY(!result))
        {
            return_err(cur, result.ec, "");
        }
        goto arr_val_end;
    }
    if (*cur == 't') {
        val_incr();
        ctn_len++;
        auto result = jsoncons::read_true(cur, val);
        cur = result.ptr;       
        if (JSONCONS_UNLIKELY(!result))
        {
            ec = result.ec;
            return_err(cur, result.ec, "");
        }
        goto arr_val_end;
    }
    if (*cur == 'f') {
        val_incr();
        ctn_len++;
        auto result = jsoncons::read_false(cur, val);
        cur = result.ptr;       
        if (JSONCONS_UNLIKELY(!result))
        {
            ec = result.ec;
            return_err(cur, result.ec, "");
        }
        goto arr_val_end;
    }
    if (*cur == 'n') {
        val_incr();
        ctn_len++;
        auto result = jsoncons::read_null(cur, val);
        cur = result.ptr;
        if (JSONCONS_UNLIKELY(!result))
        {
            if ((flags & read_json_flags::allow_inf_and_nan) != read_json_flags{}) 
            {
                auto result2 = read_nan(false, cur, flags, val);
                cur = result2.ptr;
                if (result2)
                {
                    goto arr_val_end;
                }
            }
            return_err(cur, result.ec, "");
        }
        goto arr_val_end;
    }
    if (*cur == ']') {
        cur++;
        if (JSONCONS_LIKELY(ctn_len == 0)) goto arr_end;
        if ((flags & read_json_flags::allow_trailing_commas) != read_json_flags{}) goto arr_end;
        while (*cur != ',') cur--;
        goto fail_trailing_comma;
    }
    if (char_is_space(*cur)) {
        while (char_is_space(*++cur));
        goto arr_val_begin;
    }
    if ((flags & read_json_flags::allow_inf_and_nan) != read_json_flags{} &&
        (*cur == 'i' || *cur == 'I' || *cur == 'N')) {
        val_incr();
        ctn_len++;
        auto result = read_inf_or_nan(false, cur, flags, val);
        cur = result.ptr;
        if (result)
        {
            goto arr_val_end;
        }
        goto fail_character;
    }
    if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
        auto result = skip_spaces_and_comments(cur);
        if (!result)
        {
            return_err(cur, result.ec, "unclosed multiline comment");
        }
        cur = result.ptr;
        goto arr_val_begin;
    }
    goto fail_character;
    
arr_val_end:
    if (utility::byte_match_2(cur, ",\n")) {
        cur += 2;
        goto arr_val_begin;
    }
    if (*cur == ',') {
        cur++;
        goto arr_val_begin;
    }
    if (*cur == ']') {
        cur++;
        goto arr_end;
    }
    if (char_is_space(*cur)) {
        while (char_is_space(*++cur));
        goto arr_val_end;
    }
    if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
        auto result = skip_spaces_and_comments(cur);
        if (!result)
        {
            return_err(cur, result.ec, "unclosed multiline comment");
        }
        cur = result.ptr;
        goto arr_val_end;
    }
    goto fail_character;
    
arr_end:
    /* get parent container */
    ctn_parent = ctn - ctn->uni.index;
    
    /* save the next sibling value offset */
    ctn->uni.index = (std::size_t)(val - ctn) + 1;
    ctn->info = ((ctn_len) << tag_bit) | uint8_t(json_type::array_value);
    if (JSONCONS_UNLIKELY(ctn == ctn_parent)) goto doc_end;
    
    /* pop parent as current container */
    ctn = ctn_parent;
    ctn_len = (std::size_t)(ctn->info >> tag_bit);
    if (*cur == '\n') cur++;
    if ((ctn->info & type_mask) == uint8_t(json_type::object_value)) {
        goto obj_val_end;
    } else {
        goto arr_val_end;
    }
    
obj_begin:
    /* push container */
    ctn->info = (((uint64_t)ctn_len + 1) << tag_bit) |
               (ctn->info & tag_mask);
    val_incr();
    val->info = uint8_t(json_type::object_value);
    /* offset to the parent */
    val->uni.index = (std::size_t)(val - ctn);
    ctn = val;
    ctn_len = 0;
    if (*cur == '\n') cur++;
    
obj_key_begin:
#if YYJSON_IS_REAL_GCC
    while (true) repeat16({
        if (utility::byte_match_2(cur, "  ")) cur += 2;
        else break;
    })
#else
    while (true) repeat16({
        if (JSONCONS_LIKELY(utility::byte_match_2(cur, "  "))) cur += 2;
        else break;
    })
#endif
    if (JSONCONS_LIKELY(*cur == '"')) {
        val_incr();
        ctn_len++;
        auto result = jsoncons::read_string(cur, end, inv, val);
        cur = result.ptr;
        if (JSONCONS_UNLIKELY(!result))
        {
            return_err(cur, result.ec, "");
        }
        goto obj_key_end;
    }
    if (JSONCONS_LIKELY(*cur == '}')) {
        cur++;
        if (JSONCONS_LIKELY(ctn_len == 0)) goto obj_end;
        if ((flags & read_json_flags::allow_trailing_commas) != read_json_flags{}) goto obj_end;
        while (*cur != ',') cur--;
        goto fail_trailing_comma;
    }
    if (char_is_space(*cur)) {
        while (char_is_space(*++cur));
        goto obj_key_begin;
    }
    if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
        auto result = skip_spaces_and_comments(cur);
        if (!result)
        {
            return_err(cur, result.ec, "unclosed multiline comment");
        }
        cur = result.ptr;
        goto obj_key_begin;
    }
    goto fail_character;
    
obj_key_end:
    if (utility::byte_match_2(cur, ": ")) {
        cur += 2;
        goto obj_val_begin;
    }
    if (*cur == ':') {
        cur++;
        goto obj_val_begin;
    }
    if (char_is_space(*cur)) {
        while (char_is_space(*++cur));
        goto obj_key_end;
    }
    if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
        auto result = skip_spaces_and_comments(cur);
        if (!result)
        {
            return_err(cur, result.ec, "unclosed multiline comment");
        }
        cur = result.ptr;
        goto obj_key_end;
    }
    goto fail_character;
    
obj_val_begin:
    if (*cur == '"') {
        val++;
        ctn_len++;
        auto result = jsoncons::read_string(cur, end, inv, val);
        cur = result.ptr;
        if (JSONCONS_UNLIKELY(!result))
        {
            return_err(cur, result.ec, "");
        }
        goto obj_val_end;
    }
    if (char_is_number(*cur)) {
        val++;
        ctn_len++;
        auto result = jsoncons::read_number(cur, flags, val);
        if (result)
        {
            cur = result.ptr;
            goto obj_val_end;
        }
        return_err(cur, result.ec, "");
    }
    if (*cur == '{') {
        cur++;
        goto obj_begin;
    }
    if (*cur == '[') {
        cur++;
        goto arr_begin;
    }
    if (*cur == 't') {
        val++;
        ctn_len++;
        auto result = jsoncons::read_true(cur, val);
        cur = result.ptr;       
        if (JSONCONS_UNLIKELY(!result))
        {
            ec = result.ec;
            return_err(cur, result.ec, "");
        }
        goto obj_val_end;
    }
    if (*cur == 'f') {
        val++;
        ctn_len++;
        auto result = jsoncons::read_false(cur, val);
        cur = result.ptr;       
        if (JSONCONS_UNLIKELY(!result))
        {
            ec = result.ec;
            return_err(cur, result.ec, "");
        }
        goto obj_val_end;
    }
    if (*cur == 'n') {
        val++;
        ctn_len++;
        auto result = jsoncons::read_null(cur, val);
        cur = result.ptr;
        if (JSONCONS_UNLIKELY(!result))
        {
            if ((flags & read_json_flags::allow_inf_and_nan) != read_json_flags{}) 
            {
                auto result2 = read_nan(false, cur, flags, val);
                cur = result2.ptr;
                if (result2)
                {
                    goto obj_val_end;
                }
            }
            return_err(cur, result.ec, "");
        }
        goto obj_val_end;
    }
    if (char_is_space(*cur)) {
        while (char_is_space(*++cur));
        goto obj_val_begin;
    }
    if ((flags & read_json_flags::allow_inf_and_nan) != read_json_flags{} &&
        (*cur == 'i' || *cur == 'I' || *cur == 'N')) {
        val++;
        ctn_len++;
        auto result = read_inf_or_nan(false, cur, flags, val);
        cur = result.ptr;
        if (result)
        {
            goto obj_val_end;
        }
        goto fail_character;
    }
    if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
        auto result = skip_spaces_and_comments(cur);
        if (!result)
        {
            return_err(cur, result.ec, "unclosed multiline comment");
        }
        cur = result.ptr;
        goto obj_val_begin;
    }
    goto fail_character;
    
obj_val_end:
    if (utility::byte_match_2(cur, ",\n")) {
        cur += 2;
        goto obj_key_begin;
    }
    if (JSONCONS_LIKELY(*cur == ',')) {
        cur++;
        goto obj_key_begin;
    }
    if (JSONCONS_LIKELY(*cur == '}')) {
        cur++;
        goto obj_end;
    }
    if (char_is_space(*cur)) {
        while (char_is_space(*++cur));
        goto obj_val_end;
    }
    if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
        auto result = skip_spaces_and_comments(cur);
        if (!result)
        {
            return_err(cur, result.ec, "unclosed multiline comment");
        }
        cur = result.ptr;
        goto obj_val_end;
    }
    goto fail_character;
    
obj_end:
    /* pop container */
    ctn_parent = ctn - ctn->uni.index;
    /* point to the next value */
    ctn->uni.index = (std::size_t)(val - ctn) + 1;
    ctn->info = (ctn_len << (tag_bit - 1)) | uint8_t(json_type::object_value);
    if (JSONCONS_UNLIKELY(ctn == ctn_parent)) goto doc_end;
    ctn = ctn_parent;
    ctn_len = (std::size_t)(ctn->info >> tag_bit);
    if (*cur == '\n') cur++;
    if ((ctn->info & type_mask) == uint8_t(json_type::object_value)) {
        goto obj_val_end;
    } else {
        goto arr_val_end;
    }
    
doc_end:
    /* check invalid contents after json document */
    if (JSONCONS_UNLIKELY(cur < end) && (flags & read_json_flags::stop_when_done) == read_json_flags{} ) {
        if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
            auto result = skip_spaces_and_comments(cur);
            if (!result)
            {
                return_err(cur, result.ec, "unclosed multiline comment");
            }
            cur = result.ptr;
        } else {
            while (char_is_space(*cur)) cur++;
        }
        if (JSONCONS_UNLIKELY(cur < end)) goto fail_garbage;
    }
    
    return json_container{val_hdr, alc_len, std::move(str_pool), alloc};
    
fail_alloc:
    return_err(cur, read_json_errc::memory_allocation, "memory allocation failed");
fail_trailing_comma:
    return_err(cur, read_json_errc::invalid_structure, "trailing comma is not allowed");
fail_comment:
    return_err(cur, read_json_errc::unclosed_multiline_comment, "unclosed multiline comment");
fail_character:
    return_err(cur, read_json_errc::unexpected_character, "unexpected character");
fail_garbage:
    return_err(cur, read_json_errc::unexpected_content, "unexpected content after document");
    
#undef val_incr
#undef return_err
}

template <typename Allocator>
parse_json_result<json_container<Allocator>> json_container<Allocator>::parse(char *dat,
    std::size_t len,
    read_json_flags flags,
    const allocator_type& alloc) 
{
    
#define return_err(_pos, _code, _msg) do { \
    if (!(flags & read_json_flags::insitu) != read_json_flags{} && hdr) std::allocator_traits<u8_allocator_type>::deallocate(u8_alloc, hdr, hdr_capacity); \
    return parse_json_result<json_container<Allocator>>{_code}; \
} while (false)
    
    
    u8_allocator_type u8_alloc(alloc);
    uint8_t *hdr = nullptr, *end, *cur;
    std::size_t hdr_capacity = 0;
    
    if (JSONCONS_UNLIKELY(!dat)) {
        return_err(0, read_json_errc::invalid_parameter, "input data is nullptr");
    }
    if (JSONCONS_UNLIKELY(!len)) {
        return_err(0, read_json_errc::invalid_parameter, "input length is 0");
    }
    
    /* add 4-byte zero padding for input data if necessary */
    if ((flags & read_json_flags::insitu) != read_json_flags{}) {
        hdr = (uint8_t *)dat;
        end = (uint8_t *)dat + len;
        cur = (uint8_t *)dat;
    } else {
        if (JSONCONS_UNLIKELY(len >= USIZE_MAX - buffer_padding_size)) {
            return_err(0, read_json_errc::memory_allocation, "memory allocation failed");
        }
        hdr_capacity = len + buffer_padding_size;
        hdr = std::allocator_traits<allocator_type>::allocate(u8_alloc, hdr_capacity);
        end = hdr + len;
        cur = hdr;
        memcpy(hdr, dat, len);
        memset(end, 0, buffer_padding_size);
    }
    
    /* skip empty contents before json document */
    if (JSONCONS_UNLIKELY(char_is_space_or_comment(*cur))) {
        if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
            auto result = skip_spaces_and_comments(cur);
            if (!result)
            {
                return_err(cur, result.ec, "unclosed multiline comment");
            }
            cur = result.ptr;
        } else {
            if (JSONCONS_LIKELY(char_is_space(*cur))) {
                while (char_is_space(*++cur));
            }
        }
        if (JSONCONS_UNLIKELY(cur >= end)) {
            return_err(0, read_json_errc::empty_content, "input data is empty");
        }
    }
    
    /* read json document */
    if (JSONCONS_LIKELY(char_is_container(*cur))) {
        if (char_is_space(cur[1]) && char_is_space(cur[2])) {
            return read_root_pretty((flags & read_json_flags::insitu) != read_json_flags{} ? string_pool<Allocator>{alloc} : string_pool<Allocator>{hdr, hdr_capacity, alloc}, 
                cur, end, flags, alloc);
        } else {
            return read_root_minify((flags & read_json_flags::insitu) != read_json_flags{} ? string_pool<Allocator>{alloc} : string_pool<Allocator>{hdr, hdr_capacity, alloc}, 
                cur, end, flags, alloc);
        }
    } else {
        return read_root_single((flags & read_json_flags::insitu) != read_json_flags{} ? string_pool<Allocator>{alloc} : string_pool<Allocator>{hdr, hdr_capacity, alloc}, 
            cur, end, flags, alloc);
    }
    
    #if 0
    /* check result */
    if (JSONCONS_UNLIKELY(!doc)) 
        /* RFC 8259: JSON text MUST be encoded using UTF-8 */
        if (err->pos == 0 && err->code != read_json_errc::memory_allocation) {
            if ((hdr[0] == 0xEF && hdr[1] == 0xBB && hdr[2] == 0xBF)) {
                err->msg = "byte order mark (BOM) is not supported";
            } else if (len >= 4 &&
                       ((hdr[0] == 0x00 && hdr[1] == 0x00 &&
                         hdr[2] == 0xFE && hdr[3] == 0xFF) ||
                        (hdr[0] == 0xFF && hdr[1] == 0xFE &&
                         hdr[2] == 0x00 && hdr[3] == 0x00))) {
                err->msg = "UTF-32 encoding is not supported";
            } else if (len >= 2 &&
                       ((hdr[0] == 0xFE && hdr[1] == 0xFF) ||
                        (hdr[0] == 0xFF && hdr[1] == 0xFE))) {
                err->msg = "UTF-16 encoding is not supported";
            }
        }
        if (!(flags & read_json_flags::insitu) != read_json_flags{}) alc.free(alc.ctx, (void *)hdr);
    }
    return doc;
#endif
    
#undef return_err
}

template <typename Allocator>
parse_json_result<json_container<Allocator>> json_container<Allocator>::yyjson_read_opts(char *dat,
    std::size_t len,
    read_json_flags flags,
    const allocator_type& alloc) {
    
#define return_err(_pos, _code, _msg) do { \
    if (hdr) std::allocator_traits<allocator_type>::deallocate(alloc, hdr, hdr_capacity); \
    return parse_json_result<json_container<Allocator>>{_code}; \
} while (false)
    
    view_allocator_type view_alloc{ alloc };
    json_container doc;
    uint8_t *hdr = nullptr, *end, *cur;
    std::size_t hdr_capacity = 0;
    
    if (JSONCONS_UNLIKELY(!dat)) {
        return_err(0, read_json_errc::invalid_parameter, "input data is nullptr");
    }
    if (JSONCONS_UNLIKELY(!len)) {
        return_err(0, read_json_errc::invalid_parameter, "input length is 0");
    }
    
    /* add 4-byte zero padding for input data if necessary */
    if ((flags & read_json_flags::insitu) != read_json_flags{}) {
        hdr = (uint8_t *)dat;
        end = (uint8_t *)dat + len;
        cur = (uint8_t *)dat;
    } else {
        if (JSONCONS_UNLIKELY(len >= USIZE_MAX - buffer_padding_size)) {
            return_err(0, read_json_errc::memory_allocation, "memory allocation failed");
        }
        hdr = std::allocator_traits<allocator_type>::allocate(alloc, hdr_capacity);
        end = hdr + len;
        cur = hdr;
        memcpy(hdr, dat, len);
        memset(end, 0, buffer_padding_size);
    }
    
    /* skip empty contents before json document */
    if (JSONCONS_UNLIKELY(char_is_space_or_comment(*cur))) {
        if ((flags & read_json_flags::allow_comments) != read_json_flags{}) {
            auto result = skip_spaces_and_comments(cur);
            if (!result)
            {
                return_err(cur, result.ec, "unclosed multiline comment");
            }
            cur = result.ptr;
        } else {
            if (JSONCONS_LIKELY(char_is_space(*cur))) {
                while (char_is_space(*++cur));
            }
        }
        if (JSONCONS_UNLIKELY(cur >= end)) {
            return_err(0, read_json_errc::empty_content, "input data is empty");
        }
    }
    
    /* read json document */
    if (JSONCONS_LIKELY(char_is_container(*cur))) {
        if (char_is_space(cur[1]) && char_is_space(cur[2])) {
            return read_root_pretty((flags & read_json_flags::insitu) != read_json_flags{} ? string_pool<Allocator>{alloc} : string_pool<Allocator>{hdr, hdr_capacity, alloc}, 
                cur, end, flags, alloc);
        } else {
            return read_root_minify((flags & read_json_flags::insitu) != read_json_flags{} ? string_pool<Allocator>{alloc} : string_pool<Allocator>{hdr, hdr_capacity, alloc}, 
                cur, end, flags, alloc);
        }
    } else {
        return read_root_single((flags & read_json_flags::insitu) != read_json_flags{} ? string_pool<Allocator>{alloc} : string_pool<Allocator>{hdr, hdr_capacity, alloc}, 
            cur, end, flags, alloc);
    }
#if 0    
    /* check result */
    if (JSONCONS_UNLIKELY(&doc)) {
        /* RFC 8259: JSON text MUST be encoded using UTF-8 */
        if (err->pos == 0 && err->code != read_json_errc::memory_allocation) {
            if ((hdr[0] == 0xEF && hdr[1] == 0xBB && hdr[2] == 0xBF)) {
                err->msg = "byte order mark (BOM) is not supported";
            } else if (len >= 4 &&
                       ((hdr[0] == 0x00 && hdr[1] == 0x00 &&
                         hdr[2] == 0xFE && hdr[3] == 0xFF) ||
                        (hdr[0] == 0xFF && hdr[1] == 0xFE &&
                         hdr[2] == 0x00 && hdr[3] == 0x00))) {
                err->msg = "UTF-32 encoding is not supported";
            } else if (len >= 2 &&
                       ((hdr[0] == 0xFE && hdr[1] == 0xFF) ||
                        (hdr[0] == 0xFF && hdr[1] == 0xFE))) {
                err->msg = "UTF-16 encoding is not supported";
            }
        }
        if (!(flags & read_json_flags::insitu) != read_json_flags{}) alc.free(alc.ctx, (void *)hdr);
    }
    return doc;
#endif    
#undef return_err
}

template <typename Allocator>
parse_json_result<json_container<Allocator>> json_container<Allocator>::yyjson_read_file(const char *path,
    read_json_flags flags,
    const allocator_type& alloc) {
#define return_err(_code, _msg) do { \
    return parse_json_result<json_container<Allocator>>{_code}; \
} while (false)
    
    view_allocator_type view_alloc{ alloc };
    FILE *file;
    
    if (JSONCONS_UNLIKELY(!path)) return_err(read_json_errc::invalid_parameter, "input path is nullptr");
    
    file = utility::fopen_readonly(path);
    if (JSONCONS_UNLIKELY(!file)) return_err(read_json_errc::file_open, "file opening failed");
    
    auto doc = yyjson_read_fp(file, flags, alloc, view_alloc);
    fclose(file);
    return doc;
    
#undef return_err
}

template <typename Allocator>
parse_json_result<json_container<Allocator>> json_container<Allocator>::yyjson_read_fp(FILE *file,
    read_json_flags flags,
    const allocator_type& alloc) 
{
#define return_err(_code, _msg) do { \
    if (buf) std::allocator_traits<allocator_type>::deallocate(alloc, buf, buf_size); \
    return parse_json_result<json_container<Allocator>>{_code}; \
} while (false)
    
    view_allocator_type view_alloc{ alloc };
    long file_size = 0, file_pos;
    uint8_t *buf = nullptr;
    std::size_t buf_size = 0;
    
    /* validate input parameters */
    if (JSONCONS_UNLIKELY(!file)) return_err(read_json_errc::invalid_parameter, "input file is nullptr");
    
    /* get current position */
    file_pos = ftell(file);
    if (file_pos != -1) {
        /* get total file size, may fail */
        if (fseek(file, 0, SEEK_END) == 0) file_size = ftell(file);
        /* reset to original position, may fail */
        if (fseek(file, file_pos, SEEK_SET) != 0) file_size = 0;
        /* get file size from current postion to end */
        if (file_size > 0) file_size -= file_pos;
    }
    
    /* read file */
    if (file_size > 0) {
        /* read the entire file in one call */
        buf_size = (std::size_t)file_size + buffer_padding_size;
        buf = std::allocator_traits<allocator_type>::allocate(alloc, buf_size); 
        if (utility::fread_safe(buf, (std::size_t)file_size, file) != (std::size_t)file_size) {
            return_err(read_json_errc::file_read, "file reading failed");
        }
    } else {
        /* failed to get file size, read it as a stream */
        std::size_t chunk_min = (std::size_t)64;
        std::size_t chunk_max = (std::size_t)512 * 1024 * 1024;
        std::size_t chunk_now = chunk_min;
        std::size_t read_size;
        uint8_t *tmp;
        
        buf_size = buffer_padding_size;
        while (true) {
            if (buf_size + chunk_now < buf_size) { /* overflow */
                return_err(read_json_errc::memory_allocation, "fail to alloc memory");
            }
            buf_size += chunk_now;
            if (!buf) {
                buf = std::allocator_traits<allocator_type>::allocate(alloc, buf_size); 
            } else {
                tmp = std::allocator_traits<allocator_type>::allocate(alloc, buf_size); 
                memcpy(tmp, buf, buf_size - chunk_now);    
                std::allocator_traits<allocator_type>::deallocate(alloc, buf, buf_size - chunk_now); 
                buf = tmp;
            }
            tmp = ((uint8_t *)buf) + buf_size - buffer_padding_size - chunk_now;
            read_size = utility::fread_safe(tmp, chunk_now, file);
            file_size += (long)read_size;
            if (read_size != chunk_now) break;
            
            chunk_now *= 2;
            if (chunk_now > chunk_max) chunk_now = chunk_max;
        }
    }
    
    /* read JSON */
    memset((uint8_t *)buf + file_size, 0, buffer_padding_size);
    flags |= read_json_flags::insitu;
    auto doc = yyjson_read_opts((char *)buf, (std::size_t)file_size, flags, alloc, view_alloc);
    if (doc) {
        return doc;
    } else {
        if (buf) std::allocator_traits<allocator_type>::deallocate(alloc, buf, file_size); \
        return doc;
    }
    
#undef return_err
}

template <typename Allocator>
parse_json_result<json_container<Allocator>> json_container<Allocator>::parse(std::istream is,
    read_json_flags flags, const allocator_type& alloc) 
{
#define return_err(_code, _msg) do { \
    if (buf) std::allocator_traits<allocator_type>::deallocate(alloc, buf, buf_size); \
    return parse_json_result<json_container<Allocator>>{_code}; \
} while (false)
    
    u8_allocator_type u8_alloc{alloc};
    view_allocator_type view_alloc{alloc};

    uint8_t *buf = nullptr;
    std::size_t buf_size = 0;
    
    /* validate input parameters */
    if (JSONCONS_UNLIKELY(!is)) 
    {
        return_err(read_json_errc::invalid_parameter, "input file is nullptr");
    }

    std::istream::pos_type file_size{ 0 };
    // get current position 
    std::istream::pos_type file_pos = is.tellg();
    if (file_pos != std::istream::pos_type(-1))
    {
        // Get total file size, may fail
        is.seekg (0, is.end);
        file_size = is.tellg();
        is.seekg (file_pos, is.end);
        if (file_size > 0) 
        {
            file_size -= file_pos;
        }
    }

    /* read file */
    if (file_size > 0) {
        /* read the entire file in one call */
        buf_size = (std::size_t)file_size + buffer_padding_size;
        buf = std::allocator_traits<u8_allocator_type>::allocate(u8_alloc, buf_size); 
        is.read((char*)buf, static_cast<std::size_t>(file_size));
        if (!is) 
        {
            return_err(read_json_errc::file_read, "file reading failed");
        }
    } 
    else 
    {
        // failed to get file size, read it as a stream 
        /*std::size_t chunk_min = (std::size_t)64;
        std::size_t chunk_max = (std::size_t)512 * 1024 * 1024;
        std::size_t chunk_now = chunk_min;
        std::size_t read_size;
        uint8_t *tmp;
        
        buf_size = buffer_padding_size;
        while (true) {
            if (buf_size + chunk_now < buf_size) { // overflow 
                return_err(read_json_errc::memory_allocation, "fail to alloc memory");
            }
            buf_size += chunk_now;
            if (!buf) {
                buf = std::allocator_traits<allocator_type>::allocate(alloc, buf_size); 
            } else {
                tmp = std::allocator_traits<allocator_type>::allocate(alloc, buf_size); 
                memcpy(tmp, buf, buf_size - chunk_now);    
                std::allocator_traits<allocator_type>::deallocate(alloc, buf, buf_size - chunk_now); 
                buf = tmp;
            }
            tmp = ((uint8_t *)buf) + buf_size - buffer_padding_size - chunk_now;
            read_size = utility::fread_safe(tmp, chunk_now, file);
            file_size += (long)read_size;
            if (read_size != chunk_now) break;
            
            chunk_now *= 2;
            if (chunk_now > chunk_max) chunk_now = chunk_max;
        } 
        */ 
    }
    
    /* read JSON */
    memset(buf + file_size, 0, static_cast<std::size_t>(buffer_padding_size));
    flags |= read_json_flags::insitu;
    auto doc = yyjson_read_opts((char *)buf, (std::size_t)file_size, flags, alloc);
    if (doc) {
        return doc;
    } else {
        if (buf) std::allocator_traits<allocator_type>::deallocate(u8_alloc, buf, file_size); \
        return doc;
    }
    
#undef return_err
}

bool json_view::equal(const json_view& other) const
{
    json_type type = this->type();
    if (type != other.type()) return false;

    switch (type) {
        case json_type::object_value: 
        {
            std::size_t len = size();
            if (len != other.size()) return false;
            if (len > 0) {
                auto it1 = object_range().begin();
                auto end1 = object_range().end();
                auto it2 = other.object_range().begin();
                auto end2 = other.object_range().end();
                while (it1 != end1) 
                {
                    it2 = other.find(it2, it1->key());
                    if (it2 == end2) return false;
                    if (!it2->value().equal(it1->value()))
                    {
                        return false;
                    }
                    ++it1;
                }
            }
            // json_ref allows duplicate keys, so the check may be inaccurate
            return true;
        }

        case json_type::array_value: 
         {
            std::size_t len = size();
            if (len != other.size()) return false;
            if (len > 0) 
            {
                auto it1 = array_range().begin();
                auto end1 = array_range().end();
                auto it2 = other.array_range().begin();
                while (it1 != end1)
                {
                    if (!it1->equal(*it2))
                    {
                        return false;
                    }
                    ++it1;
                    ++it2;
                }
            }
            return true;
        }

        case json_type::double_value:
        case json_type::uint64_value:
        case json_type::int64_value:
            return this->equal_num(other);

        case json_type::string_value:
        {
            return other.equal_string(this->get_string_view());
        }

        case json_type::null_value:
            return this->element_->info == other.element_->info;
        case json_type::bool_value:
            return this->element_->type() == other.element_->type() && this->element_->get_bool() == other.element_->get_bool();

        default:
            return false;
    }
}

} // jsoncons

#endif // JSONCONS_VIEWS_JSON_CONTAINER_HPP

