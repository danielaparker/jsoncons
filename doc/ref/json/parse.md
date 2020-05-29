### jsoncons::basic_json::parse

```c++
static basic_json parse(const string_view_type& s, 
                        const basic_json_decode_options<char_type>& options = basic_json_decode_options<CharT>(), 
                        std::function<bool(json_errc,const ser_context&)> err_handler = default_json_parsing()); // (1)

static basic_json parse(const string_view_type& s, 
                        std::function<bool(json_errc,const ser_context&)> err_handler); // (2)

static basic_json parse(std::basic_istream<char_type>& is, 
                        const basic_json_decode_options<char_type>& options = basic_json_decode_options<CharT>(), 
                        std::function<bool(json_errc,const ser_context&)> err_handler = default_json_parsing()); // (3)
              
static basic_json parse(std::istream& is, 
                        std::function<bool(json_errc,const ser_context&)> err_handler); // (4)

template <class Iterator>
static basic_json parse(Iterator first, Iterator last, 
                        const basic_json_decode_options<char_type>& options = basic_json_decode_options<CharT>(), 
                        std::function<bool(json_errc,const ser_context&)> err_handler = default_json_parsing()); // (5)
              
template <class Iterator>
static basic_json parse(Iterator first, Iterator last, 
                        std::function<bool(json_errc,const ser_context&)> err_handler); // (6)
```
(1) - (2) Parses JSON data from a string and returns a `basic_json` value. 
Throws [ser_error](ser_error.md) if parsing fails.

(3) - (4) Parses JSON data from an input stream and returns a `basic_json` value. 
Throws [ser_error](ser_error.md) if parsing fails.

(5) - (6) Parses JSON data from the range [`first`,`last`) and returns a `basic_json` value. 
Throws [ser_error](ser_error.md) if parsing fails.

### Examples

#### Parse from stream

Input JSON file `example.json`:

```json
{"File Format Options":{"Color Spaces":["sRGB","AdobeRGB","ProPhoto RGB"]}}
```

```c++
std::ifstream is("example.json");
json j = json::parse(is);

std::cout << pretty_print(j) << std::endl;
```

Output:

```json
{
    "File Format Options": {
        "Color Spaces": ["sRGB","AdobeRGB","ProPhoto RGB"]
    }
}
```

#### Parse from string

```c++
try 
{
    json val = json::parse("[1,2,3,4,]");
} 
catch(const jsoncons::ser_error& e) 
{
    std::cout << e.what() << std::endl;
}
```
Output:
```
Extra comma at line 1 and column 10
```

#### Parse from string with serializing options

```c++
std::string s = R"({"field1":"NaN","field2":"PositiveInfinity","field3":"NegativeInfinity"})";

json_options options;
options.nan_to_str("NaN")
       .inf_to_str("PositiveInfinity")
       .neginf_to_str("NegativeInfinity");

json j = json::parse(s,options);

std::cout << "\n(1)\n" << pretty_print(j) << std::endl;

std::cout << "\n(2)\n" << pretty_print(j,options) << std::endl;
```
Output:
```
(1)
{
    "field1": null,
    "field2": null,
    "field3": null
}

(2)
{
    "field1": "NaN",
    "field2": "PositiveInfinity",
    "field3": "NegativeInfinity"
}
```



