### jsoncons::cbor::view

```cpp
#include <jsoncons_ext/cbor/cbor_view.hpp>
```

<br>

The `cbor::view` namespace reads the encoded structure of
[Concise Binary Object Representation](http://cbor.io/) data in place,
without copying it or building a data structure. It provides two orthogonal
forms of access:

- `wire_cursor` for head-level sequential decoding;
- `walker` for incremental traversal and validation.

Structural validation covers CBOR framing, lengths, container balance and the
configured nesting limit. It does not validate UTF-8, deterministic encoding,
tag semantics, duplicate map keys or application schemas. Tags are exposed,
never interpreted.

This namespace is experimental.

#### Ownership and lifetime

Every view borrows the input bytes. Destroying, mutating or reallocating those
bytes invalidates its items, walkers, spans and string views. View-producing
entry points reject temporary owning containers at compile time, but an
explicitly constructed span remains the caller's lifetime responsibility.

#### Incremental traversal

```cpp
struct scan_error
{
    cbor_errc code;
    std::size_t offset;
};

class walk_result
{
public:
    bool has_value() const noexcept;
    bool value() const noexcept;
    const scan_error& error() const noexcept;
};

enum class position_role
{
    root, array_element, map_key, map_value
};

expected<walker, scan_error> get_walker(
    span<const uint8_t> input,
    int max_nesting_depth = default_max_nesting_depth) noexcept;

class walker
{
public:
    walker(walker&&) noexcept;
    walker& operator=(walker&&) noexcept;
    walker(const walker&) = delete;

    item_kind kind() const noexcept;
    uint64_t argument() const noexcept;
    bool indefinite() const noexcept;
    tag_range tags() const noexcept;
    position_role role() const noexcept;
    std::size_t depth() const noexcept;

    bool uint64_value(uint64_t&) const noexcept;
    bool int64_value(int64_t&) const noexcept;
    bool bool_value(bool&) const noexcept;
    bool double_value(double&) const noexcept;
    bool text(string_view&) const noexcept;
    bool bytes(span<const uint8_t>&) const noexcept;

    walk_result enter() noexcept;
    walk_result next() noexcept;
    walk_result leave() noexcept;
    expected<item, scan_error> finish_item() noexcept;

    void rewind() noexcept;
    bool extent_known() const noexcept;
};
```

`get_walker` reads the root's tags and immediate head. It does not scan the
root's descendants or reject bytes following the first item. A definite string
payload is bounds-checked before a zero-copy view can be exposed.

Movement has three outcomes. An error reports malformed input or a resource
failure. A successful `true` means movement occurred. A successful `false`
means there was nowhere to move:

- `enter`: the current value has no child;
- `next`: the current value has no next sibling;
- `leave`: the current value is the root.

`walk_result` deliberately has no boolean conversion. Inspect `has_value()` for
an error and `value()` to distinguish movement from a successful end.

Maps expose keys and values as alternating raw children. `next` validates only
the current subtree needed to locate its sibling. Calling `leave` before the end
of a container validates and skips its unread remainder. A failed operation
does not change the walker's logical position.

The current head and definite scalar/string content are already available.
`finish_item` validates and measures the complete current subtree, caches its
end, and returns it as a checked `item`. `extent_known` reports whether that
work is already complete. `rewind` returns to the root without rescanning it.

The walker owns its parent frames and reusable skip workspace. Parent-frame and
deep-scan capacity is retained, so traversal is amortized allocation-free.
With exceptions enabled, allocation failures are returned as
`cbor_errc::source_error`. No walker operation throws; exception-disabled builds
follow jsoncons's process-termination policy on allocation failure.

#### Eager validation

```cpp
expected<item, scan_error> validate(
    span<const uint8_t> input,
    int max_nesting_depth = default_max_nesting_depth) noexcept;
```

`validate` requires the supplied span to contain exactly one structurally valid
item. It is a convenience wrapper over `get_walker` and `finish_item`; trailing
bytes produce `cbor_errc::trailing_data`.

```cpp
auto made = get_walker(input);
if (!made)
{
    return;
}
auto walker = std::move(made.value());
auto checked = walker.finish_item();
if (!checked || checked.value().encoded_bytes().size() != input.size())
{
    return;
}
walker.rewind();
```

This validates and traverses with the same walker. The root extent and reusable
skip workspace remain cached after `rewind`; no second traversal engine or
checked traversal type is required.

#### Checked items

```cpp
class item
{
    span<const uint8_t> encoded_bytes() const noexcept;
    item_kind kind() const noexcept;
    uint64_t argument() const noexcept;
    bool indefinite() const noexcept;
    tag_range tags() const noexcept;

    chunk_range chunks() const noexcept;
    child_range children(scan_context& context) const noexcept;

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

An `item` is one complete structurally valid encoding. Its leading tags are
included in `encoded_bytes`; `kind` and `argument` describe the untagged head.
Strict typed accessors leave their destination unchanged on a kind or range
mismatch. Zero-copy string access requires definite-length content. `chunks`
exposes definite content as one span and indefinite content as one span per
chunk. The copying overloads assemble chunked strings transactionally.

`children` yields array elements or alternating map keys and values as complete
checked items. Its context supplies reusable skip workspace and must outlive
the range and its iterators.

```cpp
bool validate_text(const item&) noexcept;
```

`validate_text` checks a text item's UTF-8. Each chunk of an indefinite text
string is checked independently as required by RFC 8949 section 3.2.3.

#### Scanning checked items

```cpp
class scan_context
{
public:
    scan_context() noexcept;
    explicit scan_context(int max_nesting_depth) noexcept;
    int max_nesting_depth() const noexcept;
};

struct scan_result
{
    item first;
    span<const uint8_t> remainder;
};

expected<scan_result, scan_error> scan(span<const uint8_t> input);
expected<scan_result, scan_error> scan(
    span<const uint8_t> input, scan_context& context);

expected<item, scan_error> parse_item(span<const uint8_t> input);
expected<item, scan_error> parse_item(
    span<const uint8_t> input, scan_context& context);
```

`scan` checks the first item and returns the remaining bytes. `parse_item`
requires exactly one item. A reusable context retains deep-scan capacity.

#### Deterministic encoding orders

```cpp
struct bytewise_compare;
struct length_first_compare;
struct bytewise_less;
struct length_first_less;

template <typename Order = bytewise_compare>
expected<int, scan_error> compare(
    span<const uint8_t> a, span<const uint8_t> b,
    Order order = Order(),
    int max_nesting_depth = default_max_nesting_depth);

template <typename Order = bytewise_compare>
expected<bool, scan_error> map_keys_sorted(
    span<const uint8_t> input,
    Order order = Order(),
    int max_nesting_depth = default_max_nesting_depth);
```

Bytewise order implements RFC 8949 section 4.2.1; length-first order implements
section 4.2.3. Span entry points validate the first item and tolerate trailing
bytes. Item overloads operate directly on checked encodings.

#### Low-level cursor

```cpp
class wire_cursor
{
public:
    explicit wire_cursor(span<const uint8_t> input) noexcept;

    std::size_t position() const noexcept;
    span<const uint8_t> remaining() const noexcept;
    expected<item_head, scan_error> read_head() noexcept;
    expected<item, scan_error> read_item(scan_context&) noexcept;
    expected<span<const uint8_t>, scan_error> skip_item(scan_context&) noexcept;
    bool skip(std::size_t count) noexcept;
};
```

`read_head` advances past one head and exposes tags individually. `read_item`
and `skip_item` validate one complete item. `skip` advances over an already
measured payload. Errors carry offsets from the beginning of the cursor input.

### Example

```cpp
const std::vector<uint8_t> data = {
    0xa2,
    0x62,'i','d', 0x18,0x2a,
    0x66,'s','c','o','r','e','s', 0x82,0x01,0x02
};

auto made = jsoncons::cbor::view::get_walker(
    jsoncons::span<const uint8_t>(data));
if (!made)
{
    return;
}

auto walker = std::move(made.value());
auto entered = walker.enter();
if (!entered.has_value() || !entered.value())
{
    return;
}

for (;;)
{
    jsoncons::string_view key;
    if (!walker.text(key))
    {
        return;
    }

    auto value = walker.next();
    if (!value.has_value() || !value.value())
    {
        return;
    }

    handle(key, walker);

    auto next = walker.next();
    if (!next.has_value() || !next.value())
    {
        break;
    }
}
```
