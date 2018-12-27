### jsoncons::json_options

```c++
typedef basic_json_options<char> json_options
```

Specifies options for serializing and deserializing JSON text. The `json_options` class is an instantiation of the `basic_json_options` class template that uses `char` as the character type.

The default floating point formatting for a floating point value that was previously decoded from json text is to preserve the original format and precision. This ensures round-trip for both format and precision, e.g. 1.1 read will remain `1.1` when written, and not become `1.1000000000000001` (an equivalent but longer representation.)

The default floating point formatting for a floating point value that was directly inserted into a json value is [chars_format::general](chars_format.md) with shortest representation. Trailing zeros are removed, except one immediately following the decimal point. The period character (‘.’) is always used as the decimal point, non English locales are ignored.

#### Header
```c++
#include <jsoncons/json_options.hpp>
```

#### Abstract base classes

[json_read_options](json_read_options.md)
[json_write_options](json_write_options.md)

#### Member constants

    static const size_t indent_size_default = 4;
The default size indent is 4

    static const size_t line_length_limit_default = 120;
The default line length limit is 120

#### Constructors

    json_options()
Constructs an `json_options` with default values. 

#### Accessors and modifiers

    size_t  indent_size() const
    json_options& indent(size_ value)
The indent size, the default is 4.

    spaces_option spaces_around_colon() const override
    json_options & spaces_around_colon(spaces_option value)
Indicates [space option](spaces_option.md) for name separator (`:`). Default
is space after.

    spaces_option spaces_around_comma() const override
    json_options & spaces_around_comma(spaces_option value)
Indicates [space option](spaces_option.md) for array value and object name/value pair separators (`,`). Default
is space after.

    bool pad_inside_object_braces() const override
    json_options & pad_inside_object_braces(bool value)
Default is `false`

    bool pad_inside_array_brackets() const override
    json_options & pad_inside_array_brackets(bool value)
Default is `false`

    chars_format floating_point_format() const 
    json_options& floating_point_format(chars_format value)
Overrides [floating point format](chars_format.md) when serializing json.
The default, for a floating point value that was previously decoded from json text, is to preserve the original format when serializing.
The default, for a floating point value that was directly inserted into a json value, to serialize with [chars_format::general](chars_format.md). 

    bignum_chars_format bignum_format() const 
    json_options& bignum_format(bignum_chars_format value)
Overrides [bignum format](bignum_chars_format.md) when serializing json.
The default is [bignum_chars_format::base10](bignum_chars_format.md). 

    byte_string_chars_format byte_string_format() const 
    json_options& byte_string_format(byte_string_chars_format value)
Overrides [byte string format](byte_string_chars_format.md) when serializing json.
The default is [byte_string_chars_format::base64url](byte_string_chars_format.md). 

    uint8_t precision() const 
    json_options& precision(uint8_t value)
Overrides floating point precision when serializing json. 
The default, for a floating point value that was previously decoded from json text, is to preserve the original precision. 
The fefault, for a floating point value that was directly inserted into a json value, to serialize with shortest representation. 

    bool escape_all_non_ascii() const
    json_options& escape_all_non_ascii(bool value)
Escape all non-ascii characters. The default is `false`.

    bool escape_solidus() const
    json_options& escape_solidus(bool value)
Escape the solidus ('/') character. The default is `false`.

    bool is_nan_to_num() const override; // (1)
    const std::string& nan_to_num() const; // (2) 
    json_options& nan_to_num(const std::string& value); // (3)
(1) indicates number replacement for `NaN` when serializing. Defaults to `false`.
(2) returns a number replacement for `NaN`
(3) sets a number replacement for `NaN`

    bool is_inf_to_num() const override; // (1)
    const std::string& inf_to_num() const; // (2) 
    json_options& inf_to_num(const std::string& value); // (3)
(1) indicates a number replacement for `Infinity` when serializing. Defaults to `false`.
(2) returns a number replacement for `Infinity`
(3) sets a number replacement for `Infinity`

    bool is_neginf_to_num() const override; // (1)
    const std::string& neginf_to_num() const; // (2) 
    json_options& neginf_to_num(const std::string& value); // (3)
(1) indicates a number replacement for `Negative Infinity` when serializing. Defaults to `is_inf_to_num()`.
(2) returns a number replacement for `Negative Infinity`
(3) sets a number replacement for `Negative Infinity`

    bool is_nan_to_str() const override; // (1)
    bool is_str_to_nan() const override; // (2)
    const std::string& nan_to_str() const; //(3)
    json_options& nan_to_str(const std::string& value, bool is_str_to_nan = true); // (4)
(1) indicates string replacement for `NaN` when serializing. Defaults to `false`.
(2) indicates `NaN` replacement for string when parsing. Defaults to `false`.
(3) returns a string replacement for `NaN`
(4) sets a string replacement for `NaN`

    bool is_inf_to_str() const override; // (1)
    bool is_str_to_inf() const override; // (2)
    const std::string& inf_to_str() const; //(3)
    json_options& inf_to_str(const std::string& value, bool is_str_to_inf = true); // (4)
(1) indicates string replacement for `Infinity` when serializing. Defaults to `false`.
(2) indicates `Infinity` replacement for string when parsing. Defaults to `false`.
(3) returns a string replacement for `Infinity`
(4) sets a string replacement for `Infinity`

    bool is_neginf_to_str() const override; // (1)
    bool is_str_to_neginf() const override; // (2)
    const std::string& neginf_to_str() const; //(3)
    json_options& neginf_to_str(const std::string& value, bool is_str_to_neginf = true); // (4)
