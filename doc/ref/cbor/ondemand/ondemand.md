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
struct scan_error
{
    cbor_errc code;
    std::size_t position;
};

struct scan_result
{
    item first;
    span<const uint8_t> remainder;
};

expected<scan_result, scan_error> scan(
    span<const uint8_t> input,
    int max_nesting_depth = default_max_nesting_depth);

expected<item, scan_error> read_item(
    span<const uint8_t> input,
    int max_nesting_depth = default_max_nesting_depth);
```

`scan` checks the first item in `input` and returns it with the bytes that
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
obtained from it. `scan` and `read_item` reject temporary owning containers
at compile time, but an explicitly constructed span remains the caller's
lifetime responsibility.

