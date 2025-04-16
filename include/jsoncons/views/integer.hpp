// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS2_UTILITY_INTEGER_HPP
#define JSONCONS2_UTILITY_INTEGER_HPP

#include <system_error>
#include <stdexcept>
#include <string>
#include <vector>
#include <locale>
#include <string>
#include <limits> // std::numeric_limits
#include <type_traits> // std::enable_if
#include <exception>
#include <jsoncons/views/jsoncons_config.hpp>
#include <jsoncons/views/jsoncons_exception.hpp>
#include <jsoncons/views/more_type_traits.hpp>
#include <jsoncons/views/more_concepts.hpp>
#include <cctype>

namespace jsoncons { namespace utility {

    // to_integer

    enum class to_integer_errc : uint8_t {success=0, overflow, invalid_digit, invalid_number};

    class to_integer_error_category_impl
       : public std::error_category
    {
    public:
        const char* name() const noexcept override
        {
            return "jsoncons/unsafe_to_integer";
        }
        std::string message(int ev) const override
        {
            switch (static_cast<to_integer_errc>(ev))
            {
                case to_integer_errc::overflow:
                    return "Integer overflow";
                case to_integer_errc::invalid_digit:
                    return "Invalid digit";
                case to_integer_errc::invalid_number:
                    return "Invalid number";
                default:
                    return "Unknown unsafe_to_integer error";
            }
        }
    };

    inline
    const std::error_category& to_integer_error_category()
    {
      static to_integer_error_category_impl instance;
      return instance;
    }

    inline 
    std::error_code make_error_code(to_integer_errc e)
    {
        return std::error_code(static_cast<int>(e),to_integer_error_category());
    }

} // namespace utility
} // namespace jsoncons

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::utility::to_integer_errc> : public true_type
    {
    };
}

namespace jsoncons { namespace utility {

template <typename T>
struct to_integer_result
{
    const char* ptr;
    to_integer_errc ec;
    constexpr to_integer_result(const char* ptr_)
        : ptr(ptr_), ec(to_integer_errc())
    {
    }
    constexpr to_integer_result(const char* ptr_, to_integer_errc ec_)
        : ptr(ptr_), ec(ec_)
    {
    }

    to_integer_result(const to_integer_result&) = default;

    to_integer_result& operator=(const to_integer_result&) = default;

    constexpr explicit operator bool() const noexcept
    {
        return ec == to_integer_errc();
    }
    std::error_code error_code() const
    {
        return make_error_code(ec);
    }
};

enum class integer_chars_format : uint8_t {decimal=1,hex};
enum class integer_chars_state {initial,minus,integer,binary,octal,decimal,base16};

inline
bool is_base10(const char* s, std::size_t length)
{
    integer_chars_state state = integer_chars_state::initial;

    const char* end = s + length; 
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

template <typename T>
bool is_base16(const char* s, std::size_t length)
{
    integer_chars_state state = integer_chars_state::initial;

    const char* end = s + length; 
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

template <typename T>
requires unsigned_extended_integral<T>
to_integer_result<T> decimal_to_integer(const char* s, std::size_t length, T& n)
{
    n = 0;

    integer_chars_state state = integer_chars_state::initial;

    const char* end = s + length; 
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
                            return (++s == end) ? to_integer_result<T>(s) : to_integer_result<T>(s, to_integer_errc());
                        }
                        else
                        {
                            return to_integer_result<T>(s, to_integer_errc::invalid_digit);
                        }
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9': // Must be decimal
                        state = integer_chars_state::decimal;
                        break;
                    default:
                        return to_integer_result<T>(s, to_integer_errc::invalid_digit);
                }
                break;
            }
            case integer_chars_state::decimal:
            {
                static constexpr T max_value = (utility::integer_limits<T>::max)();
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
                            return to_integer_result<T>(s, to_integer_errc::invalid_digit);
                    }
                    if (n > max_value_div_10)
                    {
                        return to_integer_result<T>(s, to_integer_errc::overflow);
                    }
                    n = n * 10;
                    if (n > max_value - x)
                    {
                        return to_integer_result<T>(s, to_integer_errc::overflow);
                    }
                    n += x;
                }
                break;
            }
            default:
                JSONCONS2_UNREACHABLE();
                break;
        }
    }
    return (state == integer_chars_state::initial) ? to_integer_result<T>(s, to_integer_errc::invalid_number) : to_integer_result<T>(s, to_integer_errc());
}

