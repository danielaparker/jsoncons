### jsoncons::try_decode_json

Decodes a JSON data format to a C++ data structure. `try_decode_json` will 
work for all C++ classes that have reflection traits defined.

```cpp
#include <jsoncons/decode_json.hpp>

template <typename T,typename CharsLike>
read_result<T> try_decode_json(const CharsLike& s,
    const basic_json_decode_options<CharsLike::value_type>& options 
        = basic_json_decode_options<CharsLike::value_type>());                                  (1) since 1.4.0

template <typename T,typename CharT>
read_result<T> try_decode_json(std::basic_istream<CharT>& is,
    const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>());      (2) since 1.4.0

template <typename T,typename CharsLike,typename Allocator,typename TempAllocator>
read_result<T> try_decode_json(const allocator_set<Allocator,TempAllocator>& alloc_set,
    const CharsLike& s,
    const basic_json_decode_options<CharsLike::value_type>& options 
        = basic_json_decode_options<CharsLike::value_type>());                                  (3) (since 1.4.0)

template <typename T,typename CharT,typename Allocator,typename TempAllocator>
read_result<T> try_decode_json(const allocator_set<Allocator,TempAllocator>& alloc_set,
    std::basic_istream<CharT>& is,
    const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>());      (4) (since 1.4.0)

template <typename T,typename Iterator>
read_result<T> try_decode_json(Iterator first, Iterator last,
    const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>());      (5) since 1.4.0
```

(1) Reads JSON from a contiguous character sequence provided by `s` into a type T, using the specified (or defaulted) [options](basic_json_options.md). 
Type 'T' must be an instantiation of [basic_json](../basic_json.md) or support jsoncons reflection traits.

(2) Reads JSON from an input stream into a type T, using the specified (or defaulted) [options](basic_json_options.md). 
Type 'T' must be an instantiation of [basic_json](../basic_json.md) or support jsoncons reflection traits.

(3)-(4) are identical to (1)-(2) except an [allocator_set](allocator_set.md) is passed as an additional argument and
provides allocators for result data and temporary allocations.

(5) Reads JSON from the range [first,last) into a type T, using the specified (or defaulted) [options](basic_json_options.md). 

#### Return value

[read_result<T>](read_result.md)

#### Exceptions

No exceptions thrown by jsoncons library.

### Examples

```cpp
#include <jsoncons/json.hpp>
#include <iostream>

namespace ns {

struct book
{
    std::string author;
    std::string title;
    double price{0};
};

} // namespace ns

JSONCONS_ALL_MEMBER_TRAITS(ns::book, author, title, price)

int main()
{
    // Parsing error
    {
        std::string input = R"(
{
    "author" : "Haruki Murakami", 
    "title" : "Kafka on the Shore",
    "price" 25.17        
}
        )";

        auto result = jsoncons::try_decode_json<ns::book>(input);
        if (!result)
            std::cout << "(1) " << result.error() << "\n\n";
    }

    // Unexpected JSON
    {
        std::string input = R"(["Haruki Murakami", "Kafka on the Shore", 25.17])";

        auto result = jsoncons::try_decode_json<ns::book>(input);
        if (!result)
            std::cout << "(2) " << result.error() << "\n\n";
    }

    // Missing required member
    {
        std::string input = R"(
{
    "author" : "Haruki Murakami", 
    "title" : "Kafka on the Shore"        
}
        )";

        auto result = jsoncons::try_decode_json<ns::book>(input);
        if (!result)
            std::cout << "(3) " << result.error() << "\n\n";
    }

    // Invalid JSON
    {
        std::string input = R"(
{
    "author" : "Haruki Murakami", 
    "title" : "Kafka on the Shore",
    "price" : "foo"        
}
        )";

        auto result = jsoncons::try_decode_json<ns::book>(input);
        if (!result)
            std::cout << "(4) " << result.error() << "\n\n";
    }

    // Success
    {
        std::string input = R"(
{
    "author" : "Haruki Murakami", 
    "title" : "Kafka on the Shore",
    "price" : 25.17        
}
        )";

        auto result = jsoncons::try_decode_json<ns::book>(input);
        if (result)            
        {
            ns::book& book{result.value()};
            std::cout << "(5) " << book.author << ", " << book.title << ", " << book.price << "\n\n";
        }
    }
}
```

Output:

```
(1) Expected name separator ':' at line 5 and column 13

(2) ns::book: Expected source object at line 1 and column 2

(3) ns::book: price: Missing required JSON object member at line 2 and column 2

(4) ns::book: price: Unable to convert into the provided type at line 2 and column 2

(5) Haruki Murakami, Kafka on the Shore, 25.17
```
