### jsoncons::jmespath::jmespath_expression

```c++
#include <jsoncons_ext/jmespath/jmespath.hpp>

template <class Json>
class jmespath_expression
```

#### Static functions

    static jmespath_expression compile(const string_view_type& expr); // (1)

    static jmespath_expression compile(const string_view_type& expr,
                                       std::error_code& ec); //(2)

Checks if the pointer is empty

    const string_view_type& string_view() const
    operator string_view_type() const;
Access the JSON Pointer pointer as a string view.

#### Non-member functions
    jmespath_expression<CharT> operator/(const jmespath_expression<CharT>& lhs, const string_type& rhs);
Concatenates a JSON Pointer pointer and a token. Effectively returns jmespath_expression<CharT>(lhs) /= rhs.

    jmespath_expression<CharT> operator+( const jmespath_expression<CharT>& lhs, const jmespath_expression<CharT>& rhs );
Concatenates two JSON Pointers. Effectively returns jmespath_expression<CharT>(lhs) += rhs.

    bool operator==(const jmespath_expression<CharT>& lhs, const jmespath_expression<CharT>& rhs);

    bool operator!=(const jmespath_expression<CharT>& lhs, const jmespath_expression<CharT>& rhs);

    std::basic_ostream<CharT>&
    operator<<(std::basic_ostream<CharT>& os, const jmespath_expression<CharT>& ptr);
Performs stream output

### Examples

#### Iterate over the tokens in a JSON Pointer

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>

// for brevity
namespace jmespath = jsoncons::jmespath;

int main()
{
    jmespath::json_ptr ptr("/store/book/1/author");

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
#include <jsoncons_ext/jmespath/jmespath.hpp>

namespace jmespath = jsoncons::jmespath;

int main()
{
    jmespath::json_ptr ptr;

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
#include <jsoncons_ext/jmespath/jmespath.hpp>

namespace jmespath = jsoncons::jmespath;

int main()
{
    jmespath::json_ptr ptr("/a~1b");

    ptr += jmespath::json_ptr("//m~0n");

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

