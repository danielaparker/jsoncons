### jsoncons::csv::basic_csv_decode_options

```c++
#include <jsoncons/csv/csv_options.hpp>

template< 
    class CharT
> class basic_csv_decode_options;
```

An abstract class that defines accessors for CSV parsing options.

#### Implementing classes

[basic_csv_options](basic_csv_options.md)

Typedefs for common character types are provided:

Type                |Definition
--------------------|------------------------------
`csv_decode_options`        |`basic_csv_decode_options<char>`
`wcsv_decode_options`       |`basic_csv_decode_options<wchar_t>`

Member type                         |Definition
------------------------------------|------------------------------
`string_type`|`std::basic_string<CharT>`

#### Accessors

    virtual size_t header_lines() const = 0;

    virtual bool assume_header() const = 0;

    virtual bool ignore_empty_values() const = 0;

    virtual bool ignore_empty_lines() const = 0;

    virtual bool trim_leading() const = 0;

    virtual bool trim_trailing() const = 0;

    virtual bool trim_leading_inside_quotes() const = 0;

    virtual bool trim_trailing_inside_quotes() const = 0;

    virtual bool trim() const = 0;

    virtual bool trim_inside_quotes() const = 0;

    virtual bool unquoted_empty_value_is_null() const = 0;

    virtual std::vector<string_type> column_names() const = 0;

    virtual std::vector<csv_type_info> column_types() const = 0;

    virtual std::vector<string_type> column_defaults() const = 0;

    virtual CharT field_delimiter() const = 0;

    virtual std::pair<CharT,bool> subfield_delimiter() const = 0;

    virtual string_type line_delimiter() const = 0;

    virtual CharT quote_char() const = 0;

    virtual bool infer_types() const = 0;

    virtual bool lossless_number() const = 0;

    virtual CharT quote_escape_char() const = 0;

    virtual CharT comment_starter() const = 0;

    virtual mapping_type mapping() const = 0;

    virtual unsigned long max_lines() const = 0;

