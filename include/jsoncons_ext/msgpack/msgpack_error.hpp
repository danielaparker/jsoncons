/// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_MSGPACK_MSGPACK_ERROR_HPP
#define JSONCONS_MSGPACK_MSGPACK_ERROR_HPP

#include <system_error>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons { namespace msgpack {

enum class msgpack_errc
{
    ok = 0,
    unexpected_eof = 1,
    source_error,
    invalid_utf8_text_string
};

class msgpack_error_category_impl
   : public std::error_category
{
public:
    virtual const char* name() const noexcept
    {
        return "msgpack";
    }
    virtual std::string message(int ev) const
    {
        switch (static_cast<msgpack_errc>(ev))
        {
            case msgpack_errc::unexpected_eof:
                return "Unexpected end of file";
            case msgpack_errc::source_error:
                return "Source error";
            case msgpack_errc::invalid_utf8_text_string:
                return "Illegal UTF-8 encoding in text string";
            default:
                return "Unknown MessagePack parser error";
        }
    }
};

inline
const std::error_category& msgpack_error_category()
{
  static msgpack_error_category_impl instance;
  return instance;
}

inline 
std::error_code make_error_code(msgpack_errc e)
{
    return std::error_code(static_cast<int>(e),msgpack_error_category());
}


}}

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::msgpack::msgpack_errc> : public true_type
    {
    };
}

#endif
