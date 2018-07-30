/// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBORERRORCATEGORY_HPP
#define JSONCONS_CBOR_CBORERRORCATEGORY_HPP

#include <system_error>
#include <jsoncons/jsoncons_config.hpp>

namespace jsoncons { namespace cbor {

    enum class cbor_parse_errc
    {
        ok = 0,
        unexpected_eof = 1,
        source_error
    };

#if !defined(JSONCONS_NO_DEPRECATED)
typedef cbor_parse_errc cbor_parser_errc;
#endif

class cbor_error_category_impl
   : public std::error_category
{
public:
    virtual const char* name() const JSONCONS_NOEXCEPT
    {
        return "cbor";
    }
    virtual std::string message(int ev) const
    {
        switch (static_cast<cbor_parse_errc>(ev))
        {
        case cbor_parse_errc::unexpected_eof:
            return "Unexpected end of file";
        case cbor_parse_errc::source_error:
            return "Source error";
       default:
            return "Unknown CBOR parser error";
        }
    }
};

inline
const std::error_category& cbor_error_category()
{
  static cbor_error_category_impl instance;
  return instance;
}

inline 
std::error_code make_error_code(cbor_parse_errc result)
{
    return std::error_code(static_cast<int>(result),cbor_error_category());
}


}}

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::cbor::cbor_parse_errc> : public true_type
    {
    };
}

#endif
