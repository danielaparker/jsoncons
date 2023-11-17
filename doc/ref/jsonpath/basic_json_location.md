### jsoncons::jsonpath::basic_json_location

```cpp
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

template <class CharT>
class basic_json_location
```

Two specializations for common character types are defined:

Type      |Definition
----------|------------------------------
json_location   |`basic_json_location<char>` (since 0.172.0)
wjson_location  |`basic_json_location<wchar_t>` (since 0.172.0)

Objects of type `basic_json_location` represent a normalized path.

#### Member types
Type        |Definition
------------|------------------------------
char_type   | `CharT`
string_type | `std::basic_string<char_type>`
string_view_type | `jsoncons::basic_string_view<char_type>`
const_iterator | A constant [LegacyRandomAccessIterator](https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator) with a `value_type` of `std::basic_string<char_type>`
iterator    | An alias to `const_iterator`

#### Constructors

    basic_json_location();                                      (1)

    explicit basic_json_location(const string_view_type& str);  

    explicit basic_json_location(const string_view_type& str, 
                                std::error_code& ec);          (2)

    basic_json_location(const basic_json_location&);             (3)

    basic_json_location(basic_json_location&&) noexcept;         (4)

(1) Constructs an empty `basic_json_location`.

(2) Constructs a `basic_json_location` from a string representation or a 
URI fragment identifier (starts with `#`).

#### operator=

    basic_json_location& operator=(const basic_json_location&);

    basic_json_location& operator=(basic_json_location&&);

#### Modifiers

    basic_json_location& operator/=(const string_type& s)
Appends the token s.

    template <class IntegerType>
    basic_json_location& operator/=(IntegerType index) 
Appends the token `index`.
This overload only participates in overload resolution if `IntegerType` is an integer type.

    basic_json_location& operator+=(const basic_json_location& ptr)
Concatenates the current pointer and the specified pointer `ptr`. 

#### Iterators

    iterator begin() const;
    iterator end() const;
Iterator access to the tokens in the pointer.

#### Accessors

    bool empty() const
Checks if the pointer is empty

   string_type to_string() const
Returns a normalized path represented as a string value, escaping any `/` or `~` characters.

   string_type to_uri_fragment() const
Returns a string representing the normalized path as a URI fragment identifier, 
escaping any `/` or `~` characters.


#### Static member functions

   static parse(const string_view_type& str);
   static parse(const string_view_type& str, std::error_code& ec);
Constructs a `basic_json_location` from a string representation or a 
URI fragment identifier (starts with `#`).

#### Non-member functions
    basic_json_location<CharT> operator/(const basic_json_location<CharT>& lhs, const basic_string<CharT>& s);
Concatenates a normalized path pointer and a string. Effectively returns basic_json_location<CharT>(lhs) /= s.

    template <class CharT,class IntegerType>
    basic_json_location<CharT> operator/(const basic_json_location<CharT>& lhs, IntegerType index);
Concatenates a normalized path pointer and an index. Effectively returns basic_json_location<CharT>(lhs) /= index.
This overload only participates in overload resolution if `IntegerType` is an integer type.

    template <class CharT,class IntegerType>
    basic_json_location<CharT> operator+( const basic_json_location<CharT>& lhs, const basic_json_location<CharT>& rhs );
Concatenates two normalized paths. Effectively returns basic_json_location<CharT>(lhs) += rhs.

    template <class CharT,class IntegerType>
    bool operator==(const basic_json_location<CharT>& lhs, const basic_json_location<CharT>& rhs);

    template <class CharT,class IntegerType>
    bool operator!=(const basic_json_location<CharT>& lhs, const basic_json_location<CharT>& rhs);

    template <class CharT,class IntegerType>
    std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const basic_json_location<CharT>& ptr);
Performs stream output


