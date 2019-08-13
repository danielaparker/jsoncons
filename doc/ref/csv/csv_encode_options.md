### jsoncons::csv::csv_encode_options

```c++
#include <jsoncons/csv/csv_options.hpp>
```

An abstract class that defines accessors for CSV encode options.

#### Implementing classes

[basic_csv_options](basic_csv_options.md)

#### Accessors

    virtual float_chars_format float_format() const = 0;

    virtual int precision() const = 0;

    virtual std::vector<string_type> column_names() const = 0;

    virtual CharT field_delimiter() const = 0;

    virtual std::pair<CharT,bool> subfield_delimiter() const = 0;

    virtual string_type line_delimiter() const = 0;

    virtual CharT quote_char() const = 0;

    virtual CharT quote_escape_char() const = 0;

    virtual quote_style_type quote_style() const = 0;

