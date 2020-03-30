### jsoncons::jsonpointer::basic_json_ptr

```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template <class CharT>
class basic_json_ptr
```

Two specializations for common character types are defined:

Type      |Definition
----------|------------------------------
json_ptr   |`basic_json_ptr<char>`
wjson_ptr  |`basic_json_ptr<wchar_t>`

Objects of type `basic_json_ptr` represent a JSON Pointer.

#### Member types
Type        |Definition
------------|------------------------------
char_type   | `CharT`
string_type | `std::basic_string<char_type>`
string_view_type | `jsoncons::basic_string_view<char_type>`
const_iterator | A constant [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) with a `value_type` of `std::basic_string<char_type>`
iterator    | An alias to `const_iterator`

#### Constructors

    basic_json_ptr();

    explicit basic_json_ptr(const string_type& pointer);

    explicit basic_json_ptr(string_type&& pointer);

    explicit basic_json_ptr(const CharT* s);

    basic_json_ptr(const basic_json_ptr&);

    basic_json_ptr(basic_json_ptr&&) noexcept;

#### operator=

    basic_json_ptr& operator=(const basic_json_ptr&);

    basic_json_ptr& operator=(basic_json_ptr&&);

#### Modifiers

    basic_json_ptr& operator/=(const string_type& s)
First, appends the JSON Pointer separator `/`. Then appends the token s, escaping any `/` or `~` characters.

    basic_json_ptr& operator+=(const basic_json_ptr& ptr)
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
    basic_json_ptr<CharT> operator/(const basic_json_ptr<CharT>& lhs, const string_type& rhs);
Concatenates a JSON Pointer pointer and a token. Effectively returns basic_json_ptr<CharT>(lhs) /= rhs.

    basic_json_ptr<CharT> operator+( const basic_json_ptr<CharT>& lhs, const basic_json_ptr<CharT>& rhs );
Concatenates two JSON Pointers. Effectively returns basic_json_ptr<CharT>(lhs) += rhs.

    bool operator==(const basic_json_ptr<CharT>& lhs, const basic_json_ptr<CharT>& rhs);

    bool operator!=(const basic_json_ptr<CharT>& lhs, const basic_json_ptr<CharT>& rhs);

    std::basic_ostream<CharT>&
    operator<<(std::basic_ostream<CharT>& os, const basic_json_ptr<CharT>& ptr);
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
    jsonpointer::json_ptr ptr("/store/book/1/author");

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
    jsonpointer::json_ptr ptr;

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
    jsonpointer::json_ptr ptr("/a~1b");

    ptr += jsonpointer::json_ptr("//m~0n");

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

