### `jsoncons::json::dump`

```c++
template <class SAllocator>
void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s) const; // (1)

template <class SAllocator>
void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s, 
          const serialization_options& options) const; // (2)

void dump(std::ostream& os) const; // (3)

void dump(std::ostream<CharT> os, const serialization_options& options) const; // (4)

void dump(basic_json_output_handler<char_type>& output_handler) const; // (5)

void dump_fragment(json_output_handler& handler) const; // (6)
```

(1) Inserts json value into string using default serialization_options.

(2) Inserts json value into string using specified [serialization_options](../serialization_options.md). 

(3) Calls `begin_json()` on `output_handler`, emits json value to `output_handler`, and calls `end_json()` on `output_handler`. 

(4) Inserts json value into stream with default serialization options. 

(5) Inserts json value into stream using specified [serialization_options](../serialization_options.md). 

(6) Emits JSON events for JSON objects, arrays, object members and array elements to a [json_output_handler](../json_output_handler.md), such as a [json_serializer](../json_serializer.md). 

### Examples

#### Dump json "price" value to a string

```c++
#include <jsoncons/json.hpp>

using namespace jsoncons;

int main()
{
    const json books = json::parse(R"(
    [
        {
            "title" : "Kafka on the Shore",
            "author" : "Haruki Murakami",
            "price" : 25.17
        },
        {
            "title" : "Women: A Novel",
            "author" : "Charles Bukowski",
            "price" : 12.00
        },
        {
            "title" : "Cutter's Way",
            "author" : "Ivan Passer"
        }
    ]
    )");

    for (const auto& book : books.array_range())
    {
        try
        {
            std::string author = book["author"].as<std::string>();
            std::string title = book["title"].as<std::string>();
            std::string price;
            book.get_with_default<json>("price", "N/A").dump(price);
            std::cout << author << ", " << title << ", " << price << std::endl;
        }
        catch (const parse_error& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}
```
Output:
```
Haruki Murakami, Kafka on the Shore, 25.17
Charles Bukowski, Women: A Novel, 12.0
Ivan Passer, Cutter's Way, "N/A"
```


