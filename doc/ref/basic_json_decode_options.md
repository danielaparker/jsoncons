### jsoncons::basic_json_decode_options

```c++
#include <jsoncons/json_options.hpp>

template< 
    class CharT
> class basic_json_decode_options;
```

An abstract class that defines accessors for JSON parsing options.

#### Implementing classes

[basic_json_options](basic_json_options.md)

Typedefs for common character types are provided:

Type                |Definition
--------------------|------------------------------
`json_decode_options`        |`basic_json_decode_options<char>`
`wjson_decode_options`       |`basic_json_decode_options<wchar_t>`

Member type                         |Definition
------------------------------------|------------------------------
`string_type`|`std::basic_string<CharT>`

#### Destructor

    virtual ~json_decode_options();

#### Accessors

    virtual bool is_str_to_nan() const = 0;
Indicates `NaN` replacement for string when parsing.

    virtual string_type nan_to_str() const = 0;
When parsing JSON text, replace string with a `NaN` if `is_nan_to_str()` returns `true`.

    virtual bool is_str_to_inf() const = 0;
Indicates `Infinity` replacement for string when parsing.

    virtual const string_type& inf_to_str() const = 0; 
When parsing JSON text, replace string with infinity if `is_inf_to_str()` returns `true`.

    virtual bool is_str_to_neginf() const = 0;
Indicates `Negative Infinity` replacement for string when parsing.

    virtual const string_type& neginf_to_str() const = 0; 
When parsing JSON text, replace string with minus infinity if `is_neginf_to_str()` returns true.

    virtual bool lossless_number() const = 0; 
If set to `true`, parse decimal numbers as strings with semantic tagging `semantic_tag::bigdec` instead of double.

    virtual size_t max_nesting_depth() = 0;
 Maximum nesting depth when parsing JSON.

