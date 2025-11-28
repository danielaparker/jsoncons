### jsoncons::basic_json_options

```cpp
#include <jsoncons/json_options.hpp>

template< 
    typename CharT
> class basic_json_options;
```

<br>

![basic_json_options](./diagrams/basic_json_options.png)

Specifies options for reading and writing JSON text. 

Option|Reading|Writing|Default|comment
------|-------|-------|-------|------
nan_to_str|Substitute string with `NaN`, if enabled|Sets a string replacement for `NaN` when writing JSON|Unenabled|
inf_to_str|Substitute string with `infinity`, if enabled|Sets a string replacement for infinity when writing JSON|Unenabled|
neginf_to_str|Substitute string with `negative infinity`, if enabled|Sets a string replacement for negative infinity when writing JSON|Unenabled|
nan_to_num| |Sets a number replacement for `NaN` when writing JSON|Unenabled|
inf_to_num| |Sets a number replacement for `Infinity` when writing JSON|Unenabled|
neginf_to_num| |Sets a number replacement for `Negative Infinity` when writing JSON|Unenabled|
max_nesting_depth|Maximum nesting depth allowed when parsing JSON|Maximum nesting depth allowed when serializing JSON|**1024**|
lossless_bignum|When parsing floating point values, and value is out-of-range, produces a string with tag `semantic_tag::bigdec` if **true**, otherwise produces +- infinity.| |**true**|(since 1.4.0)</br>(until 1.5.0)
lossless_bignum|When parsing an integer value, and value is out-of-range, produces a string with tag `semantic_tag::bigint` if **true**, otherwise parses as double. When parsing floating point values, and value is out-of-range, produces a string with tag `semantic_tag::bigdec` if **true**, otherwise produces `+- infinity`.| |**true**|(since 1.5.0)
lossless_number|If **true**, reads numbers with exponents and fractional parts as strings with tag `semantic_tag::bigdec`.| |**false**|
allow_comments|If 'true', allow (and ignore) comments when parsing JSON| |**true**|(since 1.3.0)
allow_trailing_comma|If 'true', an extra comma at the end of a list of JSON values in an object or array is allowed (and ignored)| |**false**|(since 1.3.0)
err_handler|Defines an [error handler](err_handler.md) for parsing JSON.| |`default_json_parsing`|(since 0.171.0, deprecated in 1.5.0)
indent_size| |The indent size|**4**|
indent_char| |The indent character, e.g. '\t'|**' '**| (since 1.5)
spaces_around_colon| |Indicates [space option](spaces_option.md) for name separator (`:`).|space after|
spaces_around_comma| |Indicates [space option](spaces_option.md) for array value and object name/value pair separators (`,`).|space after|
pad_inside_object_braces| |Pad inside object braces|**false**|
pad_inside_array_brackets| |Pad inside array brackets|**false**|
bigint_format| |Specifies which [bigint format](bigint_chars_format.md) to use when serializing json.|`bignum_format_kind::raw`| (since 1.0.0)
bignum_format| |Specifies which [bignum format](bignum_format_kind.md) to use when serializing json. |`bignum_format_kind::raw`|
byte_string_format| |Overrides [byte string format](byte_string_chars_format.md) when serializing json. |[byte_string_chars_format::base64url](byte_string_chars_format.md)|
float_format| |Overrides [floating point format](float_chars_format.md) when serializing to JSON. |[float_chars_format::general](float_chars_format.md)|
precision| |Overrides floating point precision when serializing json.|shortest representation|
escape_all_non_ascii| |Escape all non-ascii characters. |**false**|
escape_solidus| |Escape the solidus ('/') character. |**false**|
new_line_chars| |New line characters|"\n"|
line_length_limit| |Line length limit|120|
object_object_line_splits| |For an object whose parent is an object, set whether that object is split on a new line, or if its members are split on multiple lines. |[line_split_kind::multi_line](line_split_kind.md)|
array_object_line_splits| |For an object whose parent is an array, set whether that object is split on a new line, or if its members are split on multiple lines. |[line_split_kind::multi_line](line_split_kind.md)|
object_array_line_splits| |For an array whose parent is an object, set whether that array is split on a new line, or if its elements are split on multiple lines. |[line_split_kind::multi_line](line_split_kind.md)|
array_array_line_splits| |For an array whose parent is an array, set whether that array is split on a new line, or if its elements are split on multiple lines. |[line_split_kind::multi_line](line_split_kind.md)|

