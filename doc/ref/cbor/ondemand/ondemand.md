### jsoncons::cbor::ondemand

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>
```

The `cbor::ondemand` namespace reads the encoded structure of
[Concise Binary Object Representation](http://cbor.io/) data in place,
without copying it or building a data structure. Scanning validates an item's
structure once and yields a checked `cbor_item`; everything after that is
zero-copy and cannot fail.

### Classes
<table border="0">
  <tr>
    <td><a href="cbor_item.md">cbor_item</a></td>
    <td>Represents one complete, structurally valid encoding.</td> 
  </tr>
</table>
 
### Functions

<table border="0">
  <tr>
    <td><a href="read_item.md">read_item</br>read_item_sequence</a></td>
    <td>Returns a CBOR item or sequence of items.</td> 
  </tr>
</table>

### Example

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>
#include <iostream>
#include <vector>

namespace cbor = jsoncons::cbor;

int main()
{
    // {"id": 42, "scores": [1, 2]}
    const std::vector<uint8_t> data = {
        0xa2,
        0x62, 'i', 'd', 0x18, 0x2a,
        0x66, 's', 'c', 'o', 'r', 'e', 's', 0x82, 0x01, 0x02
    };

    auto item = cbor::ondemand::read_item(jsoncons::span<const uint8_t>(data));
    if (!item)
    {
        std::cout << "error " << item.error().message() << "\n";
        return 1;
    }

    bool is_key = true;
    std::string_view key;
    for (const cbor::ondemand::cbor_item child : item.value().children())
    {
        if (is_key)
        {
            key = child.get_string().value();
        }
        else if (child.type() == cbor::ondemand::cbor_type::array)
        {
            std::cout << key << ":";
            for (const cbor::ondemand::cbor_item element : child.children())
            {
                uint64_t value = element.get_uint64().value();
                std::cout << " " << value;
            }
            std::cout << "\n";
        }
        else
        {
            uint64_t value = child.get_uint64().value();
            std::cout << key << ": " << value << "\n";
        }
        is_key = !is_key;
    }
    return 0;
}
```

Output:

```
id: 42
scores: 1 2
```
