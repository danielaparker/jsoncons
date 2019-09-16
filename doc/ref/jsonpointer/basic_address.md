### jsoncons::jsonpointer::basic_address

```
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template <class CharT>
class basic_address
```

Two specializations for common character types are defined:

Type      |Definition
----------|------------------------------
address   |`basic_address<char>`
waddress  |`basic_address<wchar_t>`

Objects of type `basic_address` represent a JSON Pointer.

#### Member types
Type        |Definition
------------|------------------------------
char_type   | `CharT`
string_type | `std::basic_string<char_type>`
string_view_type | `jsoncons::basic_string_view<char_type>`
const_iterator | A constant [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) with a `value_type` of `std::basic_string<char_type>`
iterator    | An alias to `const_iterator`

#### Constructors

    basic_address();

    explicit basic_address(const string_type& pointer);

    explicit basic_address(string_type&& pointer);

    explicit basic_address(const CharT* s);

    basic_address(const basic_address&);

    basic_address(basic_address&&) noexcept;

#### operator=

    basic_address& operator=(const basic_address&);

    basic_address& operator=(basic_address&&);

#### Modifiers

    basic_address& operator/=(const string_type& s)
First, appends the JSON Pointer separator `/`. Then appends the token s, escaping any `/` or `~` characters.

    basic_address& operator+=(const basic_address& p)
Concatenates the current pointer and the specified pointer `p`. 

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
    basic_address<CharT> operator/(const basic_address<CharT>& lhs, const string_type& rhs);
Concatenates a JSON Pointer pointer and a token. Effectively returns basic_address<CharT>(lhs) /= rhs.

    basic_address<CharT> operator+( const basic_address<CharT>& lhs, const basic_address<CharT>& rhs );
Concatenates two JSON Pointers. Effectively returns basic_address<CharT>(lhs) += rhs.

    bool operator==(const basic_address<CharT>& lhs, const basic_address<CharT>& rhs);

    bool operator!=(const basic_address<CharT>& lhs, const basic_address<CharT>& rhs);

    std::basic_ostream<CharT>&
    operator<<(std::basic_ostream<CharT>& os, const basic_address<CharT>& p);
Performs stream output

### Examples

#### Iterate over the tokens in a JSON Pointer

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

namespace jp = jsoncons::jsonpointer;

int main()
{
    jp::pointer p("/store/book/1/author");

    std::cout << "(1) " << p << "\n\n";

    std::cout << "(2)\n";
    for (const auto& token : p)
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

namespace jp = jsoncons::jsonpointer;

int main()
{
    jp::pointer p;

    p /= "a/b";
    p /= "";
    p /= "m~n";

    std::cout << "(1) " << p << "\n\n";

    std::cout << "(2)\n";
    for (const auto& token : p)
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

#### Concatentate two JSON Pointers

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

namespace jp = jsoncons::jsonpointer;

int main()
{
    jp::pointer p("/a~1b");

    p += jp::pointer("//m~0n");

    std::cout << "(1) " << p << "\n\n";

    std::cout << "(2)\n";
    for (const auto& token : p)
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