The default floating point format is [float_chars_format::general](float_chars_format.md).
The default precision is shortest representation, e.g. 1.1 read will remain `1.1` when written, and not become `1.1000000000000001` (an equivalent but longer representation.)
Trailing zeros are removed, except one immediately following the decimal point. The period character (‘.’) is always used as the decimal point, non English locales are ignored.

Aliases for common character types are provided:

Type                |Definition
--------------------|------------------------------
`json_options`        |`basic_json_options<char>`
`wjson_options`       |`basic_json_options<wchar_t>`

#### Member constants

    static const uint8_t indent_size_default = 4;
The default size indent is 4

    static const size_t line_length_limit_default = 120;
The default line length limit is 120

Member type                         |Definition
------------------------------------|------------------------------
`char_type`|`CharT`
`string_type`|`std::basic_string<CharT>`

#### Constructors

    basic_json_options()
Default constructor. 

    basic_json_options(const basic_json_options& other)
Copy constructor. 

    basic_json_options(basic_json_options&& other)
Move constructor. 

#### Setters

    basic_json_options& max_nesting_depth(int depth)
    basic_json_options& decode_escaped_unicode(bool value); 
    basic_json_options& nan_to_str(const string_type& value, bool enable_inverse = true); 
    basic_json_options& inf_to_str(const string_type& value, bool enable_inverse = true); 
    basic_json_options& neginf_to_str(const string_type& value, bool enable_inverse = true); 
    basic_json_options& nan_to_num(const string_type& value); 
    basic_json_options& inf_to_num(const string_type& value); 
    basic_json_options& neginf_to_num(const string_type& value); 
    basic_json_options& lossless_number(bool value); 
    basic_json_options& allow_comments(bool value); 
    basic_json_options& allow_trailing_comma(bool value); 
    basic_json_options& indent_size(uint8_t value)
    basic_json_options& spaces_around_colon(spaces_option value)
    basic_json_options& spaces_around_comma(spaces_option value)
    basic_json_options& pad_inside_object_braces(bool value)
    basic_json_options& pad_inside_array_brackets(bool value)
    basic_json_options& bignum_format(bignum_format_kind value)
    basic_json_options& byte_string_format(byte_string_chars_format value)
    basic_json_options& float_format(float_chars_format value);
    basic_json_options& precision(int8_t value)
    basic_json_options& escape_all_non_ascii(bool value)
    basic_json_options& escape_solidus(bool value)
    basic_json_options& new_line_chars(const string_type& value)
    basic_json_options& line_length_limit(std::size_t value)
    basic_json_options& object_object_line_splits(line_split_kind value)
    basic_json_options& array_object_line_splits(line_split_kind value)
    basic_json_options& object_array_line_splits(line_split_kind value)
    basic_json_options& array_array_line_splits(line_split_kind value)

### Examples

