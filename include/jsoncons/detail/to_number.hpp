// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_TO_NUMBER_HPP
#define JSONCONS_DETAIL_TO_NUMBER_HPP

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
namespace detail {

    enum class to_number_errc : uint8_t {success=0, overflow, invalid_number};

    class to_number_error_category_impl
       : public std::error_category
    {
    public:
        const char* name() const noexcept override
        {
            return "jsoncons/to_number";
        }
        std::string message(int ev) const override
        {
            switch (static_cast<to_number_errc>(ev))
            {
                case to_number_errc::success:
                    return "Success";
                case to_number_errc::overflow:
                    return "Integer overflow";
                case to_number_errc::invalid_number:
                    return "Invalid number";
                default:
                    return "Unknown to_number error";
            }
        }
    };

    inline
    const std::error_category& to_number_error_category() noexcept
    {
      static to_number_error_category_impl instance;
      return instance;
    }

    inline 
    std::error_code make_error_code(to_number_errc e) noexcept
    {
        return std::error_code(static_cast<int>(e),to_number_error_category());
    }

} // namespace detail
} // namespace jsoncons

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::detail::to_number_errc> : public true_type
    {
    };
} // namespace std

namespace jsoncons { 
namespace detail {

template <typename T,typename CharT>
struct to_number_result
{
    const CharT* ptr;
    to_number_errc ec;
    constexpr to_number_result(const CharT* ptr_)
        : ptr(ptr_), ec(to_number_errc())
    {
    }
    constexpr to_number_result(const CharT* ptr_, to_number_errc ec_)
        : ptr(ptr_), ec(ec_)
    {
    }

    to_number_result(const to_number_result&) = default;

    to_number_result& operator=(const to_number_result&) = default;

