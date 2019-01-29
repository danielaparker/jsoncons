### jsoncons::jsonpointer::basic_path

```
template <class CharT>
class basic_path
```
#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
```

Two specializations for common character types are defined:

Type      |Definition
----------|------------------------------
path      |`basic_path<char>`
wpath     |`basic_path<wchar_t>`

Objects of type `basic_path` represent JSON Pointer paths.

#### Member types
Type        |Definition
------------|------------------------------
char_type   | `CharT`
string_type | `std::basic_string<char_type>`
string_view_type | `jsoncons::basic_string_view<char_type>`
const_iterator | A constant [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) with a `value_type` of `std::basic_string<char_type>`
iterator    | An alias to `const_iterator`

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>


#### Constructors

    basic_path();

    basic_path(const string_type& path);

    basic_path(string_type&& path);

    basic_path(const CharT* s);

    basic_path(const basic_path&);

    basic_path(basic_path&&);

#### operator=

    basic_path& operator=(const basic_path&);

    basic_path& operator=(basic_path&&);

#### Iterators

    iterator begin() const;
    iterator end() const;
Iterator access to the path.

#### Modifiers

    basic_path& append(const string_type& s);
First, appends the JSON Pointer separator `/` to the path. Then appends s, escaping any `/` or `~` characters.

#### Accessors

    bool empty() const
Checks if the path is empty

    const string_view_type& string_view() const
    operator string_view_type() const;
Access the JSON Pointer path as a string view.

    std::basic_ostream<CharT>&
    operator<<(std::basic_ostream<CharT>& os, const basic_path<CharT>& p);

