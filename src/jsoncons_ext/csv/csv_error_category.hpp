/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CSV_CSV_TEXT_ERROR_CATEGORY_HPP
#define JSONCONS_CSV_CSV_TEXT_ERROR_CATEGORY_HPP

#include "jsoncons/jsoncons.hpp"
#include <system_error>

namespace jsoncons { namespace csv {

namespace csv_parser_errc 
{
    const int unexpected_eof = 1;
    const int expected_quote = 2;
    const int invalid_csv_text = 3;
    const int invalid_state = 4;
}

class csv_error_category_impl
   : public std::error_category
{
public:
    virtual const char* name() const JSONCONS_NOEXCEPT
    {
        return "csv";
    }
    virtual std::string message(int ev) const
    {
        switch (ev)
        {
        case csv_parser_errc::unexpected_eof:
            return "Unexpected end of file";
        case csv_parser_errc::expected_quote:
            return "Expected quote character";
        case csv_parser_errc::invalid_csv_text:
            return "Invalid CSV text";
        default:
            return "Unknown JSON parser error";
        }
    }
};

inline
const std::error_category& csv_error_category()
{
  static csv_error_category_impl instance;
  return instance;
}

}}
#endif