    constexpr explicit operator bool() const noexcept
    {
        return ec == to_number_errc();
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
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && !ext_traits::integer_limits<T>::is_signed,to_number_result<T,CharT>>::type
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
                            return (++s == end) ? to_number_result<T,CharT>(s) : to_number_result<T, CharT>(s, to_number_errc());
                        }
                        else
                        {
                            return to_number_result<T,CharT>(s, to_number_errc::invalid_number);
                        }
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9': // Must be decimal
                        state = integer_chars_state::decimal;
                        break;
                    default:
                        return to_number_result<T,CharT>(s, to_number_errc::invalid_number);
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
                            return to_number_result<T,CharT>(s, to_number_errc::invalid_number);
                    }
                    if (n > max_value_div_10)
                    {
                        return to_number_result<T,CharT>(s, to_number_errc::overflow);
                    }
                    n = n * 10;
                    if (n > max_value - x)
                    {
                        return to_number_result<T,CharT>(s, to_number_errc::overflow);
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
    return (state == integer_chars_state::initial) ? to_number_result<T,CharT>(s, to_number_errc::invalid_number) : to_number_result<T,CharT>(s, to_number_errc());
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && ext_traits::integer_limits<T>::is_signed,to_number_result<T,CharT>>::type
decstr_to_integer(const CharT* s, std::size_t length, T& n)
{
    n = 0;

    if (length == 0)
    {
        return to_number_result<T,CharT>(s, to_number_errc::invalid_number);
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
    if (ru.ec != to_number_errc())
    {
        return to_number_result<T,CharT>(ru.ptr, ru.ec);
    }
    if (is_negative)
    {
        if (u > static_cast<U>(-((ext_traits::integer_limits<T>::lowest)()+T(1))) + U(1))
        {
            return to_number_result<T,CharT>(ru.ptr, to_number_errc::overflow);
        }
        else
        {
            n = static_cast<T>(U(0) - u);
            return to_number_result<T,CharT>(ru.ptr, to_number_errc());
        }
    }
    else
    {
        if (u > static_cast<U>((ext_traits::integer_limits<T>::max)()))
        {
            return to_number_result<T,CharT>(ru.ptr, to_number_errc::overflow);
        }
        else
        {
            n = static_cast<T>(u);
            return to_number_result<T,CharT>(ru.ptr, to_number_errc());
        }
    }
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && !ext_traits::integer_limits<T>::is_signed,to_number_result<T,CharT>>::type
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
                        return to_number_result<T,CharT>(s, to_number_errc::invalid_number);
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
                        return to_number_result<T,CharT>(s, to_number_errc::invalid_number);
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
                            return to_number_result<T,CharT>(s, to_number_errc::invalid_number);
                    }
                    if (n > max_value_div_2)
                    {
                        return to_number_result<T,CharT>(s, to_number_errc::overflow);
                    }
                    n = n * 2;
                    if (n > max_value - x)
                    {
                        return to_number_result<T,CharT>(s, to_number_errc::overflow);
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
                            return to_number_result<T,CharT>(s, to_number_errc::invalid_number);
                    }
                    if (n > max_value_div_8)
                    {
                        return to_number_result<T,CharT>(s, to_number_errc::overflow);
                    }
                    n = n * 8;
                    if (n > max_value - x)
                    {
                        return to_number_result<T,CharT>(s, to_number_errc::overflow);
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
                            return to_number_result<T,CharT>(s, to_number_errc::invalid_number);
                    }
                    if (n > max_value_div_10)
                    {
                        return to_number_result<T,CharT>(s, to_number_errc::overflow);
                    }
                    n = n * 10;
                    if (n > max_value - x)
                    {
                        return to_number_result<T,CharT>(s, to_number_errc::overflow);
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
                            return to_number_result<T,CharT>(s, to_number_errc::invalid_number);
                    }
                    if (n > max_value_div_16)
                    {
                        return to_number_result<T,CharT>(s, to_number_errc::overflow);
                    }
                    n = n * 16;
                    if (n > max_value - x)
                    {
                        return to_number_result<T,CharT>(s, to_number_errc::overflow);
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
    return (state == integer_chars_state::initial) ? to_number_result<T,CharT>(s, to_number_errc::invalid_number) : to_number_result<T,CharT>(s, to_number_errc());
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && ext_traits::integer_limits<T>::is_signed,to_number_result<T,CharT>>::type
to_integer(const CharT* s, std::size_t length, T& n)
{
    n = 0;

    if (length == 0)
    {
        return to_number_result<T,CharT>(s, to_number_errc::invalid_number);
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
    if (ru.ec != to_number_errc())
    {
        return to_number_result<T,CharT>(ru.ptr, ru.ec);
    }
    if (is_negative)
    {
        if (u > static_cast<U>(-((ext_traits::integer_limits<T>::lowest)()+T(1))) + U(1))
        {
            return to_number_result<T,CharT>(ru.ptr, to_number_errc::overflow);
        }
        else
        {
            n = static_cast<T>(U(0) - u);
            return to_number_result<T,CharT>(ru.ptr, to_number_errc());
        }
    }
    else
    {
        if (u > static_cast<U>((ext_traits::integer_limits<T>::max)()))
        {
            return to_number_result<T,CharT>(ru.ptr, to_number_errc::overflow);
        }
        else
        {
            n = static_cast<T>(u);
            return to_number_result<T,CharT>(ru.ptr, to_number_errc());
        }
    }
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized,to_number_result<T,CharT>>::type
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
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && !ext_traits::integer_limits<T>::is_signed,to_number_result<T,CharT>>::type
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
                return to_number_result<T,CharT>(s, to_number_errc::overflow);
            }
            n = n * 10;
            if (n < min_value + x)
            {
                return to_number_result<T,CharT>(s, to_number_errc::overflow);
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
                return to_number_result<T,CharT>(s, to_number_errc::overflow);
            }
            n = n * 10;
            if (n > max_value - x)
            {
                return to_number_result<T,CharT>(s, to_number_errc::overflow);
            }

            n += x;
        }
    }

    return to_number_result<T,CharT>(s, to_number_errc());
}

// Precondition: s satisfies

// digit
// digit1-digits 
// - digit
// - digit1-digits

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && ext_traits::integer_limits<T>::is_signed,to_number_result<T,CharT>>::type
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
                return to_number_result<T,CharT>(s, to_number_errc::overflow);
            }
            n = n * 10;
            if (n < min_value + x)
            {
                return to_number_result<T,CharT>(s, to_number_errc::overflow);
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
                return to_number_result<T,CharT>(s, to_number_errc::overflow);
            }
            n = n * 10;
            if (n > max_value - x)
            {
                return to_number_result<T,CharT>(s, to_number_errc::overflow);
            }

