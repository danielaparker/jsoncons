/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CSV_CSV_ERROR_CATEGORY_HPP
#define JSONCONS_CSV_CSV_ERROR_CATEGORY_HPP

#include <system_error>
#include <jsoncons/json_exception.hpp>

namespace jsoncons { namespace csv {

    enum class csv_parser_errc : int
    {
        ok = 0,
        unexpected_eof = 1,
        expected_quote = 2,
        invalid_csv_text = 3,
        invalid_state = 4
    };

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
        switch (static_cast<csv_parser_errc>(ev))
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

inline 
std::error_code make_error_code(csv_parser_errc result)
{
    return std::error_code(static_cast<int>(result),csv_error_category());
}

}}

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::csv::csv_parser_errc> : public true_type
    {
    };
}

#endif