template <typename T>
requires signed_extended_integral<T>
to_integer_result<T> decimal_to_integer(const char* s, std::size_t length, T& n)
{
    n = 0;

    if (length == 0)
    {
        return to_integer_result<T>(s, to_integer_errc::invalid_number);
    }

    bool is_negative = *s == '-' ? true : false;
    if (is_negative)
    {
        ++s;
        --length;
    }

    using U = typename utility::make_unsigned<T>::type;

    U u;
    auto ru = decimal_to_integer(s, length, u);
    if (ru.ec != to_integer_errc())
    {
        return to_integer_result<T>(ru.ptr, ru.ec);
    }
    if (is_negative)
    {
        if (u > static_cast<U>(-((utility::integer_limits<T>::lowest)()+T(1))) + U(1))
        {
            return to_integer_result<T>(ru.ptr, to_integer_errc::overflow);
        }
        else
        {
            n = static_cast<T>(U(0) - u);
            return to_integer_result<T>(ru.ptr, to_integer_errc());
        }
    }
    else
    {
        if (u > static_cast<U>((utility::integer_limits<T>::max)()))
        {
            return to_integer_result<T>(ru.ptr, to_integer_errc::overflow);
        }
        else
        {
            n = static_cast<T>(u);
            return to_integer_result<T>(ru.ptr, to_integer_errc());
        }
    }
}

template <typename T>
requires unsigned_extended_integral<T>
to_integer_result<T> to_integer(const char* s, std::size_t length, T& n)
{
    n = 0;

    integer_chars_state state = integer_chars_state::initial;

    const char* end = s + length; 
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
                        return to_integer_result<T>(s, to_integer_errc::invalid_digit);
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
                        return to_integer_result<T>(s, to_integer_errc::invalid_digit);
                }
                break;
            }
            case integer_chars_state::binary:
            {
                static constexpr T max_value = (utility::integer_limits<T>::max)();
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
                            return to_integer_result<T>(s, to_integer_errc::invalid_digit);
                    }
                    if (n > max_value_div_2)
                    {
                        return to_integer_result<T>(s, to_integer_errc::overflow);
                    }
                    n = n * 2;
                    if (n > max_value - x)
                    {
                        return to_integer_result<T>(s, to_integer_errc::overflow);
                    }
                    n += x;
                }
                break;
            }
            case integer_chars_state::octal:
            {
                static constexpr T max_value = (utility::integer_limits<T>::max)();
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
                            return to_integer_result<T>(s, to_integer_errc::invalid_digit);
                    }
                    if (n > max_value_div_8)
                    {
                        return to_integer_result<T>(s, to_integer_errc::overflow);
                    }
                    n = n * 8;
                    if (n > max_value - x)
                    {
                        return to_integer_result<T>(s, to_integer_errc::overflow);
                    }
                    n += x;
                }
                break;
            }
            case integer_chars_state::decimal:
            {
                static constexpr T max_value = (utility::integer_limits<T>::max)();
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
                            return to_integer_result<T>(s, to_integer_errc::invalid_digit);
                    }
                    if (n > max_value_div_10)
                    {
                        return to_integer_result<T>(s, to_integer_errc::overflow);
                    }
                    n = n * 10;
                    if (n > max_value - x)
                    {
                        return to_integer_result<T>(s, to_integer_errc::overflow);
                    }
                    n += x;
                }
                break;
            }
            case integer_chars_state::base16:
            {
                static constexpr T max_value = (utility::integer_limits<T>::max)();
                static constexpr T max_value_div_16 = max_value / 16;
                for (; s < end; ++s)
                {
                    char c = *s;
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
                            return to_integer_result<T>(s, to_integer_errc::invalid_digit);
                    }
                    if (n > max_value_div_16)
                    {
                        return to_integer_result<T>(s, to_integer_errc::overflow);
                    }
                    n = n * 16;
                    if (n > max_value - x)
                    {
                        return to_integer_result<T>(s, to_integer_errc::overflow);
                    }

                    n += x;
                }
                break;
            }
            default:
                JSONCONS2_UNREACHABLE();
                break;
        }
    }
    return (state == integer_chars_state::initial) ? to_integer_result<T>(s, to_integer_errc::invalid_number) : to_integer_result<T>(s, to_integer_errc());
}