            n += x;
        }
    }

    return to_number_result<T,CharT>(s, to_number_errc());
}

// hexstr_to_integer

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && ext_traits::integer_limits<T>::is_signed,to_number_result<T,CharT>>::type
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
                    return to_number_result<T,CharT>(s, to_number_errc::invalid_number);
            }
            if (n < min_value_div_16)
            {
                return to_number_result<T,CharT>(s, to_number_errc::overflow);
            }
            n = n * 16;
            if (n < min_value + x)
            {
                return to_number_result<T,CharT>(s, to_number_errc::overflow);
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
                    return to_number_result<T,CharT>(s, to_number_errc::invalid_number);
            }
            if (n > max_value_div_16)
            {
                return to_number_result<T,CharT>(s, to_number_errc::overflow);
            }
            n = n * 16;
            if (n > max_value - x)
            {
                return to_number_result<T,CharT>(s, to_number_errc::overflow);
            }

            n += x;
        }
    }

    return to_number_result<T,CharT>(s, to_number_errc());
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::integer_limits<T>::is_specialized && !ext_traits::integer_limits<T>::is_signed,to_number_result<T,CharT>>::type
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
                return to_number_result<T,CharT>(s, to_number_errc::invalid_number);
        }
        if (n > max_value_div_16)
        {
            return to_number_result<T,CharT>(s, to_number_errc::overflow);
        }
        n = n * 16;
        if (n > max_value - x)
        {
            return to_number_result<T,CharT>(s, to_number_errc::overflow);
        }

        n += x;
    }

    return to_number_result<T,CharT>(s, to_number_errc());
}

#if defined(aJSONCONS_HAS_STD_FROM_CHARS)

class chars_to
{
public:

    char get_decimal_point() const
    {
        return '.';
    }

    template <typename CharT>
    typename std::enable_if<std::is_same<CharT,char>::value,double>::type
    operator()(const CharT* s, std::size_t len) const
    {
        double val = 0;
        const auto res = std::from_chars(s, s+len, val);
        if (res.ec != std::errc())
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert chars to double failed"));
        }
        return val;
    }

    template <typename CharT>
    typename std::enable_if<std::is_same<CharT,wchar_t>::value,double>::type
    operator()(const CharT* s, std::size_t len) const
    {
        std::string input(len,'0');
        for (size_t i = 0; i < len; ++i)
        {
            input[i] = static_cast<char>(s[i]);
        }

        double val = 0;
        const auto res = std::from_chars(input.data(), input.data() + len, val);
        if (res.ec != std::errc())
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert chars to double failed"));
        }
        return val;
    }
};
#elif defined(JSONCONS_HAS_MSC_STRTOD_L)

class chars_to
{
private:
    _locale_t locale_;
public:
    chars_to()
    {
        locale_ = _create_locale(LC_NUMERIC, "C");
    }
    ~chars_to() noexcept
    {
        _free_locale(locale_);
    }

    chars_to(const chars_to&)
    {
        locale_ = _create_locale(LC_NUMERIC, "C");
    }

    chars_to& operator=(const chars_to&) 
    {
        // Don't assign locale
        return *this;
    }

    char get_decimal_point() const
    {
        return '.';
    }

    template <typename CharT>
    typename std::enable_if<std::is_same<CharT,char>::value,double>::type
    operator()(const CharT* s, std::size_t) const
    {
        CharT *end = nullptr;
        double val = _strtod_l(s, &end, locale_);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }

    template <typename CharT>
    typename std::enable_if<std::is_same<CharT,wchar_t>::value,double>::type
    operator()(const CharT* s, std::size_t) const
    {
        CharT *end = nullptr;
        double val = _wcstod_l(s, &end, locale_);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }
};

#elif defined(JSONCONS_HAS_STRTOLD_L)

