### jsoncons::bigint_chars_format

```cpp
#include <jsoncons/json_options.hpp>

enum class bigint_chars_format : uint8_t {number, base10, base64, base64url};
```

Specifies `bignum` formatting. 

### Examples

#### Initializing with bigint

```cpp
#include <jsoncons/json.hpp>

using namespace jsoncons;

int main()
{
    std::string s = "-18446744073709551617";

    json j(bigint::from_string(s.c_str()));

    std::cout << "(default) ";
    j.dump(std::cout);
    std::cout << "\n\n";

    std::cout << "(integer) ";
    auto options1 = json_options{}
        .bigint_format(bigint_chars_format::number);
    j.dump(std::cout, options1);
    std::cout << "\n\n";

    std::cout << "(base64) ";
    auto options3 = json_options{}
        .bigint_format(bigint_chars_format::base64);
    j.dump(std::cout, options3);
    std::cout << "\n\n";

    std::cout << "(base64url) ";
    auto options4 = json_options{}
        .bigint_format(bigint_chars_format::base64url);
    j.dump(std::cout, options4);
    std::cout << "\n\n";
}
```
Output:
```
(default) "-18446744073709551617"

(integer) -18446744073709551617

(base64) "~AQAAAAAAAAAA"

(base64url) "~AQAAAAAAAAAA"
```

#### Integer overflow during parsing

```cpp
#include <jsoncons/json.hpp>

using namespace jsoncons;

int main()
{
    std::string s = "-18446744073709551617";

    json j = json::parse(s);

    std::cout << "(1) ";
    j.dump(std::cout);
    std::cout << "\n\n";

    std::cout << "(2) ";
    auto options1 = json_options{}
        .bigint_format(bigint_chars_format::number);
    j.dump(std::cout, options1);
    std::cout << "\n\n";

    std::cout << "(3) ";
    auto options2 = json_options{}
        .bigint_format(bigint_chars_format::base64url);
    j.dump(std::cout, options2);
    std::cout << "\n\n";
}
```
Output:
```
(1) "-18446744073709551617"

(2) -18446744073709551617

(3) "~AQAAAAAAAAAB"
```

