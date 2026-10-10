### jsoncons::cbor::view

```cpp
#include <jsoncons_ext/cbor/cbor_view.hpp>
```

<br>

The `cbor::view` namespace reads the encoded structure of
[Concise Binary Object Representation](http://cbor.io/) data in place,
without copying it or building a data structure. Scanning validates an item's
structure once and yields a checked `item`; everything after that is
zero-copy and cannot fail.

Structural validation covers CBOR framing, lengths, container balance and the
configured nesting limit. It does not validate UTF-8 (see `validate_text`),
deterministic encoding, tag semantics, duplicate map keys or application
schemas. Tags are exposed, never interpreted.

This namespace is experimental.

#### Ownership and lifetime

An `item` borrows the scanned bytes. Destroying, mutating or reallocating
those bytes invalidates the item, its children, and any spans or string views
obtained from it. `scan` and `parse_item` reject temporary owning containers
at compile time, but an explicitly constructed span remains the caller's
lifetime responsibility.

#### Scanning

```cpp
struct scan_error
{
    cbor_errc code;
    std::size_t offset;
};

struct scan_result
{
    item first;
    span<const uint8_t> remainder;
};

expected<scan_result, scan_error> scan(
    span<const uint8_t> input,
    int max_nesting_depth = default_max_nesting_depth);

expected<item, scan_error> parse_item(
    span<const uint8_t> input,
    int max_nesting_depth = default_max_nesting_depth);
```

`scan` checks the first item in `input` and returns it with the bytes that
follow, so a CBOR sequence can be read by scanning the remainder repeatedly.
`parse_item` requires `input` to hold exactly one item; trailing bytes produce
`cbor_errc::trailing_data`. Errors report the offset at which scanning
stopped. Scanning allocates only when nesting exceeds 32 open containers.

#### Checked items

```cpp
enum class item_kind
{
    unsigned_integer, negative_integer, byte_string, text_string,
    array, map, simple
};

class item
{
public:
    span<const uint8_t> encoded_bytes() const noexcept;
    item_kind kind() const noexcept;
    uint64_t argument() const noexcept;
    bool indefinite() const noexcept;

    tag_range tags() const noexcept;
    child_range children() const noexcept;

    bool uint64_value(uint64_t&) const noexcept;
    bool int64_value(int64_t&) const noexcept;
    bool bool_value(bool&) const noexcept;
    bool double_value(double&) const noexcept;
    bool text(string_view&) const noexcept;
    bool bytes(span<const uint8_t>&) const noexcept;
    bool text(std::string&) const;
    bool bytes(std::vector<uint8_t>&) const;
};
```

An `item` is one complete, structurally valid encoding. `encoded_bytes`
includes its leading tags; `kind` and `argument` describe the untagged head,
whose argument is an integer's value, a string's length, a container's count,
or a simple value's number (the bit pattern of a float). `tags` yields the
leading tag numbers, outermost first.

The typed accessors return `false`, leaving their destination unchanged, on a
kind or range mismatch. `text(string_view&)` and `bytes(span&)` view
definite-length content in place. The copying overloads also assemble
indefinite-length strings, and leave their destination unchanged on failure.

`children` yields the data items an item is made of, as checked items: an
array's elements, a map's keys and values alternating, or the definite-length
chunks of an indefinite-length string (RFC 8949 section 3.2.3). It is empty
for anything else. Each child is measured once as the iteration reaches it;
the last child of a definite-length container is not measured at all, since
it ends where its parent does.

```cpp
bool validate_text(const item&) noexcept;
```

`validate_text` checks that a text item is well-formed UTF-8. Each chunk of an
indefinite-length text string is checked independently, as RFC 8949 section
3.2.3 requires.

### Example

```cpp
#include <jsoncons_ext/cbor/cbor_view.hpp>
#include <iostream>
#include <vector>

namespace view = jsoncons::cbor::view;

int main()
{
    // {"id": 42, "scores": [1, 2]}
    const std::vector<uint8_t> data = {
        0xa2,
        0x62,'i','d', 0x18,0x2a,
        0x66,'s','c','o','r','e','s', 0x82,0x01,0x02
    };

    auto parsed = view::parse_item(jsoncons::span<const uint8_t>(data));
    if (!parsed)
    {
        std::cout << "error " << static_cast<int>(parsed.error().code)
                  << " at offset " << parsed.error().offset << "\n";
        return 1;
    }

    bool is_key = true;
    jsoncons::string_view key;
    for (const view::item child : parsed.value().children())
    {
        if (is_key)
        {
            child.text(key);
        }
        else if (child.kind() == view::item_kind::array)
        {
            std::cout << key << ":";
            for (const view::item element : child.children())
            {
                uint64_t value = 0;
                element.uint64_value(value);
                std::cout << " " << value;
            }
            std::cout << "\n";
        }
        else
        {
            uint64_t value = 0;
            child.uint64_value(value);
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
