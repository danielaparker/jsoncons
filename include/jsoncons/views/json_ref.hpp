/*==============================================================================
 Copyright (c) 2020 YaoYuan <ibireme@gmail.com>
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 *============================================================================*/

/** 
 @file json_ref.hpp
 @date 2019-03-09
 @author YaoYuan
 */

#ifndef JSONCONS2_JSON_ELEMENT_HPP
#define JSONCONS2_JSON_ELEMENT_HPP

#include <jsoncons/views/jsoncons_config.hpp>
#include <jsoncons/views/read_json_error.hpp>
#include <jsoncons/views/more_concepts.hpp>
#include <jsoncons/views/integer.hpp>
#include <jsoncons/views/semantic_tag.hpp>
#include <jsoncons/views/json_type.hpp>
#include <float.h>
#include <iterator>
#include <limits.h>
#include <stddef.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string_view>
#include <charconv>
#include <iostream>

namespace jsoncons2 {

/*==============================================================================
 * Compile Hint Begin
 *============================================================================*/

 /* warning suppress begin */
#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunused-function"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(__GNUC__)
#   if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#   pragma GCC diagnostic push
#   endif
#   pragma GCC diagnostic ignored "-Wunused-function"
#   pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable:4800) /* 'int': forcing value to 'true' or 'false' */
#endif



/*==============================================================================
 * Version
 *============================================================================*/

 /** The major version of yyjson. */
#define YYJSON_VERSION_MAJOR  0

/** The minor version of yyjson. */
#define YYJSON_VERSION_MINOR  9

/** The patch version of yyjson. */
#define YYJSON_VERSION_PATCH  0

/** The version of yyjson in hex: `(major << 16) | (minor << 8) | (patch)`. */
#define YYJSON_VERSION_HEX    0x000900

/** The version string of yyjson. */
#define YYJSON_VERSION_STRING "0.9.0"

/** The version of yyjson in hex, same as `YYJSON_VERSION_HEX`. */
    yyjson_api uint32_t yyjson_version(void);

    /** Subtype of a JSON value (2 bit). */

    /** The mask used to extract the type of a JSON value. */
    inline constexpr uint8_t type_mask = 0x0f;     /* 00001111 */
    /** The number of bits used by the type. */
    inline constexpr uint8_t type_bit = 4;
    /** The mask used to extract the subtype of a JSON value. */
    inline constexpr uint8_t subtype_mask = 0x30;     /* 00110000 */
    /** The number of bits used by the subtype. */
    inline constexpr uint8_t subtype_bit = 2;
    /** The mask used to extract the tag of a JSON value. */
    inline constexpr uint8_t tag_mask = 0xFF;     /* 11111111 */
    /** The number of bits used by the tag. */
    inline constexpr uint8_t tag_bit = 8;

    /** Padding size for JSON reader. */
    inline constexpr uint8_t buffer_padding_size = 4;


    /*==============================================================================
     * JSON Structure
     *============================================================================*/

     /**
      An immutable value for reading JSON.
      A JSON Value has the same lifetime as its document. The memory is held by its
      document and and cannot be freed alone.
      */

    /*==============================================================================
     * JSON Structure (Implementation)
     *============================================================================*/

     /** Payload of a JSON value (8 bytes). */
    union yyjson_val_uni {
        bool        bool_val;
        uint64_t    u64_val;
        int64_t     i64_val;
        double      f64_val;
        const char* str_val;
        size_t      index;
    };

    /**
     Immutable JSON value, 16 bytes.
     */

    inline constexpr noesc_arg_t noesc_arg{};

    class json_view;
    class json_ref
    {
        friend class json_view;
    public:
        uint64_t info; /**< type and length */
        yyjson_val_uni uni; /**< payload */
    public:
        constexpr json_ref() noexcept
            : info(uint8_t(json_type::null))
        {
        }

        constexpr json_ref(array_arg_t) noexcept
            : info(uint8_t(json_type::array))
        {
        }

        constexpr json_ref(object_arg_t) noexcept
            : info(uint8_t(json_type::object))
        {
        }

        template <typename T>
        requires std::signed_integral<T>
        constexpr json_ref(T val) noexcept
            : info(uint8_t(json_type::int64))
        {
            uni.i64_val = val;
        }

        template <typename T>
        requires std::unsigned_integral<T>
        constexpr json_ref(T val) noexcept
            : info(uint8_t(json_type::uint64))
        {
            uni.u64_val = val;
        }

        constexpr json_ref(double val) noexcept
            : info(uint8_t(json_type::float64))
        {
            uni.f64_val = val;
        }

        constexpr json_ref(bool val)
            : info(uint8_t(json_type::boolean))
        {
            uni.bool_val = val;
        }

        constexpr json_ref(noesc_arg_t, const char* str, std::size_t length)
        {
            info = uint64_t(length << tag_bit) | uint64_t(uint8_t(json_type::string) | (uint8_t(semantic_tag::noesc) << type_bit)); 
            uni.str_val = str;
        }

