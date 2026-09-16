/// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BIGNUM_COMMON_HPP
#define JSONCONS_BIGNUM_COMMON_HPP

#include <string> 
#include <system_error>
#include <type_traits>
#include <limits>

#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons { 

template <typename T>
typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value,bool>::type
add_overflow(T a, T b) 
{
    if (a > 0 && b > 0 && a > (std::numeric_limits<T>::max)() - b) 
    {
        return true; // Positive overflow
    }
    if (a < 0 && b < 0 && a < (std::numeric_limits<T>::min)() - b) 
    {
        return true; // Negative overflow
    }
    return false; // ok
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value,bool>::type
subtract_overflow(T a, T b) 
{
    if (b < 0 && a > (std::numeric_limits<T>::max)() + b) return true;
    if (b > 0 && a < (std::numeric_limits<T>::min)() + b) return true;
    return false;
}

enum class bignum_errc
{
    success = 0,
    invalid_argument,
    result_out_of_range
};

class bignum_error_category_impl
   : public std::error_category
{
public:
    const char* name() const noexcept override
    {
        return "jsoncons/bignum_error";
    }
    std::string message(int ev) const override
    {
        switch (static_cast<bignum_errc>(ev))
        {
            case bignum_errc::success:
                return "Success";
            case bignum_errc::invalid_argument:
                return "Invalid argument";
            case bignum_errc::result_out_of_range:
                return "Result out of range";
            default:
                return "Unknown read error";
        }
    }
};

inline
const std::error_category& bignum_error_category() noexcept
{
  static bignum_error_category_impl instance;
  return instance;
}

inline 
std::error_code make_error_code(bignum_errc e) noexcept
{
    return std::error_code(static_cast<int>(e),bignum_error_category());
}

template <typename CharT>
struct to_bignum_result
{
    const CharT* ptr;
    bignum_errc ec;
    constexpr to_bignum_result(const CharT* ptr_)
        : ptr(ptr_), ec(bignum_errc{})
    {
    }
    constexpr to_bignum_result(const CharT* ptr_, bignum_errc ec_)
        : ptr(ptr_), ec(ec_)
    {
    }

    to_bignum_result(const to_bignum_result&) = default;

    to_bignum_result& operator=(const to_bignum_result&) = default;

    constexpr explicit operator bool() const noexcept
    {
        return ec == bignum_errc{};
    }
    std::error_code error_code() const
    {
        return make_error_code(ec);
    }
};

struct bignum_result
{
    bignum_errc ec;
    constexpr bignum_result()
        : ec(bignum_errc{})
    {
    }
    constexpr bignum_result(bignum_errc ec_)
        : ec(ec_)
    {
    }

    bignum_result(const bignum_result&) = default;

    bignum_result& operator=(const bignum_result&) = default;

    constexpr explicit operator bool() const noexcept
    {
        return ec == bignum_errc{};
    }
    std::error_code error_code() const
    {
        return make_error_code(ec);
    }
};

} // namespace jsoncons

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::bignum_errc> : public true_type
    {
    };
} // namespace std

#endif // JSONCONS_BIGNUM_COMMON_HPP
