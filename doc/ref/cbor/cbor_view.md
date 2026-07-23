### jsoncons::cbor::view

```cpp
#include <jsoncons_ext/cbor/cbor_view.hpp>
```

<br>

The `cbor::view` namespace reads the *encoded* structure of
[Concise Binary Object Representation](http://cbor.io/) data in place,
without copying it or building a data structure. Its spine is four stages:

> owned bytes -> structural validation -> borrowed checked items -> application semantics

A scan validates that bytes are *structurally* well-formed CBOR once
(framing only — not validity such as UTF-8); everything afterwards
operates on checked `item` views and cannot fail structurally. The last
stage — what tag 2 bytes or a tag 25 string reference *mean* — belongs to
[decode_cbor](decode_cbor.md) and [basic_cbor_cursor](basic_cbor_cursor.md),
not here: tags are exposed, never interpreted.

This namespace is experimental.

#### Ownership and lifetime

Everything in this namespace borrows. An `item`, the items produced by
iterating it, and every span and string view obtained from them point
into the scanned input bytes, and are invalidated by anything that
invalidates those bytes: destroying the container, mutating it, or any
reallocation. Scanning a temporary container is rejected at compile
time. The copying accessors (`text` into `std::string`, `bytes` into
`std::vector`) are the way to keep content beyond the input's lifetime.

#### Scanning

```cpp
class scan_context;                    // depth policy + reusable workspace

struct scan_error
{
    cbor_errc code;
    std::size_t offset;                // bytes consumed when detected
};

struct scan_result
{
    item first;
    span<const uint8_t> remainder;
};

expected<scan_result, scan_error> scan_prefix(span<const uint8_t> input);
expected<scan_result, scan_error> scan_prefix(span<const uint8_t> input, scan_context& context);

expected<item, scan_error> parse_exact(span<const uint8_t> input);
expected<item, scan_error> parse_exact(span<const uint8_t> input, scan_context& context);
```

`scan_prefix` validates and returns the first item in `input`, with the
bytes that follow it; scan a stream of items by looping on `remainder`.
`parse_exact` requires `input` to be exactly one item, and reports
anything after it as `cbor_errc::trailing_data`.

Scanning enforces `scan_context::max_nesting_depth` (default
`default_max_nesting_depth`, 1024) using constant call-stack space at
any depth. It allocates only when nesting exceeds 32 open containers;
a reused `scan_context` retains that capacity across scans.

#### The checked item

```cpp
class item
{
    span<const uint8_t> encoded_bytes() const noexcept;  // tags included
    item_kind kind() const noexcept;                     // of the untagged content
    uint64_t argument() const noexcept;                  // RFC 8949 head argument
    bool indefinite() const noexcept;
    tag_range tags() const noexcept;                     // leading tags, outermost first

    element_range elements() const noexcept;             // array elements
    entry_range entries() const noexcept;                // map entries
    chunk_range chunks() const noexcept;                 // string content spans

    bool uint64_value(uint64_t& value) const noexcept;
    bool int64_value(int64_t& value) const noexcept;
    bool bool_value(bool& value) const noexcept;
    bool double_value(double& value) const noexcept;
    bool text(string_view& value) const noexcept;        // definite only, zero copy
    bool bytes(span<const uint8_t>& value) const noexcept;
    bool text(std::string& value) const;                 // assembles chunks
    bool bytes(std::vector<uint8_t>& value) const;
};

enum class item_kind
{
    unsigned_integer, negative_integer, byte_string, text_string,
    array, map, simple
};

struct map_entry
{
    item key;
    item value;
};
```

An `item` is one complete, well-formed encoded item: its leading
semantic tags, head, and content. `kind()` classifies the content after
tags (`simple` covers major type 7: simple values and floating point);
`argument()` is the head's argument — an integer's value, a string's
length, a container's count, a simple value's number, or the bit
pattern of a floating-point value.

The ranges iterate with plain range-`for` and cannot fail: validation
already happened. `elements()` and `entries()` yield checked items;
`chunks()` yields the contiguous spans of a string's content, one per
chunk for indefinite-length strings. Kind mismatches yield empty
ranges. Iterating a container walks its encoding, so navigating to
depth *d* of a document rescans the subtrees on that path; iteration
beyond 32 levels of nesting inside one item may allocate.

The typed accessors return `false`, leaving `value` untouched, exactly
when the item is not of the requested kind; conversions are strict.
The copying `text` and `bytes` overloads are transactional — `value`
is replaced on success and untouched on failure — and `value` may
alias the scanned bytes themselves. Tags are never silently dropped:
they are visible on every item via `tags()`, and reading the value of
a tagged item is the caller's explicit decision.

```cpp
bool validate_text(const item& text_item) noexcept;
```

True if `text_item` is a text string whose content is well-formed
UTF-8; each chunk of an indefinite-length text string must itself be
well-formed (RFC 8949 3.2.3). No other function in this namespace
validates text content.

#### Deterministic encoding orders

```cpp
struct bytewise_compare;       // RFC 8949 4.2.1 order, three-way
struct length_first_compare;   // RFC 8949 4.2.3 order, three-way
struct bytewise_less;          // strict weak orders for sorting
struct length_first_less;

template <typename Compare = bytewise_compare>
bool map_keys_sorted(const item& map_item, Compare compare = Compare());

// Span overloads for raw-span consumers; validate input, then order.
template <typename Order = bytewise_compare>
expected<int, scan_error> compare(span<const uint8_t> a, span<const uint8_t> b,
    Order order = Order(), int max_nesting_depth = default_max_nesting_depth);

template <typename Order = bytewise_compare>
expected<bool, scan_error> map_keys_sorted(span<const uint8_t> input,
    Order order = Order(), int max_nesting_depth = default_max_nesting_depth);
```

The order function objects compare encoded bytes and accept either two
items or two raw `span<const uint8_t>`. The `*_compare` forms return
negative, zero, or positive; the `*_less` forms are predicates for
`std::sort` and ordered containers. `map_keys_sorted` is true if
`map_item` is a map whose keys are strictly ascending in the given
order — the deterministic-encoding key condition. The span overloads
validate before ordering and, on malformed input, return an error carrying
the code and byte offset; trailing bytes after the first item are tolerated.

#### Low-level tier

```cpp
enum class major_type;   // CBOR major types 0-7

struct item_head { major_type major_type; uint8_t additional_info; uint64_t value; bool indefinite(); };

bool read_head(const uint8_t*& p, const uint8_t* end, item_head& head, std::error_code& ec);
bool skip_item(const uint8_t*& p, const uint8_t* end, std::error_code& ec,
    int max_nesting_depth = default_max_nesting_depth);
```

The wire-level head decoding the checked item layer is built on, for
consumers that need sub-item head access. Prefer the checked item layer
unless you are walking objects yourself with intention. `read_head`
advances `p` past one head only (tags are returned as their own heads) and
validates just that head; `skip_item` advances past one complete item.

### Examples

#### Reading a message without copying it

```cpp
#include <jsoncons_ext/cbor/cbor_view.hpp>
#include <iostream>

int main()
{
    // {"id": 42, "name": "ada", "scores": [1, 2]}
    const std::vector<uint8_t> data = {
        0xa3,
        0x62,'i','d', 0x18,0x2a,
        0x64,'n','a','m','e', 0x63,'a','d','a',
        0x66,'s','c','o','r','e','s', 0x82,0x01,0x02
    };

    auto doc = jsoncons::cbor::view::parse_exact(
        jsoncons::span<const uint8_t>(data.data(), data.size()));
    if (!doc.has_value())
    {
        std::cout << "malformed at offset " << doc.error().offset << "\n";
        return 1;
    }

    for (jsoncons::cbor::view::map_entry entry : doc.value().entries())
    {
        jsoncons::string_view name;
        if (!entry.key.text(name))
        {
            continue;
        }
        std::cout << name << ":";

        uint64_t number = 0;
        jsoncons::string_view text;
        if (entry.value.text(text))
        {
            std::cout << " " << text;
        }
        else if (entry.value.uint64_value(number))
        {
            std::cout << " " << number;
        }
        else
        {
            for (jsoncons::cbor::view::item element : entry.value.elements())
            {
                if (element.uint64_value(number))
                {
                    std::cout << " " << number;
                }
            }
        }
        std::cout << "\n";
    }
}
```

Output:
```
id: 42
name: ada
scores: 1 2
```

#### Splitting a stream of items

```cpp
jsoncons::span<const uint8_t> rest(data.data(), data.size());
while (!rest.empty())
{
    auto scanned = jsoncons::cbor::view::scan_prefix(rest);
    if (!scanned.has_value())
    {
        break;   // scanned.error().code, scanned.error().offset
    }
    handle(scanned.value().first);
    rest = scanned.value().remainder;
}
```
