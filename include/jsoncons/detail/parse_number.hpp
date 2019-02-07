// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_PARSE_NUMBER_HPP
#define JSONCONS_DETAIL_PARSE_NUMBER_HPP

#include <stdexcept>
#include <string>
#include <vector>
#include <locale>
#include <limits> // std::numeric_limits
#include <type_traits> // std::enable_if
#include <exception>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons { namespace detail {

template <class T>
struct to_integer_result
{
    T value;
    bool overflow;
};

template <class CharT>
bool is_integer(const CharT* s, size_t length)
{
    const CharT* end = s + length; 
    if (s == end)
    {
        return false;
    }
    if (*s == '-')
    {
        ++s;
    }
    if (s == end)
    {
        return false;
    }
    for (;s < end; ++s)
    {
        if (!(*s >= '0' && *s <= '9'))
        {
            return false;
        }
    }
    return true;
}

template <class CharT>
bool is_uinteger(const CharT* s, size_t length)
{
    const CharT* end = s + length; 
    if (s == end)
    {
        return false;
    }
    for (;s < end; ++s)
    {
        if (!(*s >= '0' && *s <= '9'))
        {
            return false;
        }
    }
    return true;
}

// Precondition: s satisfies

// digit
// digit1-digits 
// - digit
// - digit1-digits

template <class T, class CharT>
typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value,to_integer_result<T>>::type
to_integer(const CharT* s, size_t length)
{
    static_assert(std::numeric_limits<T>::is_specialized, "Integer type not specialized");
    JSONCONS_ASSERT(length > 0);

    T n = 0;
    bool overflow = false;
    const CharT* end = s + length; 
    if (*s == '-')
    {
        static const T min_value = (std::numeric_limits<T>::lowest)();
        static const T min_value_div_10 = min_value / 10;
        ++s;
        for (; s < end; ++s)
        {
            T x = *s - '0';
            if (n < min_value_div_10)
            {
                overflow = true;
                break;
            }
            n = n * 10;
            if (n < min_value + x)
            {
                overflow = true;
                break;
            }

            n -= x;
        }
    }
    else
    {
        static const T max_value = (std::numeric_limits<T>::max)();
        static const T max_value_div_10 = max_value / 10;
        for (; s < end; ++s)
        {
            T x = *s - '0';
            if (n > max_value_div_10)
            {
                overflow = true;
                break;
            }
            n = n * 10;
            if (n > max_value - x)
            {
                overflow = true;
                break;
            }

            n += x;
        }
    }

    return to_integer_result<T>({ n,overflow });
}

// Precondition: s satisfies

// digit
// digit1-digits 
// - digit
// - digit1-digits

template <class T, class CharT>
typename std::enable_if<std::is_integral<T>::value && !std::is_signed<T>::value,to_integer_result<T>>::type
to_integer(const CharT* s, size_t length)
{
    static_assert(std::numeric_limits<T>::is_specialized, "Integer type not specialized");
    JSONCONS_ASSERT(length > 0);

    T n = 0;
    bool overflow = false;
    const CharT* end = s + length; 

    static const T max_value = (std::numeric_limits<T>::max)();
    static const T max_value_div_10 = max_value / 10;
    for (; s < end; ++s)
    {
        T x = *s - '0';
        if (n > max_value_div_10)
        {
            overflow = true;
            break;
        }
        n = n * 10;
        if (n > max_value - x)
        {
            overflow = true;
            break;
        }

        n += x;
    }

    return to_integer_result<T>({ n,overflow });
}

#if defined(JSONCONS_HAS_MSC__STRTOD_L)

class string_to_double
{
private:
    _locale_t locale_;
public:
    string_to_double()
    {
        locale_ = _create_locale(LC_NUMERIC, "C");
    }
    ~string_to_double()
    {
        _free_locale(locale_);
    }

    char get_decimal_point() const
    {
        return '.';
    }

    template <class CharT>
    typename std::enable_if<std::is_same<CharT,char>::value,double>::type
    operator()(const CharT* s, size_t) const
    {
        CharT *end = nullptr;
        double val = _strtod_l(s, &end, locale_);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }

    template <class CharT>
    typename std::enable_if<std::is_same<CharT,wchar_t>::value,double>::type
    operator()(const CharT* s, size_t) const
    {
        CharT *end = nullptr;
        double val = _wcstod_l(s, &end, locale_);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }
private:
    // noncopyable and nonmoveable
    string_to_double(const string_to_double&) = delete;
    string_to_double& operator=(const string_to_double&) = delete;
};

#elif defined(JSONCONS_HAS_STRTOLD_L)

class string_to_double
{
private:
    locale_t locale_;
public:
    string_to_double()
    {
        locale_ = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
    }
    ~string_to_double()
    {
        freelocale(locale_);
    }

    char get_decimal_point() const
    {
        return '.';
    }

    template <class CharT>
    typename std::enable_if<std::is_same<CharT,char>::value,double>::type
    operator()(const CharT* s, size_t length) const
    {
        char *end = nullptr;
        double val = strtold_l(s, &end, locale_);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }

    template <class CharT>
    typename std::enable_if<std::is_same<CharT,wchar_t>::value,double>::type
    operator()(const CharT* s, size_t length) const
    {
        CharT *end = nullptr;
        double val = wcstold_l(s, &end, locale_);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }

private:
    // noncopyable and nonmoveable
    string_to_double(const string_to_double& fr) = delete;
    string_to_double& operator=(const string_to_double& fr) = delete;
};

#else
class string_to_double
{
private:
    std::vector<char> buffer_;
    char decimal_point_;
public:
    string_to_double()
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

    char get_decimal_point() const
    {
        return decimal_point_;
    }

    template <class CharT>
    typename std::enable_if<std::is_same<CharT,char>::value,double>::type
    operator()(const CharT* s, size_t /*length*/) const
    {
        CharT *end = nullptr;
        double val = strtod(s, &end);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }

    template <class CharT>
    typename std::enable_if<std::is_same<CharT,wchar_t>::value,double>::type
    operator()(const CharT* s, size_t /*length*/) const
    {
        CharT *end = nullptr;
        double val = wcstod(s, &end);
        if (s == end)
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Convert string to double failed"));
        }
        return val;
    }

private:
    // noncopyable and nonmoveable
    string_to_double(const string_to_double& fr) = delete;
    string_to_double& operator=(const string_to_double& fr) = delete;
};
#endif

}}

#endif
