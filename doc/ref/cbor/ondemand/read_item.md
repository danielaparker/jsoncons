### jsoncons::cbor::ondemand::read_item, jsoncons::cbor::ondemand::read_item_sequence

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>

expected<cbor_item, read_error> read_item(
    span<const uint8_t> input,                             (1)
    int max_nesting_depth = default_max_nesting_depth);

expected<cbor_sequence, read_error> read_sequence(
    span<const uint8_t> input,                             (2)
    int max_nesting_depth = default_max_nesting_depth);
```

(1) If successful, returns an [cbor_item](cbor_item.md), otherwise a [read_error](read_error.md).

(2) If successful, returns a `cbor_sequence` defined as

```cpp
struct cbor_sequence
{
    cbor_item head;                                        (3)
    span<const uint8_t> tail;
};
```
, otherwise a [read_error](read_error.md).

`read_sequence` checks the first item in `input` and returns it with the bytes that
follow, so a CBOR sequence can be read by reading the remaining bytes repeatedly.
`read_item` requires `input` to hold exactly one item; trailing bytes produce
`cbor_errc::trailing_data`. Errors report the position at which scanning
stopped. Scanning allocates only when nesting exceeds 32 open containers.

Structural validation covers CBOR framing, lengths, container balance and the
configured nesting limit. It validates UTF-8 once when first accessed,
deterministic encoding, tag semantics, duplicate map keys or application
schemas. Tags are exposed, never interpreted.

An `cbor_item` borrows the scanned bytes. Destroying, mutating or reallocating
those bytes invalidates the item, its children, and any spans or string views
obtained from it. `read_sequence` and `read_item` reject temporary owning containers
at compile time, but an explicitly constructed span remains the caller's
lifetime responsibility.


