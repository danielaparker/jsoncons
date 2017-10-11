### jsoncons::json::parse

```c++
static json parse(const string_view_type& s)
static json parse(const string_view_type& s, 
                  parse_error_handler& err_handler)
```
Parses a string of JSON text and returns a json object or array value. 
Throws [parse_error](parse_error.md) if parsing fails.

```c++
static json parse(std::istream& is)
static json parse(std::istream& is, 
                  parse_error_handler& err_handler)
```
Parses an input stream of JSON text and returns a json object or array value. 
Throws [parse_error](parse_error.md) if parsing fails.

### Examples

#### Parse from stream

Input example.json:

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
catch(const jsoncons::parse_error& e) 
{
    std::cout << e.what() << std::endl;
}
```
Output:
```
Extra comma at line 1 and column 10
```