template <typename T>
requires signed_extended_integral<T>
to_integer_result<T> to_integer(const char* s, std::size_t length, T& n)
{
    n = 0;

    if (length == 0)
    {
        return to_integer_result<T>(s, to_integer_errc::invalid_number);
    }

    bool is_negative = *s == '-' ? true : false;
    if (is_negative)
    {
        ++s;
        --length;
    }

    using U = typename utility::make_unsigned<T>::type;

    U u;
    auto ru = to_integer(s, length, u);
    if (ru.ec != to_integer_errc())
    {
        return to_integer_result<T>(ru.ptr, ru.ec);
    }
    if (is_negative)
    {
        if (u > static_cast<U>(-((utility::integer_limits<T>::lowest)()+T(1))) + U(1))
        {
            return to_integer_result<T>(ru.ptr, to_integer_errc::overflow);
        }
        else
        {
            n = static_cast<T>(U(0) - u);
            return to_integer_result<T>(ru.ptr, to_integer_errc());
        }
    }
    else
    {
        if (u > static_cast<U>((utility::integer_limits<T>::max)()))
        {
            return to_integer_result<T>(ru.ptr, to_integer_errc::overflow);
        }
        else
        {
            n = static_cast<T>(u);
            return to_integer_result<T>(ru.ptr, to_integer_errc());
        }
    }
}

template <typename T>
requires extended_integral<T>
to_integer_result<T> to_integer(const char* s, T& n)
{
    return to_integer<T,char>(s, std::char_traits<char>::length(s), n);
}

// Precondition: s satisfies

// digit
// digit1-digits 
// - digit
// - digit1-digits

template <typename T>
requires unsigned_extended_integral<T>
to_integer_result<T> unsafe_to_integer(const char* s, std::size_t length, T& n)
{
    static_assert(utility::integer_limits<T>::is_specialized, "Integer type not specialized");
    JSONCONS2_ASSERT(length > 0);

    n = 0;
    const char* end = s + length; 
    if (*s == '-')
    {
        static constexpr T min_value = (utility::integer_limits<T>::lowest)();
        static constexpr T min_value_div_10 = min_value / 10;
        ++s;
        for (; s < end; ++s)
        {
            T x = (T)*s - (T)('0');
            if (n < min_value_div_10)
            {
                return to_integer_result<T>(s, to_integer_errc::overflow);
            }
            n = n * 10;
            if (n < min_value + x)
            {
                return to_integer_result<T>(s, to_integer_errc::overflow);
            }

            n -= x;
        }
    }
    else
    {
        static constexpr T max_value = (utility::integer_limits<T>::max)();
        static constexpr T max_value_div_10 = max_value / 10;
        for (; s < end; ++s)
        {
            T x = static_cast<T>(*s) - static_cast<T>('0');
            if (n > max_value_div_10)
            {
                return to_integer_result<T>(s, to_integer_errc::overflow);
            }
            n = n * 10;
            if (n > max_value - x)
            {
                return to_integer_result<T>(s, to_integer_errc::overflow);
            }

            n += x;
        }
    }

    return to_integer_result<T>(s, to_integer_errc());
}

// Precondition: s satisfies

// digit
// digit1-digits 
// - digit
// - digit1-digits

template <typename T>
requires signed_extended_integral<T>
to_integer_result<T> unsafe_to_integer(const char* s, std::size_t length, T& n)
{
    static_assert(utility::integer_limits<T>::is_specialized, "Integer type not specialized");
    JSONCONS2_ASSERT(length > 0);

    n = 0;

    const char* end = s + length; 
    if (*s == '-')
    {
        static constexpr T min_value = (utility::integer_limits<T>::lowest)();
        static constexpr T min_value_div_10 = min_value / 10;
        ++s;
        for (; s < end; ++s)
        {
            T x = (T)*s - (T)('0');
            if (n < min_value_div_10)
            {
                return to_integer_result<T>(s, to_integer_errc::overflow);
            }
            n = n * 10;
            if (n < min_value + x)
            {
                return to_integer_result<T>(s, to_integer_errc::overflow);
            }

            n -= x;
        }
    }
    else
    {
        static constexpr T max_value = (utility::integer_limits<T>::max)();
        static constexpr T max_value_div_10 = max_value / 10;
        for (; s < end; ++s)
        {
            T x = static_cast<T>(*s) - static_cast<T>('0');
            if (n > max_value_div_10)
            {
                return to_integer_result<T>(s, to_integer_errc::overflow);
            }
            n = n * 10;
            if (n > max_value - x)
            {
                return to_integer_result<T>(s, to_integer_errc::overflow);
            }

            n += x;
        }
    }

    return to_integer_result<T>(s, to_integer_errc());
}

// hex_to_integer

