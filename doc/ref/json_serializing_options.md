### jsoncons::json_serializing_options

```c++
typedef basic_json_serializing_options<char> json_serializing_options
```

Specifies options for serializing and deserializing JSON text. The `json_serializing_options` class is an instantiation of the `basic_json_serializing_options` class template that uses `char` as the character type.

The default floating point formatting for a floating point value that was previously decoded from json text is to preserve the original format and precision. This ensures round-trip for both format and precision, e.g. 1.1 read will remain `1.1` when written, and not become `1.1000000000000001` (an equivalent but longer representation.)

The default floating point formatting for a floating point value that was directly inserted into a json value is [chars_format::general](chars_format.md) with shortest representation. Trailing zeros are removed, except one immediately following the decimal point. The period character (‘.’) is always used as the decimal point, non English locales are ignored.

#### Header
```c++
#include <jsoncons/json_serializing_options.hpp>
```

#### Abstract base classes

[json_read_options](json_read_options)
[json_write_options](json_write_options)

#### Member constants

    default_indent
The default indent is 4

#### Constructors

    json_serializing_options()
Constructs an `json_serializing_options` with default values. 

#### Accessors and modifiers

    int indent() const
    json_serializing_options& indent(int value)
The level of indenting, the default is 4.

    chars_format floating_point_format() const 
    json_serializing_options& floating_point_format(chars_format value)
Overrides [floating point format](chars_format.md) when serializing json.
The default, for a floating point value that was previously decoded from json text, is to preserve the original format when serializing.
The default, for a floating point value that was directly inserted into a json value, to serialize with [chars_format::general](chars_format.md). 

    bignum_chars_format bignum_format() const 
    json_serializing_options& bignum_format(bignum_chars_format value)
Overrides [bignum format](bignum_chars_format.md) when serializing json.
The default is [bignum_chars_format::base10](bignum_chars_format.md). 

    uint8_t precision() const 
    json_serializing_options& precision(uint8_t value)
Overrides floating point precision when serializing json. 
The default, for a floating point value that was previously decoded from json text, is to preserve the original precision. 
The fefault, for a floating point value that was directly inserted into a json value, to serialize with shortest representation. 

    bool escape_all_non_ascii() const
    json_serializing_options& escape_all_non_ascii(bool value)
Escape all non-ascii characters. The default is `false`.

    bool escape_solidus() const
    json_serializing_options& escape_solidus(bool value)
Escape the solidus ('/') character. The default is `false`.

    const std::string& nan_replacement() const 
    json_serializing_options& nan_replacement(const string_view_type& replacement)
NaN replacement. The default is `"null"`. 

    const std::string& pos_inf_replacement() const 
    json_serializing_options& pos_inf_replacement(const string_view_type& replacement)
Positive infinity replacement. The default is `"null"`

    const std::string& neg_inf_replacement() const 
    json_serializing_options& neg_inf_replacement(const string_view_type& replacement)
Negative infinity replacement. The default is `"null"`

    size_t max_nesting_depth() const
    void max_nesting_depth(size_t depth)
The maximum nesting depth allowed when deserializing. By default `jsoncons` can read a `JSON` text of arbitrarily large depth.

    json_serializing_options& object_object_split_lines(line_split_kind value)
For an object whose parent is an object, set whether that object is split on a new line, or if its members are split on multiple lines. The default is [line_split_kind::multi_line](line_split_kind.md).

    json_serializing_options& array_object_split_lines(line_split_kind value)
For an object whose parent is an array, set whether that object is split on a new line, or if its members are split on multiple lines. The default is [line_split_kind::multi_line](line_split_kind.md).

    json_serializing_options& object_array_split_lines(line_split_kind value)
For an array whose parent is an object, set whether that array is split on a new line, or if its elements are split on multiple lines. The default is [line_split_kind::same_line](line_split_kind).

    json_serializing_options& array_array_split_lines(line_split_kind value)
For an array whose parent is an array, set whether that array is split on a new line, or if its elements are split on multiple lines. The default is [line_split_kind::new_line](line_split_kind).

    size_t max_nesting_depth()
    json_serializing_options& max_nesting_depth(size_t value)
 Maximum nesting depth when parsing JSON.


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
#### User specified `Nan`, `Inf` and `-Inf` replacement

```c++
json obj;
obj["field1"] = std::sqrt(-1.0);
obj["field2"] = 1.79e308*1000;
obj["field3"] = -1.79e308*1000;

json_serializing_options options;
format.nan_replacement("null");        // default is "null"
format.pos_inf_replacement("1e9999");  // default is "null"
format.neg_inf_replacement("-1e9999"); // default is "null"

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
    json_serializing_options options1;
    format1.object_array_split_lines(line_split_kind::new_line);
    std::cout << pretty_print(val,options1) << std::endl;

    std::cout << "Multi line" << std::endl;
    json_serializing_options options2;
    format2.object_array_split_lines(line_split_kind::multi_line);
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
    json_serializing_options options1;
    format1.array_array_split_lines(line_split_kind::same_line);
    std::cout << pretty_print(val, options1) << std::endl;

    std::cout << "Multi line" << std::endl;
    json_serializing_options options2;
    format2.array_array_split_lines(line_split_kind::multi_line);
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

