/// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UBJSON_UBJSON_ERROR_CATEGORY_HPP
#define JSONCONS_UBJSON_UBJSON_ERROR_CATEGORY_HPP

#include <system_error>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons { namespace ubjson {

enum class ubjson_parse_errc
{
    ok = 0,
    unexpected_eof = 1,
    source_error
};

class ubjson_error_category_impl
   : public std::error_category
{
public:
    virtual const char* name() const noexcept
    {
        return "ubjson";
    }
    virtual std::string message(int ev) const
    {
        switch (static_cast<ubjson_parse_errc>(ev))
        {
        case ubjson_parse_errc::unexpected_eof:
            return "Unexpected end of file";
        case ubjson_parse_errc::source_error:
            return "Source error";
       default:
            return "Unknown CBOR parser error";
        }
    }
};

inline
const std::error_category& ubjson_error_category()
{
  static ubjson_error_category_impl instance;
  return instance;
}

inline 
std::error_code make_error_code(ubjson_parse_errc result)
{
    return std::error_code(static_cast<int>(result),ubjson_error_category());
}


}}

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::ubjson::ubjson_parse_errc> : public true_type
    {
    };
}

#endif
