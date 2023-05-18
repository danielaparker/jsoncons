### jsoncons::basic_json::parse

```c++
template <class Source>
static basic_json parse(const Source& source, 
    const basic_json_decode_options<char_type>& options = basic_json_decode_options<CharT>(),    (1) (since 0.171.0)
    const allocator_type& alloc = allocator_type());                                                 

template <class Source>
static basic_json parse(const Source& source, 
    const basic_json_decode_options<char_type>& options,                                         (2) (deprecated since 0.171.0)
    std::function<bool(json_errc,const ser_context&)> err_handler);                              

template <class Source>
static basic_json parse(const Source& source, 
    std::function<bool(json_errc,const ser_context&)> err_handler);                              (3) (deprecated since 0.171.0)

static basic_json parse(const char_type* str, 
    const basic_json_decode_options<char_type>& options = basic_json_decode_options<CharT>(), 
    const allocator_type& alloc = allocator_type());                                             (4) (since 0.171.0)

static basic_json parse(const char_type* str, 
    const basic_json_decode_options<char_type>& options,                                         (5) (deprecated since 0.171.0)
    std::function<bool(json_errc,const ser_context&)> err_handler);                              

static basic_json parse(const char_type* str, 
    std::function<bool(json_errc,const ser_context&)> err_handler);                              (6) (deprecated since 0.171.0)

static basic_json parse(std::basic_istream<char_type>& is, 
    const basic_json_decode_options<char_type>& options = basic_json_decode_options<CharT>(),    (7) (since 0.171.0)
    const allocator_type& alloc = allocator_type()); (5)

static basic_json parse(std::basic_istream<char_type>& is, 
    const basic_json_decode_options<char_type>& options,                                         (8) (deprecated since 0.171.0)
    std::function<bool(json_errc,const ser_context&)> err_handler);                              
              
static basic_json parse(std::istream& is, 
    std::function<bool(json_errc,const ser_context&)> err_handler);                              (9) (deprecated since 0.171.0)

template <class InputIt>
static basic_json parse(InputIt first, InputIt last, 
    const basic_json_decode_options<char_type>& options = basic_json_decode_options<CharT>(),    (10) (since 0.171.0)
    const allocator_type& alloc = allocator_type());           

template <class InputIt>
static basic_json parse(InputIt first, InputIt last, 
    const basic_json_decode_options<char_type>& options,                                         (11) (deprecated since 0.171.0)
    std::function<bool(json_errc,const ser_context&)> err_handler);           
              
template <class InputIt>
static basic_json parse(InputIt first, InputIt last,                                             (12) (deprecated since 0.171.0)
    std::function<bool(json_errc,const ser_context&)> err_handler);                              
```
(1) - (3) Parses JSON data from a contiguous character sequence provided by `source` and returns a `basic_json` value. 
Throws a [ser_error](../ser_error.md) if parsing fails.

(4) - (6) Parses JSON data from a null-terminated character string and returns a `basic_json` value. 
Throws a [ser_error](../ser_error.md) if parsing fails.

(7) - (9) Parses JSON data from an input stream and returns a `basic_json` value. 
Throws a [ser_error](../ser_error.md) if parsing fails.

(10) - (12) Parses JSON data from the range [`first`,`last`) and returns a `basic_json` value. 
Throws a [ser_error](../ser_error.md) if parsing fails.

#### Parameters

`source` = a contigugous character source, such as a `std::string` or `std::string_view`

`str` - a null terminated character string  

`is` - an input stream  

`first`, `last` - pair of [LegacyInputIterators](https://en.cppreference.com/w/cpp/named_req/InputIterator) that specify a character sequence  

`options` - a [basic_json_options](../basic_json_options.md)  

`err_handler` - an error handler. Since 0.171.0, an error handler may be provided as a member of a [basic_json_options](../basic_json_options.md).  

### Examples

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

#### Parse from string with options

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

#### Parse from pair of input iterators

```c++
#include <jsoncons/json.hpp>

class MyIterator
{
    const char* p_;
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = char;
    using difference_type = std::ptrdiff_t;
    using pointer = const char*; 
    using reference = const char&;

    MyIterator(const char* p)
        : p_(p)
    {
    }

    reference operator*() const
    {
        return *p_;
    }

    pointer operator->() const 
    {
        return p_;
    }

    MyIterator& operator++()
    {
        ++p_;
        return *this;
    }

    MyIterator operator++(int) 
    {
        MyIterator temp(*this);
        ++*this;
        return temp;
    }

    bool operator!=(const MyIterator& rhs) const
    {
        return p_ != rhs.p_;
    }
};

int main()
{
    char source[] = {'[','\"', 'f','o','o','\"',',','\"', 'b','a','r','\"',']'};

    MyIterator first(source);
    MyIterator last(source + sizeof(source));

    json j = json::parse(first, last);

    std::cout << j << "\n\n";
}
```

Output:
```json
["foo","bar"]
```



