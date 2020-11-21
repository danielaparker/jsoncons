### jsoncons::jsonpointer::basic_json_pointer

```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template <class CharT>
class basic_json_pointer
```

Two specializations for common character types are defined:

Type      |Definition
----------|------------------------------
json_pointer   |`basic_json_pointer<char>` (since 0.159.0)
json_ptr       |`basic_json_pointer<char>` (until 0.159.0)
wjson_pointer  |`basic_json_pointer<wchar_t>` (since 0.159.0)
wjson_ptr      |`basic_json_pointer<wchar_t>` (unitl 0.159.0)

Objects of type `basic_json_pointer` represent a JSON Pointer.

#### Member types
Type        |Definition
------------|------------------------------
char_type   | `CharT`
string_type | `std::basic_string<char_type>`
string_view_type | `jsoncons::basic_string_view<char_type>`
const_iterator | A constant [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) with a `value_type` of `std::basic_string<char_type>`
iterator    | An alias to `const_iterator`

#### Constructors

    basic_json_pointer();

    explicit basic_json_pointer(const string_type& pointer);

    explicit basic_json_pointer(string_type&& pointer);

    explicit basic_json_pointer(const CharT* s);

    basic_json_pointer(const basic_json_pointer&);

    basic_json_pointer(basic_json_pointer&&) noexcept;

#### operator=

    basic_json_pointer& operator=(const basic_json_pointer&);

    basic_json_pointer& operator=(basic_json_pointer&&);

#### Modifiers

    basic_json_pointer& operator/=(const string_type& s)
First, appends the JSON Pointer separator `/`. Then appends the token s, escaping any `/` or `~` characters.

    template <class IntegerType>
    basic_json_pointer& operator/=(IntegerType index) 
First, appends the JSON Pointer separator `/`. Then appends the token `index`.
This overload only participates in overload resolution if `IntegerType` is an integer type.

    basic_json_pointer& operator+=(const basic_json_pointer& ptr)
Concatenates the current pointer and the specified pointer `ptr`. 

#### Iterators

    iterator begin() const;
    iterator end() const;
Iterator access to the tokens in the pointer.

#### Accessors

    bool empty() const
Checks if the pointer is empty

    const string_view_type& string_view() const
    operator string_view_type() const;
Access the JSON Pointer pointer as a string view.

#### Non-member functions
    basic_json_pointer<CharT> operator/(const basic_json_pointer<CharT>& lhs, const basic_string<CharT>& s);
Concatenates a JSON Pointer pointer and a string. Effectively returns basic_json_pointer<CharT>(lhs) /= s.

    template <class CharT,class IntegerType>
    basic_json_pointer<CharT> operator/(const basic_json_pointer<CharT>& lhs, IntegerType index);
Concatenates a JSON Pointer pointer and an index. Effectively returns basic_json_pointer<CharT>(lhs) /= index.
This overload only participates in overload resolution if `IntegerType` is an integer type.

    template <class CharT,class IntegerType>
    basic_json_pointer<CharT> operator+( const basic_json_pointer<CharT>& lhs, const basic_json_pointer<CharT>& rhs );
Concatenates two JSON Pointers. Effectively returns basic_json_pointer<CharT>(lhs) += rhs.

    template <class CharT,class IntegerType>
    bool operator==(const basic_json_pointer<CharT>& lhs, const basic_json_pointer<CharT>& rhs);

    template <class CharT,class IntegerType>
    bool operator!=(const basic_json_pointer<CharT>& lhs, const basic_json_pointer<CharT>& rhs);

    template <class CharT,class IntegerType>
    std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const basic_json_pointer<CharT>& ptr);
Performs stream output

### Examples

#### Iterate over the tokens in a JSON Pointer

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

// for brevity
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    jsonpointer::json_pointer ptr("/store/book/1/author");

    std::cout << "(1) " << ptr << "\n\n";

    std::cout << "(2)\n";
    for (const auto& token : ptr)
    {
        std::cout << token << "\n";
    }

    std::cout << "\n";
}
```
Output:
```
(1) /store/book/1/author

(2)
store
book
1
author
```

#### Append tokens to a JSON Pointer

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    jsonpointer::json_pointer ptr;

    ptr /= "a/b";
    ptr /= "";
    ptr /= "m~n";

    std::cout << "(1) " << ptr << "\n\n";

    std::cout << "(2)\n";
    for (const auto& token : ptr)
    {
        std::cout << token << "\n";
    }

    std::cout << "\n";
}
```
Output:
```
(1) /a~1b//m~0n

(2)
a/b

m~n
```

#### Concatentate two JSONPointers

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    jsonpointer::json_pointer ptr("/a~1b");

    ptr += jsonpointer::json_pointer("//m~0n");

    std::cout << "(1) " << ptr << "\n\n";

    std::cout << "(2)\n";
    for (const auto& token : ptr)
    {
        std::cout << token << "\n";
    }

    std::cout << "\n";
}
```
Output:
```
(1) /a~1b//m~0n

(2)
a/b

m~n
```