[Default NaN and inf replacement](#E1)  
[User specified `Nan` and `Inf` replacement](#E2)  
[Decimal precision](#E3)  
[Parse integer with lossless_bignum](#E4)  
[Parse floating point with lossless_bignum](#E5)  
[Object-array block formatting](#E6)  
[Array-array block formatting](#E7)  
[Prettify single line output](#E8)  
[Indent with tabs](#E9)  
[Allow trailing commas](#E10)  

<div id="E1"/> 

#### Default NaN and inf replacement
```cpp
json obj;
obj["field1"] = std::sqrt(-1.0);
obj["field2"] = 1.79e308*1000;
obj["field3"] = -1.79e308*1000;
std::cout << obj << '\n';
```
Output:
```json
{"field1":null,"field2":null,"field3":null}
```

<div id="E2"/> 

#### User specified `Nan` and `Inf` replacement

```cpp
json obj;
obj["field1"] = std::sqrt(-1.0);
obj["field2"] = 1.79e308*1000;
obj["field3"] = -1.79e308*1000;

auto options = json_options{}
    .nan_to_num("null");        // default is "null"
    .inf_to_num("1e9999");      // default is "null"

std::cout << pretty_print(obj,options) << '\n';
```

Output:
```json
    {
        "field1":null,
        "field2":1e9999,
        "field3":-1e9999
    }
```

<div id="E3"/> 

#### Decimal precision

By default, jsoncons parses a number with an exponent or fractional part
into a double precision floating point number. If you wish, you can
keep the number as a string with semantic tagging `bigdec`, 
using the `lossless_number` option. You can then put it into a `float`, 
`double`, a boost multiprecision number, or whatever type you want. 

```cpp
int main()
{
    std::string s = R"(
    {
        "a" : 12.00,
        "b" : 1.23456789012345678901234567890
    }
    )";

    // Default
    json j = json::parse(s);

    std::cout.precision(15);

    // Access as string
    std::cout << "(1) a: " << j["a"].as<std::string>() << ", b: " << j["b"].as<std::string>() << "\n"; 
    // Access as double
    std::cout << "(2) a: " << j["a"].as<double>() << ", b: " << j["b"].as<double>() << "\n\n"; 

    // Using lossless_number option
    auto options = json_options{}
        .lossless_number(true);

    json j2 = json::parse(s, options);
    // Access as string
    std::cout << "(3) a: " << j2["a"].as<std::string>() << ", b: " << j2["b"].as<std::string>() << "\n";
    // Access as double
    std::cout << "(4) a: " << j2["a"].as<double>() << ", b: " << j2["b"].as<double>() << "\n\n"; 
}
```
Output:
```
(1) a: 12.0, b: 1.2345678901234567
(2) a: 12, b: 1.23456789012346

(3) a: 12.00, b: 1.23456789012345678901234567890
(4) a: 12, b: 1.23456789012346
```

<div id="E4"/> 

#### Parse integer with lossless_bignum 

```cpp
#include <jsoncons/json.hpp>
#include <iostream>

int main()
{
    try
    {
        std::string str = R"({"a":123456789012345678901234567890})";

        auto options = jsoncons::json_options{}
            .lossless_bignum(true);  // default

        auto j1 = jsoncons::json::parse(str, options);
        std::string buffer1;
        j1.dump(buffer1);
        std::cout << "(1) " << buffer1 << "\n";

        options.lossless_bignum(false);
        auto j2 = jsoncons::json::parse(str, options);
        std::string buffer2;
        j2.dump(buffer2);
        std::cout << "(2) " << buffer2 << "\n";
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << "\n";
    }
}
```

Output:
```
(1) {"a":123456789012345678901234567890}
(2) {"a":1.2345678901234568e+29}
```

<div id="E5"/> 

#### Parse floating point with lossless_bignum 

```cpp
#include <jsoncons/json.hpp>
#include <iostream>

int main()
{
    try
    {
        std::string str = R"({"a":1.5e999})";

        auto options = jsoncons::json_options{}
            .lossless_bignum(true);  // default

        auto j1 = jsoncons::json::parse(str, options);
        std::string buffer1;
        j1.dump(buffer1);
        std::cout << "(1) " << buffer1 << "\n";

        options.lossless_bignum(false);
        auto j2 = jsoncons::json::parse(str, options);
        std::cout << "(2) " << j2.at("a").as<double>() << "\n";
        std::string buffer2;
        j2.dump(buffer2);
        // By default, an inf value is serialzed to null
        std::cout << "(3) " << buffer2 << "\n"; 
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << "\n";
    }
}
```

Output:
```
(1) {"a":1.5e999}
(2) inf
(3) {"a":null}
```

<div id="E6"/> 

#### Object-array block formatting

```cpp
#include <jsoncons/json.hpp>
#include <iostream>

using namespace jsoncons;

int main()
{
    json j = json::parse(R"(
{
    "normals": [1, 0, 1],
    "uvs": [0, 0, 1, 1],
    "verts": [1, 2, 3]
}
    )");

    std::cout << "multi_line: (default)" << '\n';
    auto options1 = json_options{}
        .object_array_line_splits(line_split_kind::multi_line);
    std::cout << pretty_print(j, options1) << "\n\n";

    std::cout << "new_iine:" << '\n';
    auto options3 = json_options{}
        .object_array_line_splits(line_split_kind::new_line);
    std::cout << pretty_print(j, options3) << "\n\n";

    std::cout << "same_line: " << '\n';
    auto options2 = json_options{}
        .object_array_line_splits(line_split_kind::same_line);
    std::cout << pretty_print(j, options2) << "\n\n";
}
```

Output:
```
multi_line: (default)
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

same_line:
{
    "normals": [1, 0, 1],
    "uvs": [0, 0, 1, 1],
    "verts": [1, 2, 3]
}

new_ine:
{
    "normals": [
        1, 0, 1
    ],
    "uvs": [
        0, 0, 1, 1
    ],
    "verts": [
        1, 2, 3
    ]
}
```

<div id="E7"/> 

#### Array-array block formatting

```cpp
#include <jsoncons/json.hpp>
#include <iostream>

using namespace jsoncons;

int main()
{
    auto j = json::parse(R"(
        [[0,1]]
    )");

    std::cout << "multi_line (default):" << "\n";
    auto options1 = json_options{}
        .array_array_line_splits(line_split_kind::multi_line);
    std::cout << pretty_print(j, options1) << "\n\n";

    std::cout << "new_line:" << "\n";
    auto options2 = json_options{}
        .array_array_line_splits(line_split_kind::new_line);
    std::cout << pretty_print(j, options2) << "\n\n";
    
    std::cout << "same_line:" << "\n";
    auto options3 = json_options{}
        .array_array_line_splits(line_split_kind::same_line);
    std::string buffer;
    j.dump_pretty(buffer, options3);
    std::cout << buffer << "\n";
}
```

Output:
```
multi_line (default):
[
    [
        0,
        1
    ]
]

new_line:
[
    [
        0, 1
    ]
]

same_line:
[
    [0, 1]
]
```

<div id="E8"/> 

#### Prettify single line output

```cpp
#include <jsoncons/json.hpp>
#include <iostream>

using namespace jsoncons;

int main()
{
    auto j = json::parse(R"(
        [[1,2,3,4]]
    )");

    jsoncons::json_options options;
    options.spaces_around_comma(jsoncons::spaces_option::space_after) // default when using pretty printing 
        .line_splits(jsoncons::line_split_kind::same_line);           // default is multi_line 
    
    std::cout << "(1)\n" << pretty_print(j) << "\n\n";
    std::cout << "(2)\n" << pretty_print(j, options) << "\n\n";
}
```
Output:
```
(1)
[
    [
        1,
        2,
        3,
        4
    ]
]

(2)
[[1, 2, 3, 4]]
```

<div id="E9"/> 

#### Indent with tabs

```cpp
#include <jsoncons/json.hpp>
#include <cassert>

int main()
{
    jsoncons::json j{jsoncons::json_array_arg};
    j.push_back(jsoncons::json{jsoncons::json_object_arg});
    j[0]["foo"] = 1;

    auto options = jsoncons::json_options{}
        .indent_char('\t')
        .indent_size(1);

    std::string buffer;
    j.dump_pretty(buffer, options);

    std::string expected = "[\n\t{\n\t\t\"foo\": 1\n\t}\n]";
    assert(expected == buffer);
}
```

<div id="E10"/> 

#### Allow trailing commas

```cpp
int main()
{
    std::string s = R"(
    {
        "first" : 1,
        "second" : 2,
    }
    )";

    // Default
    try
    {
        auto j = json::parse(s);
    }
    catch (const ser_error& e)
    {
        std::cout << "(1) " << e.what() << "\n\n";
    }

    // Allow trailing commas

    // until 0.170.0
    // auto j = json::parse(s, allow_trailing_commas());

    // since 1.3.0
    auto options = json_options{}
        .allow_trailing_comma(true));
    auto j = json::parse(s, options);

    std::cout << "(2)" << j << "\n\n";
}
```
Output:
```
(1) Extra comma at line 5 and column 5

(2) {"first":1,"second":2}
```