class chars_to
{
private:
    locale_t locale_;
public:
    chars_to()
    {
        locale_ = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
    }
    ~chars_to() noexcept
    {
        freelocale(locale_);
    }

    chars_to(const chars_to&)
    {
        locale_ = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
    }

    chars_to& operator=(const chars_to&) 
    {
        return *this;
    }

    char get_decimal_point() const
    {
        return '.';
    }

    template <typename CharT>
    typename std::enable_if<std::is_same<CharT,char>::value,double>::type
    operator()(const CharT* s, std::size_t) const
    {
        char *end = nullptr;
        double val = strtold_l(s, &end, locale_);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }

    template <typename CharT>
    typename std::enable_if<std::is_same<CharT,wchar_t>::value,double>::type
    operator()(const CharT* s, std::size_t) const
    {
        CharT *end = nullptr;
        double val = wcstold_l(s, &end, locale_);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }
};

#else
class chars_to
{
private:
    std::vector<char> buffer_;
    char decimal_point_;
public:
    chars_to()
        : buffer_()
    {
        struct lconv * lc = localeconv();
        if (lc != nullptr && lc->decimal_point[0] != 0)
        {
            decimal_point_ = lc->decimal_point[0];    
        }
        else
        {
            decimal_point_ = '.'; 
        }
        buffer_.reserve(100);
    }

    chars_to(const chars_to&) = default;
    chars_to& operator=(const chars_to&) = default;

    char get_decimal_point() const
    {
        return decimal_point_;
    }

    template <typename CharT>
    typename std::enable_if<std::is_same<CharT,char>::value,double>::type
    operator()(const CharT* s, std::size_t /*length*/) const
    {
        CharT *end = nullptr;
        double val = strtod(s, &end);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }

    template <typename CharT>
    typename std::enable_if<std::is_same<CharT,wchar_t>::value,double>::type
    operator()(const CharT* s, std::size_t /*length*/) const
    {
        CharT *end = nullptr;
        double val = wcstod(s, &end);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }
};
#endif

// str_to_double

#if defined(JSONCONS_HAS_STD_FROM_CHARS)

    inline double str_to_double(const char* s, std::size_t len) 
    {
        double val = 0;
        const auto res = std::from_chars(s, s+len, val);
        if (res.ec != std::errc())
        {
            if (res.ec == std::errc::result_out_of_range)
            {
                return val;
            }
            else
            {
                JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert chars to double failed"));
            }
        }
        return val;
    }

    inline double str_to_double(const wchar_t* s, std::size_t len) 
    {
        std::string input(len,'0');
        for (size_t i = 0; i < len; ++i)
        {
            input[i] = static_cast<char>(s[i]);
        }
        
        return str_to_double(input.data(), len);
    }

#elif defined(JSONCONS_HAS_MSC_STRTOD_L)

    inline double str_to_double(const char* s, std::size_t)
    {
        static _locale_t locale = _create_locale(LC_NUMERIC, "C");

        char *end = nullptr;
        double val = _strtod_l(s, &end, locale);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }

    inline double str_to_double(const wchar_t* s, std::size_t)
    {
        static _locale_t locale = _create_locale(LC_NUMERIC, "C");

        wchar_t* end = nullptr;
        double val = _wcstod_l(s, &end, locale);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }


#elif defined(JSONCONS_HAS_STRTOLD_L)

    inline double str_to_double(const char* s, std::size_t)
    {
        locale_t locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);

        char *end = nullptr;
        double val = strtod_l(s, &end, locale);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }

    inline double str_to_double(const wchar_t* s, std::size_t)
    {
        locale_t locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);

        wchar_t *end = nullptr;
        double val = wcstod_l(s, &end, locale);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }

#else

    inline double str_to_double(char* s, std::size_t length)
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
                JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
            }
        }
        return val;
    }

    inline double str_to_double(wchar_t* s, std::size_t length)
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
                JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
            }
        }
        return val;
    }
#endif

} // namespace detail
} // namespace jsoncons

#endif // JSONCONS_DETAIL_TO_NUMBER_HPP
