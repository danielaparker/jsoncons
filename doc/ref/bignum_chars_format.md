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

```c++
#include <jsoncons/json.hpp>

using namespace jsoncons;

int main()
{
    json j(bignum("-18446744073709551617"));

    std::cout << "(1)\n";
    j.dump(std::cout);
    std::cout << "\n\n";

    std::cout << "(2)\n";
    json_serializing_options options2;
    options2.bignum_format(bignum_chars_format::integer);
    j.dump(std::cout, options2);
    std::cout << "\n\n";

    std::cout << "(3)\n";
    json_serializing_options options3;
    options3.bignum_format(bignum_chars_format::base64url);
    j.dump(std::cout, options3);
    std::cout << "\n\n";
}
```
Output:
```
(1)
"-18446744073709551617"

(2)
-18446744073709551617

(3)
"~AQAAAAAAAAAB"
```

