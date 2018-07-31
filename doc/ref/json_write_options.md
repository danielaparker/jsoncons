### jsoncons::json_write_options

```c++
typedef basic_json_write_options<char> json_write_options
```

An abstract class that defines accessors for JSON serialization options. The `json_write_options` class is an instantiation of the `basic_json_write_options` class template that uses `char` as the character type.

#### Header
```c++
#include <jsoncons/json_write_options.hpp>
```

#### Implementing classes

[json_serializing_options](json_serializing_options)

#### Destructor

    virtual ~json_write_options();

#### Accessors

    int indent() const
The level of indenting, the default is 4.

    virtual chars_format floating_point_format() const = 0;
Overrides [floating point format](chars_format.md) when serializing json.
The default, for a floating point value that was previously decoded from json text, is to preserve the original format when serializing.
The default, for a floating point value that was directly inserted into a json value, to serialize with [chars_format::general](chars_format.md). 

    virtual bignum_chars_format bignum_format() const = 0; 
Overrides [bignum format](bignum_chars_format.md) when serializing json.
The default is [bignum_chars_format::base10](bignum_chars_format.md). 

    virtual uint8_t precision() const = 0; 
Overrides floating point precision when serializing json. 
The default, for a floating point value that was previously decoded from json text, is to preserve the original precision. 
The fefault, for a floating point value that was directly inserted into a json value, to serialize with shortest representation. 

    virtual bool escape_all_non_ascii() const = 0;
Escape all non-ascii characters. The default is `false`.

    virtual bool escape_solidus() const = 0;
Escape the solidus ('/') character. The default is `false`.

    virtual bool can_write_nan_replacement() const = 0;

    virtual bool can_write_pos_inf_replacement() const = 0;

    virtual bool can_write_neg_inf_replacement() const = 0;

    virtual const std::string& nan_replacement() const = 0; 
NaN replacement. The default is `"null"`. 

    virtual const std::string& pos_inf_replacement() const = 0; 
Positive infinity replacement. The default is `"null"`

    virtual const std::string& neg_inf_replacement() const = 0; 
Negative infinity replacement. The default is `"null"`

    virtual json_write_options& object_object_split_lines(line_split_kind value) = 0;
For an object whose parent is an object, set whether that object is split on a new line, or if its members are split on multiple lines. The default is [line_split_kind::multi_line](line_split_kind.md).

    virtual json_write_options& array_object_split_lines(line_split_kind value) = 0;
For an object whose parent is an array, set whether that object is split on a new line, or if its members are split on multiple lines. The default is [line_split_kind::multi_line](line_split_kind.md).

    virtual json_write_options& object_array_split_lines(line_split_kind value) = 0;
For an array whose parent is an object, set whether that array is split on a new line, or if its elements are split on multiple lines. The default is [line_split_kind::same_line](line_split_kind).

    virtual json_write_options& array_array_split_lines(line_split_kind value) = 0;
For an array whose parent is an array, set whether that array is split on a new line, or if its elements are split on multiple lines. The default is [line_split_kind::new_line](line_split_kind).


