# JSONCONS

jsoncons is a C++, header-only library for constructing [JSON](http://www.json.org) and JSON-like
data formats such as [CBOR](http://cbor.io/). For each supported data format, it enables you
to work with the data in a number of ways:

- As a variant-like data structure, [basic_json](doc/ref/basic_json.md) 

- As a strongly typed C++ data structure that implements [json_type_traits](doc/ref/json_type_traits.md)

- With cursor-level access to a stream of parse events, somewhat analogous to StAX pull parsing and push serializing
  in the XML world.

Compared to other JSON libraries, jsoncons has been designed to handle very large JSON texts. At its heart are
SAX-style parsers and serializers. It supports reading an entire JSON text in memory in a variant-like structure.
But it also supports efficient access to the underlying data using StAX-style pull parsing and push serializing.
And it supports incremental parsing into a user's preferred form, using
information about user types provided by specializations of [json_type_traits](doc/ref/json_type_traits.md).

The [jsoncons data model](doc/ref/data-model.md) supports the familiar JSON types - nulls,
booleans, numbers, strings, arrays, objects - plus byte strings. In addition, jsoncons 
supports semantic tagging of date-time values, timestamp values, big integers, 
big decimals, bigfloats and binary encodings. This allows it to preserve these type semantics when parsing 
JSON-like data formats such as CBOR that have them.

jsoncons is distributed under the [Boost Software License](http://www.boost.org/users/license.html). 

## Extensions

- [jsonpointer](doc/ref/jsonpointer/jsonpointer.md) implements the IETF standard [JavaScript Object Notation (JSON) Pointer](https://tools.ietf.org/html/rfc6901)
- [jsonpatch](doc/ref/jsonpatch/jsonpatch.md) implements the IETF standard [JavaScript Object Notation (JSON) Patch](https://tools.ietf.org/html/rfc6902)
- [jsonpath](doc/ref/jsonpath/jsonpath.md) implements [Stefan Goessner's JSONPath](http://goessner.net/articles/JsonPath/).  It also supports search and replace using JSONPath expressions.
- [cbor](doc/ref/cbor/cbor.md) implements decode from and encode to the IETF standard [Concise Binary Object Representation](http://cbor.io/) data format.
  In addition it supports tags for [stringref](http://cbor.schmorp.de/stringref) and tags for [typed arrays](https://tools.ietf.org/html/rfc8746). 
- [msgpack](doc/ref/msgpack/msgpack.md) implements decode from and encode to the [MessagePack](http://msgpack.org/index.html) data format.
- [ubjson](doc/ref/ubjson/ubjson.md) implements decode from and encode to the [Universal Binary JSON Specification](http://ubjson.org/) data format.
- [bson](doc/ref/bson/bson.md) implements decode from and encode to the [Binary JSON](http://bsonspec.org/) data format.
- [csv](doc/ref/csv/csv.md) implements decode from and encode to CSV files.

## What users say

_"I am so happy I have come across your json c++ library!"_

_"I’m using your library for an external interface to pass data, as well as using the conversions from csv to json, which are really helpful for converting data for use in javascript ... it's a great library"_

_"Verified that, for my needs in JSON and CBOR, it is working perfectly"_

_"this software is great and the ability to have an xpath like facility is so useful"_

_"I think this is the closest
cpp can get from languages with reflection behavior for serialization/deserialization"_

_"really good"_ _"awesome project"_ _"very solid and very dependable"_ _"amazing work"_ _"Your repo rocks!!!!!"_

## Get jsoncons

You can use the [vcpkg](https://github.com/Microsoft/vcpkg) platform library manager to install the [jsoncons package](https://github.com/microsoft/vcpkg/tree/master/ports/jsoncons).

Or, download the [latest release](https://github.com/danielaparker/jsoncons/releases) and unpack the zip file. Copy the directory `include/jsoncons` to your `include` directory. If you wish to use extensions, copy `include/jsoncons_ext` as well. 

Or, download the latest code on [master](https://github.com/danielaparker/jsoncons/archive/master.zip).

## How to use it

- [Quick guide](http://danielaparker.github.io/jsoncons)
- [Examples](doc/Examples.md)
- [Reference](doc/Reference.md)
- [Roadmap](Roadmap.md)

The library requires a C++ Compiler with C++11 support. In addition the library defines `jsoncons::endian`,
`jsoncons::basic_string_view`, `jsoncons::optional`, and `jsoncons::span`, which will be typedefed to
their standard library equivalents if detected. Otherwise they will be typedefed to internal, C++11 compatible, implementations.

The library uses exceptions and in some cases [std::error_code](https://en.cppreference.com/w/cpp/error/error_code)'s to report errors. Apart from `jsoncons::assertion_error`,
all jsoncons exception classes implement the [jsoncons::json_error](doc/ref/json_error.md) interface.
If exceptions are disabled or if the compile time macro `JSONCONS_NO_EXCEPTIONS` is defined, throws become calls to `std::terminate`.

## Benchmarks

[json_benchmarks](https://github.com/danielaparker/json_benchmarks) provides some measurements about how `jsoncons` compares to other `json` libraries.

- [JSONTestSuite and JSON_checker test suites](https://danielaparker.github.io/json_benchmarks/) 

- [Performance benchmarks with text and integers](https://github.com/danielaparker/json_benchmarks/blob/master/report/performance.md)

- [Performance benchmarks with text and doubles](https://github.com/danielaparker/json_benchmarks/blob/master/report/performance_fp.md)

[JSONPath Comparison](https://cburgmer.github.io/json-path-comparison/) shows how jsoncons JsonPath compares with other implementations

## Examples

[Working with JSON data](#E1)  

[Working with CBOR data](#E2)  

[Constructing json values](#E3)  

[Playing around with CBOR, JSON, and CSV](#E4)  

<div id="E1"/> 

### Working with JSON data

For the examples below you need to include some header files and initialize a string of JSON data:

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>
#include <iostream>

using namespace jsoncons; // for convenience

std::string data = R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum",
           "assertion": "advanced",
           "rated": "Marilyn C",
           "rating": 0.90,
           "confidence": 0.99
         }
       ]
    }
)";
```

jsoncons allows you to work with the data in a number of ways:

- As a variant-like data structure, [basic_json](doc/ref/basic_json.md) 

- As a strongly typed C++ data structure that implements [json_type_traits](doc/ref/json_type_traits.md)

- With [cursor-level access](doc/ref/basic_json_cursor.md) to a stream of parse events

#### As a variant-like data structure

```c++
int main()
{
    // Parse the string of data into a json value
    json j = json::parse(data);

    // Does object member reputons exist?
    std::cout << "(1) " << std::boolalpha << j.contains("reputons") << "\n\n";

    // Get a reference to reputons array 
    const json& v = j["reputons"]; 

    // Iterate over reputons array 
    std::cout << "(2)\n";
    for (const auto& item : v.array_range())
    {
        // Access rated as string and rating as double
        std::cout << item["rated"].as<std::string>() << ", " << item["rating"].as<double>() << "\n";
    }
    std::cout << "\n";

    // Select all "rated" with JSONPath
    std::cout << "(3)\n";
    json result = jsonpath::json_query(j,"$..rated");
    std::cout << pretty_print(result) << "\n\n";

    // Serialize back to JSON
    std::cout << "(4)\n" << pretty_print(j) << "\n\n";
}
```
Output:
```
(1) true

(2)
Marilyn C, 0.9

(3)
[
    "Marilyn C"
]

(4)
{
    "application": "hiking",
    "reputons": [
        {
            "assertion": "advanced",
            "confidence": 0.99,
            "rated": "Marilyn C",
            "rater": "HikingAsylum",
            "rating": 0.9
        }
    ]
}
```

#### As a strongly typed C++ data structure

jsoncons supports mapping JSON data into C++ data structures. 
The functions [decode_json](doc/ref/decode_json.md) and [encode_json](doc/ref/encode_json.md) 
convert strings or streams of JSON data to C++ data structures and back. 
Decode and encode work for all C++ classes that have 
[json_type_traits](doc/ref/json_type_traits.md) 
defined. The standard library containers are already supported, 
and your own types will be supported too if you specialize `json_type_traits`
in the `jsoncons` namespace. 

```c++
namespace ns {
    enum class hiking_experience {beginner,intermediate,advanced};

    class hiking_reputon
    {
        std::string rater_;
        hiking_experience assertion_;
        std::string rated_;
        double rating_;
        std::optional<double> confidence_; // assumes C++17, otherwise use jsoncons::optional
        std::optional<uint64_t> expires_;
    public:
        hiking_reputon(const std::string& rater,
                       hiking_experience assertion,
                       const std::string& rated,
                       double rating,
                       const std::optional<double>& confidence = std::optional<double>(),
                       const std::optional<uint64_t>& expires = std::optional<uint64_t>())
            : rater_(rater), assertion_(assertion), rated_(rated), rating_(rating),
              confidence_(confidence), expires_(expires)
        {
        }

        const std::string& rater() const {return rater_;}
        hiking_experience assertion() const {return assertion_;}
        const std::string& rated() const {return rated_;}
        double rating() const {return rating_;}
        std::optional<double> confidence() const {return confidence_;}
        std::optional<uint64_t> expires() const {return expires_;}
    };

    class hiking_reputation
    {
        std::string application_;
        std::vector<hiking_reputon> reputons_;
    public:
        hiking_reputation(const std::string& application, 
                          const std::vector<hiking_reputon>& reputons)
            : application_(application), 
              reputons_(reputons)
        {}

        const std::string& application() const { return application_;}
        const std::vector<hiking_reputon>& reputons() const { return reputons_;}
    };

} // namespace ns

// Declare the traits. Specify which data members need to be serialized.

JSONCONS_ENUM_TRAITS(ns::hiking_experience, beginner, intermediate, advanced)
// First four members listed are mandatory, confidence and expires are optional
JSONCONS_N_CTOR_GETTER_TRAITS(ns::hiking_reputon, 4, rater, assertion, rated, rating, 
                              confidence, expires)
// All members are mandatory
JSONCONS_ALL_CTOR_GETTER_TRAITS(ns::hiking_reputation, application, reputons)

int main()
{
    // Decode the string of data into a c++ structure
    ns::hiking_reputation v = decode_json<ns::hiking_reputation>(data);

    // Iterate over reputons array value
    std::cout << "(1)\n";
    for (const auto& item : v.reputons())
    {
        std::cout << item.rated() << ", " << item.rating() << "\n";
    }

    // Encode the c++ structure into a string
    std::string s;
    encode_json<ns::hiking_reputation>(v, s, indenting::indent);
    std::cout << "(2)\n";
    std::cout << s << "\n";
}
```
Output:
```
(1)
Marilyn C, 0.9
(2)
{
    "application": "hiking",
    "reputons": [
        {
            "assertion": "advanced",
            "confidence": 0.99,
            "rated": "Marilyn C",
            "rater": "HikingAsylum",
            "rating": 0.9
        }
    ]
}
```
This example makes use of the convenience macros `JSONCONS_ENUM_TRAITS`,
`JSONCONS_N_CTOR_GETTER_TRAITS`, and `JSONCONS_ALL_CTOR_GETTER_TRAITS` to specialize the 
[json_type_traits](doc/ref/json_type_traits.md) for the enum type
`ns::hiking_experience`, the class `ns::hiking_reputon` (with some non-mandatory members), and the class
`ns::hiking_reputation` (with all mandatory members.)
The macro `JSONCONS_ENUM_TRAITS` generates the code from
the enum identifiers, and the macros `JSONCONS_N_CTOR_GETTER_TRAITS`
and `JSONCONS_ALL_CTOR_GETTER_TRAITS` 
generate the code from the get functions and a constructor. 
These macro declarations must be placed outside any namespace blocks.

See [examples](doc/Examples.md#G0) for other ways of specializing `json_type_traits`.

#### With cursor-level access

A typical pull parsing application will repeatedly process the `current()` 
event and call `next()` to advance to the next event, until `done()` 
returns `true`.

```c++
int main()
{
    json_cursor cursor(data);
    for (; !cursor.done(); cursor.next())
    {
        const auto& event = cursor.current();
        switch (event.event_type())
        {
            case staj_event_type::begin_array:
                std::cout << event.event_type() << " " << "\n";
                break;
            case staj_event_type::end_array:
                std::cout << event.event_type() << " " << "\n";
                break;
            case staj_event_type::begin_object:
                std::cout << event.event_type() << " " << "\n";
                break;
            case staj_event_type::end_object:
                std::cout << event.event_type() << " " << "\n";
                break;
            case staj_event_type::key:
                // Or std::string_view, if supported
                std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::string_value:
                // Or std::string_view, if supported
                std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::null_value:
                std::cout << event.event_type() << "\n";
                break;
            case staj_event_type::bool_value:
                std::cout << event.event_type() << ": " << std::boolalpha << event.get<bool>() << "\n";
                break;
            case staj_event_type::int64_value:
                std::cout << event.event_type() << ": " << event.get<int64_t>() << "\n";
                break;
            case staj_event_type::uint64_value:
                std::cout << event.event_type() << ": " << event.get<uint64_t>() << "\n";
                break;
            case staj_event_type::double_value:
                std::cout << event.event_type() << ": " << event.get<double>() << "\n";
                break;
            default:
                std::cout << "Unhandled event type: " << event.event_type() << " " << "\n";
                break;
        }
    }    
}
```
Output:
```
Marilyn C
begin_object
key: application
string_value: hiking
key: reputons
begin_array
begin_object
key: rater
string_value: HikingAsylum
key: assertion
string_value: advanced
key: rated
string_value: Marilyn C
key: rating
double_value: 0.9
key: confidence
double_value: 0.99
end_object
end_array
end_object
```

You can apply a filter to a cursor using the pipe syntax (e.g., `cursor | filter1 | filter2 | ...`)

```c++
int main()
{
    std::string name;
    auto filter = [&](const staj_event& ev, const ser_context&) -> bool
    {
        if (ev.event_type() == staj_event_type::key)
        {
            name = ev.get<std::string>();
            return false;
        }
        if (name == "rated")
        {
            name.clear();
            return true;
        }
        return false;
    };

    json_cursor cursor(data);
    auto filtered_c = cursor | filter;

    for (; !filtered_c.done(); filtered_c.next())
    {
        const auto& event = filtered_c.current();
        switch (event.event_type())
        {
            case staj_event_type::string_value:
                // Or std::string_view, if C++17
                std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << "\n";
                break;
            default:
                std::cout << "Unhandled event type\n";
                break;
        }
    }
}    
```
Output:
```
string_value: Marilyn C
```

<div id="E2"/> 

### Working with CBOR data

For the examples below you need to include some header files and initialize a buffer of CBOR data:

```c++
#include <iomanip>
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

using namespace jsoncons; // for convenience

const std::vector<uint8_t> data = {
    0x9f, // Start indefinte length array
      0x83, // Array of length 3
        0x63, // String value of length 3
          0x66,0x6f,0x6f, // "foo" 
        0x44, // Byte string value of length 4
          0x50,0x75,0x73,0x73, // 'P''u''s''s'
        0xc5, // Tag 5 (bigfloat)
          0x82, // Array of length 2
            0x20, // -1
            0x03, // 3   
      0x83, // Another array of length 3
        0x63, // String value of length 3
          0x62,0x61,0x72, // "bar"
        0xd6, // Expected conversion to base64
        0x44, // Byte string value of length 4
          0x50,0x75,0x73,0x73, // 'P''u''s''s'
        0xc4, // Tag 4 (decimal fraction)
          0x82, // Array of length 2
            0x38, // Negative integer of length 1
              0x1c, // -29
            0xc2, // Tag 2 (positive bignum)
              0x4d, // Byte string value of length 13
                0x01,0x8e,0xe9,0x0f,0xf6,0xc3,0x73,0xe0,0xee,0x4e,0x3f,0x0a,0xd2,
    0xff // "break"
};
```

jsoncons allows you to work with the CBOR data similarly to JSON data:

- As a variant-like data structure, [basic_json](doc/ref/basic_json.md) 

- As a strongly typed C++ data structure that implements [json_type_traits](doc/ref/json_type_traits.md)

- With [cursor-level access](doc/ref/cbor/basic_cbor_cursor.md) to a stream of parse events

#### As a variant-like data structure

```c++
int main()
{
    // Parse the CBOR data into a json value
    json j = cbor::decode_cbor<json>(data);

    // Pretty print
    std::cout << "(1)\n" << pretty_print(j) << "\n\n";

    // Iterate over rows
    std::cout << "(2)\n";
    for (const auto& row : j.array_range())
    {
        std::cout << row[1].as<jsoncons::byte_string>() << " (" << row[1].tag() << ")\n";
    }
    std::cout << "\n";

    // Select the third column with JSONPath
    std::cout << "(3)\n";
    json result = jsonpath::json_query(j,"$[*][2]");
    std::cout << pretty_print(result) << "\n\n";

    // Serialize back to CBOR
    std::vector<uint8_t> buffer;
    cbor::encode_cbor(j, buffer);
    std::cout << "(4)\n" << byte_string_view(buffer) << "\n\n";
}
```
Output:
```
(1)
[
    ["foo", "UHVzcw", "0x3p-1"],
    ["bar", "UHVzcw==", "1.23456789012345678901234567890"]
]

(2)
50,75,73,73 (n/a)
50,75,73,73 (base64)

(3)
[
    "0x3p-1",
    "1.23456789012345678901234567890"
]

(4)
82,83,63,66,6f,6f,44,50,75,73,73,c5,82,20,03,83,63,62,61,72,d6,44,50,75,73,73,c4,82,38,1c,c2,4d,01,8e,e9,0f,f6,c3,73,e0,ee,4e,3f,0a,d2
```

#### As a strongly typed C++ data structure

```c++
int main()
{
    // Parse the string of data into a std::vector<std::tuple<std::string,jsoncons::byte_string,std::string>> value
    auto val = cbor::decode_cbor<std::vector<std::tuple<std::string,jsoncons::byte_string,std::string>>>(data);

    std::cout << "(1)\n";
    for (const auto& row : val)
    {
        std::cout << std::get<0>(row) << ", " << std::get<1>(row) << ", " << std::get<2>(row) << "\n";
    }
    std::cout << "\n";

    // Serialize back to CBOR
    std::vector<uint8_t> buffer;
    cbor::encode_cbor(val, buffer);
    std::cout << "(2)\n" << byte_string_view(buffer) << "\n\n";
}
```
Output:
```
(1)
foo, 50,75,73,73, 0x3p-1
bar, 50,75,73,73, 1.23456789012345678901234567890

(2)
82,9f,63,66,6f,6f,44,50,75,73,73,66,30,78,33,70,2d,31,ff,9f,63,62,61,72,44,50,75,73,73,78,1f,31,2e,32,33,34,35,36,37,38,39,30,31,32,33,34,35,36,37,38,39,30,31,32,33,34,35,36,37,38,39,30,ff
```

Note that when decoding the bigfloat and decimal fraction into a `std::string`, we lose the semantic information
that the variant like data structure preserved with a tag, so serializing back to CBOR produces a text string.

#### With cursor-level access

A typical pull parsing application will repeatedly process the `current()` 
event and call `next()` to advance to the next event, until `done()` 
returns `true`.

```c++
int main()
{
    cbor::cbor_bytes_cursor cursor(data);
    for (; !cursor.done(); cursor.next())
    {
        const auto& event = cursor.current();
        switch (event.event_type())
        {
            case staj_event_type::begin_array:
                std::cout << event.event_type() << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::end_array:
                std::cout << event.event_type() << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::begin_object:
                std::cout << event.event_type() << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::end_object:
                std::cout << event.event_type() << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::key:
                // Or std::string_view, if supported
                std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::string_value:
                // Or std::string_view, if supported
                std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::byte_string_value:
                std::cout << event.event_type() << ": " << event.get<jsoncons::span<const uint8_t>>() << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::null_value:
                std::cout << event.event_type() << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::bool_value:
                std::cout << event.event_type() << ": " << std::boolalpha << event.get<bool>() << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::int64_value:
                std::cout << event.event_type() << ": " << event.get<int64_t>() << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::uint64_value:
                std::cout << event.event_type() << ": " << event.get<uint64_t>() << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::half_value:
            case staj_event_type::double_value:
                std::cout << event.event_type() << ": "  << event.get<double>() << " " << "(" << event.tag() << ")\n";
                break;
            default:
                std::cout << "Unhandled event type " << event.event_type() << " " << "(" << event.tag() << ")\n";
                break;
        }
    }
}
```
Output:
```
begin_array (n/a)
begin_array (n/a)
string_value: foo (n/a)
byte_string_value: 50,75,73,73 (n/a)
string_value: 0x3p-1 (bigfloat)
end_array (n/a)
begin_array (n/a)
string_value: bar (n/a)
byte_string_value: 50,75,73,73 (base64)
string_value: 1.23456789012345678901234567890 (bigdec)
end_array (n/a)
end_array (n/a)
```

You can apply a filter to a cursor using the pipe syntax, 

```c++
int main()
{
    auto filter = [&](const staj_event& ev, const ser_context&) -> bool
    {
        return (ev.tag() == semantic_tag::bigdec) || (ev.tag() == semantic_tag::bigfloat);  
    };

    cbor::cbor_bytes_cursor cursor(data);
    auto filtered_c = cursor | filter;

    for (; !filtered_c.done(); filtered_c.next())
    {
        const auto& event = filtered_c.current();
        switch (event.event_type())
        {
            case staj_event_type::string_value:
                // Or std::string_view, if supported
                std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << " " << "(" << event.tag() << ")\n";
                break;
            default:
                std::cout << "Unhandled event type " << event.event_type() << " " << "(" << event.tag() << ")\n";
                break;
        }
    }
}
```
Output:
```
string_value: 0x3p-1 (bigfloat)
string_value: 1.23456789012345678901234567890 (bigdec)
```

<div id="E3"/> 

### Constructing json values

```c++
#include <iostream>
#include <fstream>
#include <jsoncons/json.hpp>

// For convenience
using jsoncons::json;

int main()
{
    json color_spaces(json_array_arg);
    color_spaces.push_back("sRGB");
    color_spaces.push_back("AdobeRGB");
    color_spaces.push_back("ProPhoto RGB");

    json image_sizing; // empty object
    image_sizing["Resize To Fit"] = true; // a boolean 
    image_sizing["Resize Unit"] = "pixels"; // a string
    image_sizing["Resize What"] = "long_edge"; // a string
    image_sizing["Dimension 1"] = 9.84; // a double
    
    json export_settings;

    // create "File Format Options" as an object and put "Color Spaces" in it
    export_settings["File Format Options"]["Color Spaces"] = std::move(color_spaces); 

    export_settings["Image Sizing"] = std::move(image_sizing);

    // Pretty print
    std::cout << pretty_print(export_settings) << "\n\n";
}
```
Output:
```json
{
    "File Format Options": {
        "Color Spaces": ["sRGB","AdobeRGB","ProPhoto RGB"],
        "Image Formats": ["JPEG","PSD","TIFF","DNG"]
    },
    "File Settings": {
        "Color Space": "sRGB",
        "Image Format": "JPEG",
        "Limit File Size": true,
        "Limit File Size To": 10000
    },
    "Image Sizing": {
        "Dimension 1": 9.84,
        "Resize To Fit": true,
        "Resize Unit": "pixels",
        "Resize What": "long_edge"
    }
}
```

<div id="E4"/> 

### Playing around with CBOR, JSON, and CSV

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/csv/csv.hpp>

// For convenience
using namespace jsoncons;    

int main()
{
    // Construct some CBOR using the streaming API
    std::vector<uint8_t> bytes_in;
    cbor::cbor_bytes_encoder encoder(bytes_in);
    encoder.begin_array(); // indefinite length outer array
    encoder.begin_array(3); // a fixed length array
    encoder.string_value("foo");
    encoder.byte_string_value(std::vector<uint8_t>{'P','u','s','s'}); // no suggested conversion
    encoder.string_value("-18446744073709551617", semantic_tag::bigint);
    encoder.end_array();
    encoder.end_array();
    encoder.flush();

    // Print bytes
    std::cout << "(1)\n" << byte_string_view(bytes_in) << "\n\n";
/*
    9f -- Start indefinte length array
      83 -- Array of length 3
        63 -- String value of length 3
          666f6f -- "foo" 
        44 -- Byte string value of length 4
          50757373 -- 'P''u''s''s'
        c3 -- Tag 3 (negative bignum)
          49 -- Byte string value of length 9
            010000000000000000 -- Bytes content
      ff -- "break" 
*/
    // Unpack bytes into a json variant value, and add some more elements
    json j = cbor::decode_cbor<json>(bytes_in);

    // Loop over the rows
    std::cout << "(2)\n";
    for (const json& row : j.array_range())
    {
        std::cout << row << "\n";
    }
    std::cout << "\n";

    // Get bignum value at position 0/2 using jsonpointer 
    json& v = jsonpointer::get(j, "/0/2");
    std::cout << "(3) " << v.as<std::string>() << "\n\n";

    // Print JSON representation with default options
    std::cout << "(4)\n";
    std::cout << pretty_print(j) << "\n\n";

    // Print JSON representation with different options
    json_options options;
    options.byte_string_format(byte_string_chars_format::base64)
           .bigint_format(bigint_chars_format::base64url);
    std::cout << "(5)\n";
    std::cout << pretty_print(j, options) << "\n\n";

    // Add some more elements

    json another_array(json_array_arg); 
    another_array.emplace_back(byte_string_arg, std::vector<uint8_t>({'P','u','s','s'}),
                               semantic_tag::base64); // suggested conversion to base64
    another_array.emplace_back("273.15", semantic_tag::bigdec);
    another_array.emplace(another_array.array_range().begin(),"bar"); // place at front

    j.push_back(std::move(another_array));
    std::cout << "(6)\n";
    std::cout << pretty_print(j) << "\n\n";

    // Get big decimal value at position /1/2 using jsonpointer
    json& ref = jsonpointer::get(j, "/1/2");
    std::cout << "(7) " << ref.as<std::string>() << "\n\n";

#if (defined(__GNUC__) || defined(__clang__)) && (!defined(__ALL_ANSI__) && defined(_GLIBCXX_USE_INT128))
    // e.g. if code compiled with GCC and std=gnu++11 (rather than std=c++11)
    __int128 i = j[1][2].as<__int128>();
#endif

    // Get byte string value at position /1/1 as a std::vector<uint8_t>
    auto bstr = j[1][1].as<std::vector<uint8_t>>();
    std::cout << "(8) " << byte_string_view(bstr) << "\n\n";

    // Repack bytes
    std::vector<uint8_t> bytes_out;
    cbor::encode_cbor(j, bytes_out);

    // Print the repacked bytes
    std::cout << "(9)\n" << byte_string_view(bytes_out) << "\n\n";
/*
    82 -- Array of length 2
      83 -- Array of length 3
        63 -- String value of length 3
          666f6f -- "foo" 
        44 -- Byte string value of length 4
          50757373 -- 'P''u''s''s'
        c3 -- Tag 3 (negative bignum)
        49 -- Byte string value of length 9
          010000000000000000 -- Bytes content
      83 -- Another array of length 3
      63 -- String value of length 3
        626172 -- "bar"
      d6 - Expected conversion to base64
      44 -- Byte string value of length 4
        50757373 -- 'P''u''s''s'
      c4 -- Tag 4 (decimal fraction)
        82 -- Array of length 2
          21 -- -2
          19 6ab3 -- 27315
*/
    // Encode to CSV
    csv::csv_options csv_options;
    csv_options.column_names("Column 1,Column 2,Column 3");

    std::cout << "(10)\n";
    csv::encode_csv(j, std::cout, csv_options);    
}

```
Output:
```
(1)
9f,83,63,66,6f,6f,44,50,75,73,73,c3,49,01,00,00,00,00,00,00,00,00,ff

(2)
["foo","UHVzcw","-18446744073709551617"]

(3) -18446744073709551617

(4)
[
    ["foo", "UHVzcw", "-18446744073709551617"]
]

(5)
[
    ["foo", "UHVzcw==", "~AQAAAAAAAAAA"]
]

(6)
[
    ["foo", "UHVzcw", "-18446744073709551617"],
    ["bar", "UHVzcw==", "273.15"]
]

(7) 273.15

(8) 50,75,73,73

(9)
82,83,63,66,6f,6f,44,50,75,73,73,c3,49,01,00,00,00,00,00,00,00,00,83,63,62,61,72,d6,44,50,75,73,73,c4,82,21,19,6a,b3

(10)
Column 1,Column 2,Column 3
foo,UHVzcw,-18446744073709551617
bar,UHVzcw==,273.15
```

## Supported compilers

jsoncons requires a C++11 compiler. It is tested in continuous integration on [AppVeyor](https://ci.appveyor.com/project/danielaparker/jsoncons), [Travis](https://travis-ci.org/danielaparker/jsoncons), and [doozer](https://doozer.io/).
[UndefinedBehaviorSanitizer (UBSan)](http://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) diagnostics are enabled for selected gcc and clang builds.
Since v0.151.0, it is integrated with [Google OSS-fuzz](https://github.com/google/oss-fuzz), with coverage for all parsers and encoders.

| Compiler                | Version                   |Architecture | Operating System  | Notes |
|-------------------------|---------------------------|-------------|-------------------|-------|
| Microsoft Visual Studio | vs2015 (MSVC 19.0.24241.7)| x86,x64     | Windows 10        |       |
|                         | vs2017                    | x86,x64     | Windows 10        |       |
|                         | vs2019                    | x86,x64     | Windows 10        |       |
| g++                     | 4.8 and above             | x64         | Ubuntu            |`std::regex` isn't fully implemented in 4.8, so `jsoncons::jsonpath` regular expression filters aren't supported in 4.8 |
|                         | 4.8.5                     | x64         | CentOS 7.6        |`std::regex` isn't fully implemented in 4.8, so `jsoncons::jsonpath` regular expression filters aren't supported in 4.8 |
|                         | 6.3.1 (Red Hat 6.3.1-1)   | x64         | Fedora release 24 |       |
|                         | 4.9.2                     | i386        | Debian 8          |       |
| clang                   | 3.8 and above             | x64         | Ubuntu            |       |
| clang xcode             | 6.4 and above             | x64         | OSX               |       |

## Building the test suite and examples with CMake

[CMake](https://cmake.org/) is a cross-platform build tool that generates makefiles and solutions for the compiler environment of your choice. On Windows you can download a [Windows Installer package](https://cmake.org/download/). On Linux it is usually available as a package, e.g., on Ubuntu,
```
sudo apt-get install cmake
```
Once cmake is installed, you can build the tests:
```
mkdir build
cd build
cmake ../ -DBUILD_TESTS=ON
cmake --build . --target test_jsoncons --config Release
```
Run from the jsoncons tests directory:

On Windows:
```
..\build\tests\Release\test_jsoncons
```

On UNIX:
```
../build/tests/Release/test_jsoncons
```

## Acknowledgements

A big thanks to the comp.lang.c++ community for help with implementation details. 

The jsoncons platform dependent binary configuration draws on to the excellent MIT licensed [tinycbor](https://github.com/intel/tinycbor).

Thanks to Milo Yip, author of [RapidJSON](http://rapidjson.org/), for raising the quality of JSON libraries across the board, by publishing [the benchmarks](https://github.com/miloyip/nativejson-benchmark), and contacting this project (among others) to share the results.

The jsoncons implementation of the Grisu3 algorithm for printing floating-point numbers follows Florian Loitsch's MIT licensed [grisu3_59_56 implementation](http://florian.loitsch.com/publications), with minor modifications. 

The macro `JSONCONS_ALL_MEMBER_TRAITS` was inspired by Martin York's [ThorsSerializer](https://github.com/Loki-Astari/ThorsSerializer)

Special thanks to our [contributors](https://github.com/danielaparker/jsoncons/blob/master/acknowledgements.md)
 
