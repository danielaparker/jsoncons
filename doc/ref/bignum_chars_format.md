### jsoncons::bignum_chars_format

```c++
enum class bignum_chars_format : uint8_t {integer, string, base64url};
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

    std::cout << "(1) " << j.as<bignum>() << "\n\n";

    std::cout << "(2) " << j.as<std::string>() << "\n\n";

    std::cout << "(3) ";
    j.dump(std::cout);
    std::cout << "\n\n";

    std::cout << "(4) ";
    json_serializing_options options1;
    options1.bignum_format(bignum_chars_format::integer);
    j.dump(std::cout, options1);
    std::cout << "\n\n";

    std::cout << "(5) ";
    json_serializing_options options2;
    options2.bignum_format(bignum_chars_format::base64url);
    j.dump(std::cout, options2);
    std::cout << "\n\n";
}
```
Output:
```
(1) -18446744073709551617

(2) -18446744073709551617

(3) "-18446744073709551617"

(4) -18446744073709551617

(5) "~AQAAAAAAAAAB"
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

