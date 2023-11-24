### jsoncons::jsonpath::basic_json_location

```cpp
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

template <class CharT, class Allocator = std::allocator<CharT>>     (since 0.172.0)
class basic_json_location
```

Two specializations for common character types are defined:

Type      |Definition
----------|------------------------------
json_location   |`basic_json_location<char>`
wjson_location  |`basic_json_location<wchar_t>`

Objects of type `basic_json_location` represents the location of a specific value in a JSON document.

#### Member types
Type        |Definition
------------|------------------------------
char_type   | `CharT`
allocator_type | Allocator
string_view_type | `jsoncons::basic_string_view<char_type>`
value_type  | basic_path_element<CharT,Allocator>
const_iterator | A constant [LegacyRandomAccessIterator](https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator) with a `value_type` of [basic_path_element<char_type>](basic_path_element.md)
iterator    | An alias to `const_iterator`

#### Constructors

    basic_json_location(const allocator_type& alloc=Allocator());         (1)

    basic_json_location(const basic_path_node<char_type>& path, 
        const allocator_type& alloc=Allocator());                         (2)

    basic_json_location(const basic_json_location&);                      (3)

    basic_json_location(basic_json_location&&) noexcept;                  (4)

(1) Constructs an empty `basic_json_location`.

(2) Constructs a `basic_json_location` from a path node.

(3) Copy constructor

(4) Move constructor

#### operator=

    basic_json_location& operator=(const basic_json_location&);

    basic_json_location& operator=(basic_json_location&&);

#### Iterators

    iterator begin() const;
    iterator end() const;
Iterator access to the tokens in the pointer.

#### Accessors

    bool empty() const
Checks if the location is empty

    std::size_t size() const

    const path_element_type& operator[](std::size_t index) const

#### Modifiers

    void clear();    

    basic_json_location& append(const string_view_type& name)
Appends `name` to the location.

    template <class IntegerType>
    basic_json_location& append(IntegerType index) 
Appends `index` to the location.
This overload only participates in overload resolution if `IntegerType` is an integer type.

    basic_json_location& append(const basic_json_location& relative_location)
Appends `relative_location` to the location.

#### Static member functions

   static parse(const string_view_type& str);
   static parse(const string_view_type& str, std::error_code& ec);
Constructs a `basic_json_location` from a normalized path, or an equivalent representation
using the dot notation.

#### Non-member functions

    template<class Json>
    Json* get(Json& root_value, const basic_json_location<Json::char_type>& location)
Returns a pointer to a JSON value at the specified `location` in the `root_value`.

    template<class Json>
    std::size_t remove(Json& root_value, const basic_json_location<Json::char_type>& location)

    template <class CharT, class Allocator = std::allocator<CharT>>
    std::basic_string<CharT, std::char_traits<CharT>, Allocator> to_basic_string(const basic_json_location<CharT,Allocator>& location, 
        const Allocator& alloc = Allocator())
Returns a normalized path

    std::string to_string(const json_location& location)

    std::wstring to_wstring(const wjson_location& location)

### Examples

The examples below uses the sample data file `books.json`, 

```json
{
    "books":
    [
        {
            "category": "fiction",
            "title" : "A Wild Sheep Chase",
            "author" : "Haruki Murakami",
            "price" : 22.72
        },
        {
            "category": "fiction",
            "title" : "The Night Watch",
            "author" : "Sergei Lukyanenko",
            "price" : 23.58
        },
        {
            "category": "fiction",
            "title" : "The Comedians",
            "author" : "Graham Greene",
            "price" : 21.99
        },
        {
            "category": "memoir",
            "title" : "The Night Watch",
            "author" : "Phillips, David Atlee"
        }
    ]
}
```

#### Select the normalized paths to some books, and remove them
 
```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>
#include <fstream>

using jsoncons::json; 
namespace jsonpath = jsoncons::jsonpath;

int main()
{
    std::ifstream is(/*path_to_books_file*/);
    json doc = json::parse(is);

    auto expr = jsonpath::make_expression<json>("$.books[?(@.category == 'fiction')]");
    std::vector<jsonpath::json_location> locations = expr.select_paths(doc, 
        jsonpath::result_options::sort_descending);

    for (const auto& location : locations)
    {
        std::cout << jsonpath::to_string(location) << "\n";
    }
    std::cout << "\n";

    for (const auto& location : locations)
    {
        jsonpath::remove(doc, location);
    }

    std::cout << jsoncons::pretty_print(doc) << "\n\n";
} 
```

Output:

```
$['books'][2]
$['books'][1]
$['books'][0]

{
    "books": [
        {
            "author": "Phillips, David Atlee",
            "category": "memoir",
            "title": "The Night Watch"
        }
    ]
}
```
                        
#### Get a pointer to the book at index 1

```
jsonpath::json_location loc;
loc.append("store").append("book").append(1);

json* ptr = jsonpath::get(doc, loc);    
```

#### Remove the book at index 2

```
jsonpath::json_location loc;
loc.append("store").append("book").append(2);

jsonpath::remove(doc, loc);    
```
                        
