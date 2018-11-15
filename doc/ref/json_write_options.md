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

[json_serializing_options](json_serializing_options.md)

#### Destructor

    virtual ~json_write_options();

#### Accessors


    size_t  indent_size() const
The indent size, the default is 4.

    spaces_option spaces_around_colon() const override
Indicates [space option](spaces_option.md) for name separator (`:`). Default
is space after.

    spaces_option spaces_around_comma() const override
Indicates [space option](spaces_option.md) for array value and object name/value pair separators (`,`). Default
is space after.

    bool pad_inside_object_braces() const override
Default is `false`

    bool pad_inside_array_brackets() const override
Default is `false`

    chars_format floating_point_format() const 
Overrides [floating point format](chars_format.md) when serializing json.
The default, for a floating point value that was previously decoded from json text, is to preserve the original format when serializing.
The default, for a floating point value that was directly inserted into a json value, to serialize with [chars_format::general](chars_format.md). 

    bignum_chars_format bignum_format() const 
Overrides [bignum format](bignum_chars_format.md) when serializing json.
The default is [bignum_chars_format::base10](bignum_chars_format.md). 

    byte_string_chars_format byte_string_format() const 
Overrides [byte string format](byte_string_chars_format.md) when serializing json.
The default is [byte_string_chars_format::base64url](byte_string_chars_format.md). 

    uint8_t precision() const 
Overrides floating point precision when serializing json. 
The default, for a floating point value that was previously decoded from json text, is to preserve the original precision. 
The fefault, for a floating point value that was directly inserted into a json value, to serialize with shortest representation. 

    bool escape_all_non_ascii() const
Escape all non-ascii characters. The default is `false`.

    bool escape_solidus() const
Escape the solidus ('/') character. The default is `false`.

    const std::string& nan_to_num() const 
    const std::string& nan_to_str() const 
Replace `NaN` with a number (if `nan_to_num()` returns a non-empty string)
or a string (if `nan_to_str()` returns a non-empty string.) If both
are empty, replace `NaN` with `null`.

    const std::string& inf_to_num() const 
    const std::string& inf_to_str() const 
Replace positive infinity with a number (if `inf_to_num()` returns a non-empty string)
or a string (if `inf_to_str()` returns a non-empty string.) If both
are empty, replace positive infinity with `null`.

    const std::string& neginf_to_num() const 
    const std::string& neginf_to_str() const 
Replace negative infinity with a number (if `neginf_to_num()` returns a non-empty string)
or a string (if `neginf_to_str()` returns a non-empty string.) If both
are empty, replace negative infinity with `null`.

    std::string new_line_chars() const
Defaults to "\n"

    size_t line_length_limit() const

    line_split_kind object_object_line_splits() const;
For an object whose parent is an object, indicates whether that object is split on a new line, or if its members are split on multiple lines. The default is [line_split_kind::multi_line](line_split_kind.md).

    line_split_kind array_object_line_splits() const;
For an object whose parent is an array, indicates whether that object is split on a new line, or if its members are split on multiple lines. The default is [line_split_kind::multi_line](line_split_kind.md).

    line_split_kind object_array_line_splits() const;
For an array whose parent is an object, indicates whether that array is split on a new line, or if its elements are split on multiple lines. The default is [line_split_kind::same_line](line_split_kind.md).

    line_split_kind array_array_line_splits() const;
For an array whose parent is an array, indicates whether that array is split on a new line, or if its elements are split on multiple lines. The default is [line_split_kind::new_line](line_split_kind.md).