        constexpr json_ref(raw_json_arg_t, const char* str, std::size_t length, semantic_tag subtype = semantic_tag::bignum)
        {
            info = uint64_t(length << tag_bit) | uint64_t(uint8_t(json_type::string) | (uint8_t(subtype) << type_bit)); 
            uni.str_val = str;
        }

        constexpr json_ref(const char* str, std::size_t length)
        {
            info = uint64_t(length << tag_bit) | uint64_t(uint8_t(json_type::string)); 
            uni.str_val = str;
        }

        constexpr json_ref(const json_ref& other) noexcept = default;

        constexpr json_ref& operator=(const json_ref& other) noexcept = default;
        
        template <typename T>
        requires jsoncons2::utility::extended_integral<T>
        constexpr T cast() const
        {
            switch (type())
            {
                case json_type::int64:
                    return static_cast<T>(uni.i64_val);
                case json_type::uint64:
                    return static_cast<T>(uni.u64_val);
                case json_type::float64:
                    return static_cast<T>(uni.f64_val);
                case json_type::string:
                {
                    auto sv = get_string_view();
                    T val;
                    auto result = jsoncons2::utility::to_integer(sv.data(), sv.length(), val);
                    if (!result)
                    {
                        JSONCONS2_THROW(std::system_error(result.error_code()));
                    }
                    return val;
                }
                default:
                    return 0;
            }
        }

        template <typename T>
        requires std::floating_point<T>
        constexpr T cast() const
        {
            switch (type())
            {
                case json_type::int64:
                    return static_cast<T>(uni.i64_val);
                case json_type::uint64:
                    return static_cast<T>(uni.u64_val);
                case json_type::float64:
                    return static_cast<T>(uni.f64_val);
                case json_type::string:
                {
                    T val;
                    auto sv = get_string_view();
                    auto result = std::from_chars(sv.data(), sv.data()+sv.length(), val);
                    if (result.ec != std::errc{})
                    {
                        JSONCONS2_THROW(std::system_error(read_json_errc::not_a_number));
                    }
                    return val;
                }
                default:
                    return 0;
            }
        }

        constexpr json_type type() const noexcept
        {
            return (json_type)((uint8_t)info & type_mask);
        }

        constexpr semantic_tag tag() const noexcept
        {
            return (semantic_tag)(((uint8_t)info & subtype_mask) >> type_bit);
        }

        constexpr std::size_t size() const noexcept
        {
            return (size_t)(info >> tag_bit);
        }
        constexpr bool is_container() const noexcept
        {
            constexpr uint8_t mask{ uint8_t(json_type::array) & uint8_t(json_type::object) };
            return ((info & tag_mask) & mask) == mask;
        }

        constexpr bool is_flat() const noexcept
        {
            return size() + 1 == uni.index;
        }

        constexpr bool get_bool() const
        {
            switch (type())
            {
            case json_type::boolean:
                return uni.bool_val;
            default:
                return false;
            }
        }

        constexpr std::string_view get_string_view() const
        {
            switch (type())
            {
                case json_type::string:
                    return std::string_view(uni.str_val, size());
                default:
                    return std::string_view{};
            }
        }

        constexpr const char* get_cstring() const
        {
            switch (type())
            {
                case json_type::string:
                    return uni.str_val;
                default:
                    return nullptr;
            }
        }

        constexpr double get_double() const
        {
            switch (type())
            {
            case json_type::float64:
                return uni.f64_val;
            default:
                return 0;
            }
        }

        constexpr double get_number() const
        {
            switch (type())
            {
            case json_type::float64:
                return uni.f64_val;
            case json_type::uint64:
                return static_cast<double>(uni.u64_val);
            case json_type::int64:
                return static_cast<double>(uni.i64_val);
            default:
                return 0;
            }
        }
    private:

        constexpr bool equal_string(std::string_view sv) const noexcept
        {
            return equal_string(sv.data(), sv.size());
        }

        constexpr bool equal_string(const char* str, std::size_t len) const noexcept
        {
            if (JSONCONS2_LIKELY(type() == json_type::string))
            {
                return size() == len && memcmp(uni.str_val, str, len) == 0;
            }
            else
            {
                return false;
            }
        }

        constexpr bool equal_num(const json_ref& rhs) const noexcept 
        {
            const yyjson_val_uni* luni = &(uni);
            const yyjson_val_uni* runi = &(rhs.uni);
            json_type lt = type();
            json_type rt = rhs.type();
            if (lt == rt) return luni->u64_val == runi->u64_val;
            if (lt == json_type::int64 && rt == json_type::uint64) {
                return luni->i64_val >= 0 && luni->u64_val == runi->u64_val;
            }
            if (lt == json_type::uint64 && rt == json_type::int64) {
                return runi->i64_val >= 0 && luni->u64_val == runi->u64_val;
            }
            return false;
        }
    };

} // jsoncons2

/*==============================================================================
 * Compiler Hint End
 *============================================================================*/

#if defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(__GNUC__)
#   if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#   pragma GCC diagnostic pop
#   endif
#elif defined(_MSC_VER)
#   pragma warning(pop)
#endif /* warning suppress end */

#endif /* YYJSON_H */
