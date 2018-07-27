### jsoncons::bignum_chars_format

```c++
enum class bignum_chars_format : uint8_t {integer, base10, base64, base64url};
```

#### Header
```c++
#include <jsoncons/json_serializing_options.hpp>
```

Specifies `bignum` formatting. 

### Examples

#### Initializing with bignum

```c++
#include <jsoncons/json.hpp>

using namespace jsoncons;

int main()
{
    std::string s = "-18446744073709551617";

    json j(bignum(s.c_str()));

    std::cout << "(default) ";
    j.dump(std::cout);
    std::cout << "\n\n";

    std::cout << "(integer) ";
    json_serializing_options options1;
    options1.bignum_format(bignum_chars_format::integer);
    j.dump(std::cout, options1);
    std::cout << "\n\n";

    std::cout << "(base64) ";
    json_serializing_options options3;
    options3.bignum_format(bignum_chars_format::base64);
    j.dump(std::cout, options3);
    std::cout << "\n\n";

    std::cout << "(base64url) ";
    json_serializing_options options4;
    options4.bignum_format(bignum_chars_format::base64url);
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

```c++
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
    json_serializing_options options1;
    options1.bignum_format(bignum_chars_format::integer);
    j.dump(std::cout, options1);
    std::cout << "\n\n";

    std::cout << "(3) ";
    json_serializing_options options2;
    options2.bignum_format(bignum_chars_format::base64url);
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