template <typename T>
requires signed_extended_integral<T>
to_integer_result<T> hex_to_integer(const char* s, std::size_t length, T& n)
{
    static_assert(utility::integer_limits<T>::is_specialized, "Integer type not specialized");
    JSONCONS2_ASSERT(length > 0);

    n = 0;

    const char* end = s + length; 
    if (*s == '-')
    {
        static constexpr T min_value = (utility::integer_limits<T>::lowest)();
        static constexpr T min_value_div_16 = min_value / 16;
        ++s;
        for (; s < end; ++s)
        {
            char c = *s;
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
                    return to_integer_result<T>(s, to_integer_errc::invalid_digit);
            }
            if (n < min_value_div_16)
            {
                return to_integer_result<T>(s, to_integer_errc::overflow);
            }
            n = n * 16;
            if (n < min_value + x)
            {
                return to_integer_result<T>(s, to_integer_errc::overflow);
            }
            n -= x;
        }
    }
    else
    {
        static constexpr T max_value = (utility::integer_limits<T>::max)();
        static constexpr T max_value_div_16 = max_value / 16;
        for (; s < end; ++s)
        {
            char c = *s;
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
                    return to_integer_result<T>(s, to_integer_errc::invalid_digit);
            }
            if (n > max_value_div_16)
            {
                return to_integer_result<T>(s, to_integer_errc::overflow);
            }
            n = n * 16;
            if (n > max_value - x)
            {
                return to_integer_result<T>(s, to_integer_errc::overflow);
            }

            n += x;
        }
    }

    return to_integer_result<T>(s, to_integer_errc());
}

template <typename T>
requires unsigned_extended_integral<T>
to_integer_result<T> hex_to_integer(const char* s, std::size_t length, T& n)
{
    static_assert(utility::integer_limits<T>::is_specialized, "Integer type not specialized");
    JSONCONS2_ASSERT(length > 0);

    n = 0;
    const char* end = s + length; 

    static constexpr T max_value = (utility::integer_limits<T>::max)();
    static constexpr T max_value_div_16 = max_value / 16;
    for (; s < end; ++s)
    {
        char c = *s;
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
                return to_integer_result<T>(s, to_integer_errc::invalid_digit);
        }
        if (n > max_value_div_16)
        {
            return to_integer_result<T>(s, to_integer_errc::overflow);
        }
        n = n * 16;
        if (n > max_value - x)
        {
            return to_integer_result<T>(s, to_integer_errc::overflow);
        }

        n += x;
    }

    return to_integer_result<T>(s, to_integer_errc());
}

// integer_to_decimal

inline
char to_hex_character(uint8_t c)
{
    return (char)((c < 10) ? ('0' + c) : ('A' - 10 + c));
}

// integer_to_decimal

template <typename Integer, typename Result>
requires extended_integral<Integer>
std::size_t integer_to_decimal(Integer value, Result& result)
{
    using char_type = typename Result::value_type;

    char_type buf[255];
    char_type *p = buf;
    const char_type* last = buf+255;

    bool is_negative = value < 0;

    if (value < 0)
    {
        do
        {
            *p++ = static_cast<char_type>(48 - (value % 10));
        }
        while ((value /= 10) && (p < last));
    }
    else
    {

        do
        {
            *p++ = static_cast<char_type>(48 + value % 10);
        }
        while ((value /= 10) && (p < last));
    }
    JSONCONS2_ASSERT(p != last);

    std::size_t count = (p - buf);
    if (is_negative)
    {
        result.push_back('-');
        ++count;
    }
    while (--p >= buf)
    {
        result.push_back(*p);
    }

    return count;
}

// integer_to_hex

template <typename Integer, typename Result>
requires extended_integral<Integer>
std::size_t integer_to_hex(Integer value, Result& result)
{
    using char_type = typename Result::value_type;

    char_type buf[255];
    char_type *p = buf;
    const char_type* last = buf+255;

    bool is_negative = value < 0;

    if (value < 0)
    {
        do
        {
            *p++ = to_hex_character(0-(value % 16));
        }
        while ((value /= 16) && (p < last));
    }
    else
    {

        do
        {
            *p++ = to_hex_character(value % 16);
        }
        while ((value /= 16) && (p < last));
    }
    JSONCONS2_ASSERT(p != last);

    std::size_t count = (p - buf);
    if (is_negative)
    {
        result.push_back('-');
        ++count;
    }
    while (--p >= buf)
    {
        result.push_back(*p);
    }

    return count;
}

} // namespace utility
} // namespace jsoncons

#endif
