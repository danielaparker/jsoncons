### jsoncons::json_serializing_options

```c++
typedef basic_json_serializing_options<char> json_serializing_options
```
Specifies options for serializing and deserializing JSON text. The `json_serializing_options` class is an instantiation of the `basic_json_serializing_options` class template that uses `char` as the character type.

The default floating point formatting produces digits in decimal format if possible, if not, it produces digits in exponential format. Trailing zeros are removed, except the one immediately following the decimal point. The period character (‘.’) is always used as the decimal point, non English locales are ignored.  A `precision` gives the maximum number of significant digits, the default precision is `15`. On most modern machines, 17 digits is usually enough to capture a floating-point number's value exactly, however, if you change precision to 17, conversion to text becomes an issue for floating point numbers that do not have an exact representation, e.g. 1.1 read may become 1.1000000000000001 when written. 

When parsing text, the precision of the fractional number is retained, and used for subsequent serialization, to allow round-trip.

#### Header
```c++
#include <jsoncons/json_serializing_options.hpp>
```
#### Member constants

    default_indent
The default indent is 4

#### Constructors

    json_serializing_options()
Constructs an `json_serializing_options` with default values. 

#### Accessors

    int indent() const
Returns the level of indenting, the default is 4

    uint8_t precision() const 
If set, returns an override for the number of significant digits, otherwise zero.

    bool escape_all_non_ascii() const
The default is false

    bool escape_solidus() const
The default is false

    std::string nan_replacement() const 
The default is "null"

    std::string pos_inf_replacement() const 
The default is "null"

    std::string neg_inf_replacement() const 
The default is "null"

#### Modifiers

    json_serializing_options& indent(int value)

    json_serializing_options& escape_all_non_ascii(bool value)

    json_serializing_options& escape_solidus(bool value)

    json_serializing_options& nan_replacement(const std::string& replacement)

    json_serializing_options& pos_inf_replacement(const std::string& replacement)

    json_serializing_options& neg_inf_replacement(const std::string& replacement)
Sets replacement text for negative infinity.

    json_serializing_options& precision(uint8_t prec)
Overrides floating point precision

    json_serializing_options& object_object_split_lines(line_split_kind value)
For an object whose parent is an object, set whether that object is split on a new line, or if its members are split on multiple lines. The default is [line_split_kind::multi_line](line_split_kind.md).

    json_serializing_options& array_object_split_lines(line_split_kind value)
For an object whose parent is an array, set whether that object is split on a new line, or if its members are split on multiple lines. The default is [line_split_kind::multi_line](line_split_kind.md).

    json_serializing_options& object_array_split_lines(line_split_kind value)
For an array whose parent is an object, set whether that array is split on a new line, or if its elements are split on multiple lines. The default is [line_split_kind::same_line](line_split_kind).

    json_serializing_options& array_array_split_lines(line_split_kind value)
For an array whose parent is an array, set whether that array is split on a new line, or if its elements are split on multiple lines. The default is [line_split_kind::new_line](line_split_kind).

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