(1) indicates string replacement for `Negative Infinity` when serializing. Defaults to `is_inf_to_str()`.
(2) indicates `Negative Infinity` replacement for string when parsing. Defaults to `is_str_to_inf()`.
(3) returns a string replacement for `Negative Infinity` if one has been set, otherwise `-` followed by
`inf_to_num()`.
(4) sets a string replacement for `Negative Infinity`

    std::string new_line_chars() const
    serializing_options& new_line_chars(const std::string& value)
Defaults to "\n"

    size_t line_length_limit() const
    json_options & line_length_limit(size_t value)

    size_t max_nesting_depth() const
    void max_nesting_depth(size_t depth)
The maximum nesting depth allowed when parsing JSON. By default `jsoncons` can read a `JSON` text of arbitrarily large depth.

    line_split_kind object_object_line_splits() const;
    json_options& object_object_line_splits(line_split_kind value)
For an object whose parent is an object, set whether that object is split on a new line, or if its members are split on multiple lines. The default is [line_split_kind::multi_line](line_split_kind.md).

    line_split_kind array_object_line_splits() const;
    json_options& array_object_line_splits(line_split_kind value)
For an object whose parent is an array, set whether that object is split on a new line, or if its members are split on multiple lines. The default is [line_split_kind::multi_line](line_split_kind.md).

    line_split_kind object_array_line_splits() const;
    json_options& object_array_line_splits(line_split_kind value)
For an array whose parent is an object, set whether that array is split on a new line, or if its elements are split on multiple lines. The default is [line_split_kind::same_line](line_split_kind.md).

    line_split_kind array_array_line_splits() const;
    json_options& array_array_line_splits(line_split_kind value)
For an array whose parent is an array, set whether that array is split on a new line, or if its elements are split on multiple lines. The default is [line_split_kind::new_line](line_split_kind.md).

### Examples

#### Default NaN, inf and -inf replacement
```c++
json obj;
obj["field1"] = std::sqrt(-1.0);
obj["field2"] = 1.79e308*1000;
obj["field3"] = -1.79e308*1000;
std::cout << obj << std::endl;
```
Output:
```json
{"field1":null,"field2":null,"field3":null}
```
#### User specified `Nan` and `Inf` replacement

```c++
json obj;
obj["field1"] = std::sqrt(-1.0);
obj["field2"] = 1.79e308*1000;
obj["field3"] = -1.79e308*1000;

json_options options;
format.nan_to_num("null");        // default is "null"
format.inf_to_num("1e9999");  // default is "null"

std::cout << pretty_print(obj,options) << std::endl;
```

Output:
```json
    {
        "field1":null,
        "field2":1e9999,
        "field3":-1e9999
    }
```

#### Object-array block formatting

```c++
    json val;

    val["verts"] = json::array{1, 2, 3};
    val["normals"] = json::array{1, 0, 1};
    val["uvs"] = json::array{0, 0, 1, 1};

    std::cout << "Default (same line)" << std::endl;
    std::cout << pretty_print(val) << std::endl;

    std::cout << "New line" << std::endl;
    json_options options1;
    format1.object_array_line_splits(line_split_kind::new_line);
    std::cout << pretty_print(val,options1) << std::endl;

    std::cout << "Multi line" << std::endl;
    json_options options2;
    format2.object_array_line_splits(line_split_kind::multi_line);
    std::cout << pretty_print(val,options2) << std::endl;
```

Output:

Default (same line)

```json
{
    "normals": [1,0,1],
    "uvs": [0,0,1,1],
    "verts": [1,2,3]
}
```

New line

```json
{
    "normals": [
        1,0,1
    ],
    "uvs": [
        0,0,1,1
    ],
    "verts": [
        1,2,3
    ]
}
```
Multi line
```json
{
    "normals": [
        1,
        0,
        1
    ],
    "uvs": [
        0,
        0,
        1,
        1
    ],
    "verts": [
        1,
        2,
        3
    ]
}
```

#### Array-array block formatting

```c++
    json val;
    val["data"]["id"] = json::array{0,1,2,3,4,5,6,7};
    val["data"]["item"] = json::array{json::array{2},
                                      json::array{4,5,2,3},
                                      json::array{4},
                                      json::array{4,5,2,3},
                                      json::array{2},
                                      json::array{4,5,3},
                                      json::array{2},
                                      json::array{4,3}};

    std::cout << "Default (new line)" << std::endl;
    std::cout << pretty_print(val) << std::endl;

    std::cout << "Same line" << std::endl;
    json_options options1;
    format1.array_array_line_splits(line_split_kind::same_line);
    std::cout << pretty_print(val, options1) << std::endl;

    std::cout << "Multi line" << std::endl;
    json_options options2;
    format2.array_array_line_splits(line_split_kind::multi_line);
    std::cout << pretty_print(val, options2) << std::endl;
```

Output:

Default (new line)

```json
{
    "data": {
        "id": [0,1,2,3,4,5,6,7],
        "item": [
            [2],
            [4,5,2,3],
            [4],
            [4,5,2,3],
            [2],
            [4,5,3],
            [2],
            [4,3]
        ]
    }
}
```
Same line

```json
{
    "data": {
        "id": [0,1,2,3,4,5,6,7],
        "item": [[2],[4,5,2,3],[4],[4,5,2,3],[2],[4,5,3],[2],[4,3]]
    }
}
```

Multi line

```json
{
    "data": {
        "id": [
            0,1,2,3,4,5,6,7
        ],
        "item": [
            [
                2
            ],
            [
                4,
                5,
                2,
                3
            ],
            [
                4
            ],
            [
                4,
                5,
                2,
                3
            ],
            [
                2
            ],
            [
                4,
                5,
                3
            ],
            [
                2
            ],
            [
                4,
                3
            ]
        ]
    }
}
```

