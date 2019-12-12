/// Copyright 2019 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CONVERSION_ERROR_HPP
#define JSONCONS_CONVERSION_ERROR_HPP

#include <system_error>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons {

    enum class conversion_errc
    {
        success = 0,
        json_not_vector,
        json_not_array,
        json_not_map
    };
}

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::conversion_errc> : public true_type
    {
    };
}

namespace jsoncons {

namespace detail {
    class conversion_error_category_impl
       : public std::error_category
    {
    public:
        const char* name() const noexcept override
        {
            return "jsoncons/conversion";
        }
        std::string message(int ev) const override
        {
            switch (static_cast<conversion_errc>(ev))
            {
                case conversion_errc::json_not_vector:
                    return "json cannot be converted to vector";
                case conversion_errc::json_not_array:
                    return "json cannot be converted to array";
                case conversion_errc::json_not_map:
                    return "json cannot be converted to map";
                default:
                    return "Unknown conversion error";
            }
        }
    };
} // detail

extern inline
const std::error_category& conversion_error_category()
{
  static detail::conversion_error_category_impl instance;
  return instance;
}

inline 
std::error_code make_error_code(conversion_errc result)
{
    return std::error_code(static_cast<int>(result),conversion_error_category());
}

}

#endif
