/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_CSV_CSV_PARSER_CATEGORY_HPP
#define JSONCONS_CSV_CSV_PARSER_CATEGORY_HPP

#include "jsoncons/jsoncons.hpp"
#include <system_error>

namespace jsoncons { namespace csv {

namespace csv_parser_errc 
{
    enum csv_parser_errc_t 
    {
        unexpected_eof
    };
}

class csv_text_error_category_impl
   : public std::error_category
{
public:
    virtual const char* name() const JSONCONS_NOEXCEPT
    {
        return "csv text";
    }
    virtual std::string message(int ev) const
    {
        switch (ev)
        {
        case csv_parser_errc::unexpected_eof:
            return "Unexpected end of file";
        default:
            return "Unknown JSON parser error";
        }
    }
};

inline
const std::error_category& csv_text_error_category()
{
  static csv_text_error_category_impl instance;
  return instance;
}

}}
#endif
