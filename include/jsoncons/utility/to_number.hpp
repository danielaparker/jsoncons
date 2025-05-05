// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UTILITY_TO_NUMBER_HPP
#define JSONCONS_UTILITY_TO_NUMBER_HPP

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <locale>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits> // std::enable_if
#include <vector>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/utility/more_type_traits.hpp>

namespace jsoncons { 
namespace utility {

template <typename CharT>
struct to_number_result
{
    const CharT* ptr;
    std::errc ec;
    constexpr to_number_result(const CharT* ptr_)
        : ptr(ptr_), ec(std::errc())
    {
    }
    constexpr to_number_result(const CharT* ptr_, std::errc ec_)
        : ptr(ptr_), ec(ec_)
    {
    }

    to_number_result(const to_number_result&) = default;

    to_number_result& operator=(const to_number_result&) = default;

    constexpr explicit operator bool() const noexcept
    {
        return ec == std::errc();
    }
    std::error_code error_code() const
    {
        return make_error_code(ec);
    }
};

enum class integer_chars_format : uint8_t {decimal=1,hex};
enum class integer_chars_state {initial,minus,integer,binary,octal,decimal,base16};

template <typename CharT>
bool is_base10(const CharT* s, std::size_t length)
{
    integer_chars_state state = integer_chars_state::initial;

    const CharT* end = s + length; 
    for (;s < end; ++s)
    {
        switch(state)
        {
            case integer_chars_state::initial:
            {
                switch(*s)
                {
                    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        state = integer_chars_state::decimal;
                        break;
                    case '-':
                        state = integer_chars_state::minus;
                        break;
                    default:
                        return false;
                }
                break;
            }
            case integer_chars_state::minus:
            {
                switch(*s)
                {
                    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        state = integer_chars_state::decimal;
                        break;
                    default:
                        return false;
                }
                break;
            }
            case integer_chars_state::decimal:
            {
                switch(*s)
                {
                    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        break;
                    default:
                        return false;
                }
                break;
            }
            default:
                break;
        }
    }
    return state == integer_chars_state::decimal ? true : false;
}

template <typename T,typename CharT>
bool is_base16(const CharT* s, std::size_t length)
{
    integer_chars_state state = integer_chars_state::initial;

    const CharT* end = s + length; 
    for (;s < end; ++s)
    {
        switch(state)
        {
            case integer_chars_state::initial:
            {
                switch(*s)
                {
                    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9': // Must be base16
                    case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
                    case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
                        state = integer_chars_state::base16;
                        break;
                    default:
                        return false;
                }
                break;
            }
            case integer_chars_state::base16:
            {
                switch(*s)
                {
                    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9': // Must be base16
                    case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
                    case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
                        state = integer_chars_state::base16;
                        break;
                    default:
                        return false;
                }
                break;
            }
            default:
                break;
        }
    }
    return state == integer_chars_state::base16 ? true : false;
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && !ext_traits::integer_limits<T>::is_signed,to_number_result<CharT>>::type
decstr_to_integer(const CharT* s, std::size_t length, T& n)
{
    n = 0;

    integer_chars_state state = integer_chars_state::initial;

    const CharT* end = s + length; 
    while (s < end)
    {
        switch(state)
        {
            case integer_chars_state::initial:
            {
                switch(*s)
                {
                    case '0':
                        if (++s == end)
                        {
                            return (++s == end) ? to_number_result<CharT>(s) : to_number_result<CharT>(s, std::errc());
                        }
                        else
                        {
                            return to_number_result<CharT>(s, std::errc::invalid_argument);
                        }
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9': // Must be decimal
                        state = integer_chars_state::decimal;
                        break;
                    default:
                        return to_number_result<CharT>(s, std::errc::invalid_argument);
                }
                break;
            }
            case integer_chars_state::decimal:
            {
                static constexpr T max_value = (ext_traits::integer_limits<T>::max)();
                static constexpr T max_value_div_10 = max_value / 10;
                for (; s < end; ++s)
                {
                    T x = 0;
                    switch(*s)
                    {
                        case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                            x = static_cast<T>(*s) - static_cast<T>('0');
                            break;
                        default:
                            return to_number_result<CharT>(s, std::errc::invalid_argument);
                    }
                    if (n > max_value_div_10)
                    {
                        return to_number_result<CharT>(s, std::errc::result_out_of_range);
                    }
                    n = n * 10;
                    if (n > max_value - x)
                    {
                        return to_number_result<CharT>(s, std::errc::result_out_of_range);
                    }
                    n += x;
                }
                break;
            }
            default:
                JSONCONS_UNREACHABLE();
                break;
        }
    }
    return (state == integer_chars_state::initial) ? to_number_result<CharT>(s, std::errc::invalid_argument) : to_number_result<CharT>(s, std::errc());
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && ext_traits::integer_limits<T>::is_signed,to_number_result<CharT>>::type
decstr_to_integer(const CharT* s, std::size_t length, T& n)
{
    n = 0;

    if (length == 0)
    {
        return to_number_result<CharT>(s, std::errc::invalid_argument);
    }

    bool is_negative = *s == '-' ? true : false;
    if (is_negative)
    {
        ++s;
        --length;
    }

    using U = typename ext_traits::make_unsigned<T>::type;

    U u;
    auto ru = decstr_to_integer(s, length, u);
    if (ru.ec != std::errc())
    {
        return to_number_result<CharT>(ru.ptr, ru.ec);
    }
    if (is_negative)
    {
        if (u > static_cast<U>(-((ext_traits::integer_limits<T>::lowest)()+T(1))) + U(1))
        {
            return to_number_result<CharT>(ru.ptr, std::errc::result_out_of_range);
        }
        else
        {
            n = static_cast<T>(U(0) - u);
            return to_number_result<CharT>(ru.ptr, std::errc());
        }
    }
    else
    {
        if (u > static_cast<U>((ext_traits::integer_limits<T>::max)()))
        {
            return to_number_result<CharT>(ru.ptr, std::errc::result_out_of_range);
        }
        else
        {
            n = static_cast<T>(u);
            return to_number_result<CharT>(ru.ptr, std::errc());
        }
    }
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && !ext_traits::integer_limits<T>::is_signed,to_number_result<CharT>>::type
to_integer(const CharT* s, std::size_t length, T& n)
{
    n = 0;

    integer_chars_state state = integer_chars_state::initial;

    const CharT* end = s + length; 
    while (s < end)
    {
        switch(state)
        {
            case integer_chars_state::initial:
            {
                switch(*s)
                {
                    case '0':
                        state = integer_chars_state::integer; // Could be binary, octal, hex 
                        ++s;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9': // Must be decimal
                        state = integer_chars_state::decimal;
                        break;
                    default:
                        return to_number_result<CharT>(s, std::errc::invalid_argument);
                }
                break;
            }
            case integer_chars_state::integer:
            {
                switch(*s)
                {
                    case 'b':case 'B':
                    {
                        state = integer_chars_state::binary;
                        ++s;
                        break;
                    }
                    case 'x':case 'X':
                    {
                        state = integer_chars_state::base16;
                        ++s;
                        break;
                    }
                    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                    {
                        state = integer_chars_state::octal;
                        break;
                    }
                    default:
                        return to_number_result<CharT>(s, std::errc::invalid_argument);
                }
                break;
            }
            case integer_chars_state::binary:
            {
                static constexpr T max_value = (ext_traits::integer_limits<T>::max)();
                static constexpr T max_value_div_2 = max_value / 2;
                for (; s < end; ++s)
                {
                    T x = 0;
                    switch(*s)
                    {
                        case '0':case '1':
                            x = static_cast<T>(*s) - static_cast<T>('0');
                            break;
                        default:
                            return to_number_result<CharT>(s, std::errc::invalid_argument);
                    }
                    if (n > max_value_div_2)
                    {
                        return to_number_result<CharT>(s, std::errc::result_out_of_range);
                    }
                    n = n * 2;
                    if (n > max_value - x)
                    {
                        return to_number_result<CharT>(s, std::errc::result_out_of_range);
                    }
                    n += x;
                }
                break;
            }
            case integer_chars_state::octal:
            {
                static constexpr T max_value = (ext_traits::integer_limits<T>::max)();
                static constexpr T max_value_div_8 = max_value / 8;
                for (; s < end; ++s)
                {
                    T x = 0;
                    switch(*s)
                    {
                        case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':
                            x = static_cast<T>(*s) - static_cast<T>('0');
                            break;
                        default:
                            return to_number_result<CharT>(s, std::errc::invalid_argument);
                    }
                    if (n > max_value_div_8)
                    {
                        return to_number_result<CharT>(s, std::errc::result_out_of_range);
                    }
                    n = n * 8;
                    if (n > max_value - x)
                    {
                        return to_number_result<CharT>(s, std::errc::result_out_of_range);
                    }
                    n += x;
                }
                break;
            }
            case integer_chars_state::decimal:
            {
                static constexpr T max_value = (ext_traits::integer_limits<T>::max)();
                static constexpr T max_value_div_10 = max_value / 10;
                for (; s < end; ++s)
                {
                    T x = 0;
                    switch(*s)
                    {
                        case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                            x = static_cast<T>(*s) - static_cast<T>('0');
                            break;
                        default:
                            return to_number_result<CharT>(s, std::errc::invalid_argument);
                    }
                    if (n > max_value_div_10)
                    {
                        return to_number_result<CharT>(s, std::errc::result_out_of_range);
                    }
                    n = n * 10;
                    if (n > max_value - x)
                    {
                        return to_number_result<CharT>(s, std::errc::result_out_of_range);
                    }
                    n += x;
                }
                break;
            }
            case integer_chars_state::base16:
            {
                static constexpr T max_value = (ext_traits::integer_limits<T>::max)();
                static constexpr T max_value_div_16 = max_value / 16;
                for (; s < end; ++s)
                {
                    CharT c = *s;
                    T x = 0;
                    switch (c)
                    {
                        case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                            x = c - '0';
                            break;
                        case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
                            x = c - ('a' - 10);
                            break;
                        case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
                            x = c - ('A' - 10);
                            break;
                        default:
                            return to_number_result<CharT>(s, std::errc::invalid_argument);
                    }
                    if (n > max_value_div_16)
                    {
                        return to_number_result<CharT>(s, std::errc::result_out_of_range);
                    }
                    n = n * 16;
                    if (n > max_value - x)
                    {
                        return to_number_result<CharT>(s, std::errc::result_out_of_range);
                    }

                    n += x;
                }
                break;
            }
            default:
                JSONCONS_UNREACHABLE();
                break;
        }
    }
    return (state == integer_chars_state::initial) ? to_number_result<CharT>(s, std::errc::invalid_argument) : to_number_result<CharT>(s, std::errc());
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && ext_traits::integer_limits<T>::is_signed,to_number_result<CharT>>::type
to_integer(const CharT* s, std::size_t length, T& n)
{
    n = 0;

    if (length == 0)
    {
        return to_number_result<CharT>(s, std::errc::invalid_argument);
    }

    bool is_negative = *s == '-' ? true : false;
    if (is_negative)
    {
        ++s;
        --length;
    }

    using U = typename ext_traits::make_unsigned<T>::type;

    U u;
    auto ru = to_integer(s, length, u);
    if (ru.ec != std::errc())
    {
        return to_number_result<CharT>(ru.ptr, ru.ec);
    }
    if (is_negative)
    {
        if (u > static_cast<U>(-((ext_traits::integer_limits<T>::lowest)()+T(1))) + U(1))
        {
            return to_number_result<CharT>(ru.ptr, std::errc::result_out_of_range);
        }
        else
        {
            n = static_cast<T>(U(0) - u);
            return to_number_result<CharT>(ru.ptr, std::errc());
        }
    }
    else
    {
        if (u > static_cast<U>((ext_traits::integer_limits<T>::max)()))
        {
            return to_number_result<CharT>(ru.ptr, std::errc::result_out_of_range);
        }
        else
        {
            n = static_cast<T>(u);
            return to_number_result<CharT>(ru.ptr, std::errc());
        }
    }
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized,to_number_result<CharT>>::type
to_integer(const CharT* s, T& n)
{
    return to_integer<T,CharT>(s, std::char_traits<CharT>::length(s), n);
}

// Precondition: s satisfies

// digit
// digit1-digits 
// - digit
// - digit1-digits

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && !ext_traits::integer_limits<T>::is_signed,to_number_result<CharT>>::type
to_integer_unchecked(const CharT* s, std::size_t length, T& n)
{
    static_assert(ext_traits::integer_limits<T>::is_specialized, "Integer type not specialized");
    JSONCONS_ASSERT(length > 0);

    n = 0;
    const CharT* end = s + length; 
    if (*s == '-')
    {
        static constexpr T min_value = (ext_traits::integer_limits<T>::lowest)();
        static constexpr T min_value_div_10 = min_value / 10;
        ++s;
        for (; s < end; ++s)
        {
            T x = (T)*s - (T)('0');
            if (n < min_value_div_10)
            {
                return to_number_result<CharT>(s, std::errc::result_out_of_range);
            }
            n = n * 10;
            if (n < min_value + x)
            {
                return to_number_result<CharT>(s, std::errc::result_out_of_range);
            }

            n -= x;
        }
    }
    else
    {
        static constexpr T max_value = (ext_traits::integer_limits<T>::max)();
        static constexpr T max_value_div_10 = max_value / 10;
        for (; s < end; ++s)
        {
            T x = static_cast<T>(*s) - static_cast<T>('0');
            if (n > max_value_div_10)
            {
                return to_number_result<CharT>(s, std::errc::result_out_of_range);
            }
            n = n * 10;
            if (n > max_value - x)
            {
                return to_number_result<CharT>(s, std::errc::result_out_of_range);
            }

            n += x;
        }
    }

    return to_number_result<CharT>(s, std::errc());
}

// Precondition: s satisfies

// digit
// digit1-digits 
// - digit
// - digit1-digits

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && ext_traits::integer_limits<T>::is_signed,to_number_result<CharT>>::type
to_integer_unchecked(const CharT* s, std::size_t length, T& n)
{
    static_assert(ext_traits::integer_limits<T>::is_specialized, "Integer type not specialized");
    JSONCONS_ASSERT(length > 0);

    n = 0;

    const CharT* end = s + length; 
    if (*s == '-')
    {
        static constexpr T min_value = (ext_traits::integer_limits<T>::lowest)();
        static constexpr T min_value_div_10 = min_value / 10;
        ++s;
        for (; s < end; ++s)
        {
            T x = (T)*s - (T)('0');
            if (n < min_value_div_10)
            {
                return to_number_result<CharT>(s, std::errc::result_out_of_range);
            }
            n = n * 10;
            if (n < min_value + x)
            {
                return to_number_result<CharT>(s, std::errc::result_out_of_range);
            }

            n -= x;
        }
    }
    else
    {
        static constexpr T max_value = (ext_traits::integer_limits<T>::max)();
        static constexpr T max_value_div_10 = max_value / 10;
        for (; s < end; ++s)
        {
            T x = static_cast<T>(*s) - static_cast<T>('0');
            if (n > max_value_div_10)
            {
                return to_number_result<CharT>(s, std::errc::result_out_of_range);
            }
            n = n * 10;
            if (n > max_value - x)
            {
                return to_number_result<CharT>(s, std::errc::result_out_of_range);
            }

            n += x;
        }
    }

    return to_number_result<CharT>(s, std::errc());
}

// hexstr_to_integer

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && ext_traits::integer_limits<T>::is_signed,to_number_result<CharT>>::type
hexstr_to_integer(const CharT* s, std::size_t length, T& n)
{
    static_assert(ext_traits::integer_limits<T>::is_specialized, "Integer type not specialized");
    JSONCONS_ASSERT(length > 0);

    n = 0;

    const CharT* end = s + length; 
    if (*s == '-')
    {
        static constexpr T min_value = (ext_traits::integer_limits<T>::lowest)();
        static constexpr T min_value_div_16 = min_value / 16;
        ++s;
        for (; s < end; ++s)
        {
            CharT c = *s;
            T x = 0;
            switch (c)
            {
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                    x = c - '0';
                    break;
                case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
                    x = c - ('a' - 10);
                    break;
                case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
                    x = c - ('A' - 10);
                    break;
                default:
                    return to_number_result<CharT>(s, std::errc::invalid_argument);
            }
            if (n < min_value_div_16)
            {
                return to_number_result<CharT>(s, std::errc::result_out_of_range);
            }
            n = n * 16;
            if (n < min_value + x)
            {
                return to_number_result<CharT>(s, std::errc::result_out_of_range);
            }
            n -= x;
        }
    }
    else
    {
        static constexpr T max_value = (ext_traits::integer_limits<T>::max)();
        static constexpr T max_value_div_16 = max_value / 16;
        for (; s < end; ++s)
        {
            CharT c = *s;
            T x = 0;
            switch (c)
            {
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                    x = c - '0';
                    break;
                case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
                    x = c - ('a' - 10);
                    break;
                case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
                    x = c - ('A' - 10);
                    break;
                default:
                    return to_number_result<CharT>(s, std::errc::invalid_argument);
            }
            if (n > max_value_div_16)
            {
                return to_number_result<CharT>(s, std::errc::result_out_of_range);
            }
            n = n * 16;
            if (n > max_value - x)
            {
                return to_number_result<CharT>(s, std::errc::result_out_of_range);
            }

            n += x;
        }
    }

    return to_number_result<CharT>(s, std::errc());
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && !ext_traits::integer_limits<T>::is_signed,to_number_result<CharT>>::type
hexstr_to_integer(const CharT* s, std::size_t length, T& n)
{
    static_assert(ext_traits::integer_limits<T>::is_specialized, "Integer type not specialized");
    JSONCONS_ASSERT(length > 0);

    n = 0;
    const CharT* end = s + length; 

    static constexpr T max_value = (ext_traits::integer_limits<T>::max)();
    static constexpr T max_value_div_16 = max_value / 16;
    for (; s < end; ++s)
    {
        CharT c = *s;
        T x = *s;
        switch (c)
        {
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                x = c - '0';
                break;
            case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
                x = c - ('a' - 10);
                break;
            case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
                x = c - ('A' - 10);
                break;
            default:
                return to_number_result<CharT>(s, std::errc::invalid_argument);
        }
        if (n > max_value_div_16)
        {
            return to_number_result<CharT>(s, std::errc::result_out_of_range);
        }
        n = n * 16;
        if (n > max_value - x)
        {
            return to_number_result<CharT>(s, std::errc::result_out_of_range);
        }

        n += x;
    }

    return to_number_result<CharT>(s, std::errc());
}

// to_double

#if defined(aJSONCONS_HAS_STD_FROM_CHARS)

    inline to_number_result<char> to_double(const char* s, std::size_t len, double& val) 
    {
        const auto res = std::from_chars(s, s+len, val);
        return to_number_result<char>{res.ptr,res.ec};
    }

    inline to_number_result<wchar_t> to_double(const wchar_t* s, std::size_t len, double& val)
    {
        std::string input(len,'0');
        for (size_t i = 0; i < len; ++i)
        {
            input[i] = static_cast<char>(s[i]);
        }
        
        const auto res = std::from_chars(input.data(), input.data()+len, val);
        return to_number_result<wchar_t>{s+(res.ptr-input.data()),res.ec};
    }

#elif defined(aJSONCONS_HAS_MSC_STRTOD_L)

    inline to_number_result<char> to_double(const char* s, std::size_t, double& val)
    {
        static _locale_t locale = _create_locale(LC_NUMERIC, "C");

        char *end = nullptr;
        val = _strtod_l(s, &end, locale);
        if (s == end)
        {
            return to_number_result<char>{end,std::errc::invalid_argument};
        }
        if (JSONCONS_UNLIKELY(val <= -HUGE_VAL || val >= HUGE_VAL))
        {
            return to_number_result<char>{end, std::errc::result_out_of_range};
        }
        return to_number_result<char>{end};
    }

    inline to_number_result<wchar_t> to_double(const wchar_t* s, std::size_t, double& val)
    {
        static _locale_t locale = _create_locale(LC_NUMERIC, "C");

        wchar_t* end = nullptr;
        val = _wcstod_l(s, &end, locale);
        if (s == end)
        {
            return to_number_result<wchar_t>{end,std::errc::invalid_argument};
        }
        if (JSONCONS_UNLIKELY(val <= -HUGE_VAL || val >= HUGE_VAL))
        {
            return to_number_result<wchar_t>{end, std::errc::result_out_of_range};
        }
        return to_number_result<wchar_t>{end};
    }


#elif defined(JSONCONS_HAS_STRTOLD_L)

    inline to_number_result<char> to_double(const char* s, std::size_t, double& val)
    {
        locale_t locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);

        char *end = nullptr;
        val = strtod_l(s, &end, locale);
        if (s == end)
        {
            return to_number_result<char>{end,std::errc::invalid_argument};
        }
        if (JSONCONS_UNLIKELY(val <= -HUGE_VAL || val >= HUGE_VAL))
        {
            return to_number_result<char>{end, std::errc::result_out_of_range};
        }
        return to_number_result<char>{end};
    }

    inline to_number_result<wchar_t> to_double(const wchar_t* s, std::size_t, double& val)
    {
        locale_t locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);

        wchar_t *end = nullptr;
        val = wcstod_l(s, &end, locale);
        if (s == end)
        {
            return to_number_result<wchar_t>{end,std::errc::invalid_argument};
        }
        if (JSONCONS_UNLIKELY(val <= -HUGE_VAL || val >= HUGE_VAL))
        {
            return to_number_result<wchar_t>{end, std::errc::result_out_of_range};
        }
        return to_number_result<wchar_t>{end};
    }

#else

    inline to_number_result<char> to_double(const char* s, std::size_t length, double& val)
    {
        const char* cur = s+length;
        char *end = nullptr;
        val = strtod(s, &end);
        const char* str_end = end;
        if (JSONCONS_UNLIKELY(end < cur))
        {
            if (*end == '.')
            {
                std::string buf{s, length};
                char* dot_ptr = &buf[0] + (cur - end - 1);
                *dot_ptr = ',';
                end = nullptr;
                val = strtod(buf.c_str(), &end);
                str_end = s + (end - &buf[0]);
            }
            if (JSONCONS_UNLIKELY(str_end != cur))
            {
                return to_number_result<char>{str_end,std::errc::invalid_argument};
            }
        }
        if (JSONCONS_UNLIKELY(val <= -HUGE_VAL || val >= HUGE_VAL))
        {
            return to_number_result<char>{str_end, std::errc::result_out_of_range};
        }
        return to_number_result<char>{str_end};
    }

    inline to_number_result<wchar_t> to_double(const wchar_t* s, std::size_t length, double& val)
    {
        const wchar_t* cur = s+length;
        wchar_t *end = nullptr;
        val = wcstod(s, &end);
        const wchar_t* str_end = end;
        if (JSONCONS_UNLIKELY(end < cur))
        {
            if (*end == '.')
            {
                std::wstring buf{s, length};
                wchar_t* dot_ptr = &buf[0] + (cur - end - 1);
                *dot_ptr = ',';
                end = nullptr;
                val = wcstod(buf.c_str(), &end);
                str_end = s + (end-&buf[0]);
            }
            if (JSONCONS_UNLIKELY(str_end != cur))
            {
                return to_number_result<wchar_t>{str_end,std::errc::invalid_argument};
            }
        }
        if (JSONCONS_UNLIKELY(val <= -HUGE_VAL || val >= HUGE_VAL))
        {
            return to_number_result<wchar_t>{str_end, std::errc::result_out_of_range};
        }
        return to_number_result<wchar_t>{str_end};
    }
    
    inline to_number_result<char> to_double(char* s, std::size_t length)
    {
        char* cur = s+length;
        char *end = nullptr;
        double val = strtod(s, &end);
        if (JSONCONS_UNLIKELY(end < cur))
        {
            if (*end == '.')
            {
                char* dot_ptr = end;
                *end = ',';
                val = strtod(s, &end);
                *dot_ptr = '.';
            }
            if (JSONCONS_UNLIKELY(end != cur))
            {
                return to_number_result<char>{end,std::errc::invalid_argument};
            }
        }
        if (JSONCONS_UNLIKELY(val <= -HUGE_VAL || val >= HUGE_VAL))
        {
            return to_number_result<char>{end, std::errc::result_out_of_range};
        }
        return to_number_result<char>{end};
    }

    inline to_number_result<wchar_t> to_double(wchar_t* s, std::size_t length)
    {
        wchar_t* cur = s+length;
        wchar_t *end = nullptr;
        double val = wcstod(s, &end);
        if (JSONCONS_UNLIKELY(end < cur))
        {
            if (*end == '.')
            {
                wchar_t* dot_ptr = end;
                *end = ',';
                val = wcstod(s, &end);
                *dot_ptr = '.';
            }
            if (JSONCONS_UNLIKELY(end != cur))
            {
                return to_number_result<wchar_t>{end,std::errc::invalid_argument};
            }
        }
        if (JSONCONS_UNLIKELY(val <= -HUGE_VAL || val >= HUGE_VAL))
        {
            return to_number_result<wchar_t>{end, std::errc::result_out_of_range};
        }
        return to_number_result<wchar_t>{end};
    }

#endif

    inline to_number_result<char> hexstr_to_double(const char* s, std::size_t length, double& val)
    {
        const char* cur = s+length;
        char *end = nullptr;
        val = strtod(s, &end);
        const char* str_end = end;
        if (JSONCONS_UNLIKELY(end < cur))
        {
            if (*end == '.')
            {
                std::string buf{s, length};
                char* dot_ptr = &buf[0] + (cur - end - 1);
                *dot_ptr = ',';
                end = nullptr;
                val = strtod(buf.c_str(), &end);
                str_end = s + (end - &buf[0]);
            }
            if (JSONCONS_UNLIKELY(str_end != cur))
            {
                return to_number_result<char>{str_end,std::errc::invalid_argument};
            }
        }
        if (JSONCONS_UNLIKELY(val <= -HUGE_VAL || val >= HUGE_VAL))
        {
            return to_number_result<char>{str_end, std::errc::result_out_of_range};
        }
        return to_number_result<char>{str_end};
    }

    inline to_number_result<wchar_t> hexstr_to_double(const wchar_t* s, std::size_t length, double& val)
    {
        const wchar_t* cur = s+length;
        wchar_t *end = nullptr;
        val = wcstod(s, &end);
        const wchar_t* str_end = end;
        if (JSONCONS_UNLIKELY(end < cur))
        {
            if (*end == '.')
            {
                std::wstring buf{s, length};
                wchar_t* dot_ptr = &buf[0] + (cur - end - 1);
                *dot_ptr = ',';
                end = nullptr;
                val = wcstod(buf.c_str(), &end);
                str_end = s + (end-&buf[0]);
            }
            if (JSONCONS_UNLIKELY(str_end != cur))
            {
                return to_number_result<wchar_t>{str_end,std::errc::invalid_argument};
            }
        }
        if (JSONCONS_UNLIKELY(val <= -HUGE_VAL || val >= HUGE_VAL))
        {
            return to_number_result<wchar_t>{str_end, std::errc::result_out_of_range};
        }
        return to_number_result<wchar_t>{str_end};
    }
    
} // namespace utility
} // namespace jsoncons

#endif // JSONCONS_UTILITY_TO_NUMBER_HPP
