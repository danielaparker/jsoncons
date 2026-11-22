### jsoncons::cbor::ondemand

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>
```

The `cbor::ondemand` namespace reads the encoded structure of
[Concise Binary Object Representation](http://cbor.io/) data in place,
without copying it or building a data structure. Scanning validates an item's
structure once and yields a checked `item`; everything after that is
zero-copy and cannot fail.

### Classes
<table border="0">
  <tr>
    <td><a href="item.md">item</a></td>
    <td>Represents one complete, structurally valid encoding.</td> 
  </tr>
</table>

### Functions

```cpp
struct read_error
{
    cbor_errc code;
    std::size_t position;
};

struct read_result
{
    item first;
    span<const uint8_t> remainder;
};

expected<read_result, read_error> read(
    span<const uint8_t> input,
    int max_nesting_depth = default_max_nesting_depth);

expected<item, read_error> read_item(
    span<const uint8_t> input,
    int max_nesting_depth = default_max_nesting_depth);
```

`read` checks the first item in `input` and returns it with the bytes that
follow, so a CBOR sequence can be read by scanning the remainder repeatedly.
`read_item` requires `input` to hold exactly one item; trailing bytes produce
`cbor_errc::trailing_data`. Errors report the position at which scanning
stopped. Scanning allocates only when nesting exceeds 32 open containers.

Structural validation covers CBOR framing, lengths, container balance and the
configured nesting limit. It does not validate UTF-8 (see `validate_text`),
deterministic encoding, tag semantics, duplicate map keys or application
schemas. Tags are exposed, never interpreted.

#### Ownership and lifetime

An `item` borrows the scanned bytes. Destroying, mutating or reallocating
those bytes invalidates the item, its children, and any spans or string views
obtained from it. `read` and `read_item` reject temporary owning containers
at compile time, but an explicitly constructed span remains the caller's
lifetime responsibility.

### Example

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>
#include <iostream>
#include <vector>

namespace ondemand = jsoncons::cbor::ondemand;

int main()
{
    // {"id": 42, "scores": [1, 2]}
    const std::vector<uint8_t&gt; data = {
        0xa2,
        0x62, 'i', 'd', 0x18, 0x2a,
        0x66, 's', 'c', 'o', 'r', 'e', 's', 0x82, 0x01, 0x02
    };

    auto parsed = ondemand::read_item(jsoncons::span<const uint8_t&gt;(data));
    if (!parsed)
    {
        std::cout << "error " << static_cast<int>(parsed.error().code)
            << " at offset " << parsed.error().offset << "\n";
        return 1;
    }

    bool is_key = true;
    std::string_view key;
    for (const ondemand::item child : parsed.value().children())
    {
        if (is_key)
        {
            key = child.get_string().value();
        }
        else if (child.type() == ondemand::item_type::array)
        {
            std::cout << key << ":";
            for (const ondemand::item element : child.children())
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
}
```

Output:

```
id: 42
scores: 1 2
```
