### jsoncons::cbor::view

```cpp
#include <jsoncons_ext/cbor/cbor_view.hpp>
```

<br>

The `cbor::view` namespace reads the *encoded* structure of
[Concise Binary Object Representation](http://cbor.io/) data in place,
without copying it or building a data structure. Its checked-item layer
borrows complete encodings; its move-only `navigator` owns reusable traversal
workspace while borrowing the same bytes. The data flow is:
> owned bytes -> structural validation -> checked item or navigator -> application semantics

Validation checks structural well-formedness once (framing only — not content
validity such as UTF-8). Checked items and navigator movement then cannot fail
structurally. The last stage — what tag 2 bytes or a tag 25 string reference
*mean* — belongs to [decode_cbor](decode_cbor.md) and
[basic_cbor_cursor](basic_cbor_cursor.md), not here: tags are exposed, never
interpreted.

This namespace is experimental.

#### Ownership and lifetime

All views borrow the input bytes. An `item`, a `navigator`, and every span or
string view obtained from them are
invalidated by anything that invalidates those bytes: destroying the container,
mutating it, or reallocation. A navigator owns only its mutable navigation and
validation workspace; it does not own the encoded bytes. Factory, reset, and
`wire_cursor` entry points reject temporary owning containers at compile time.
The copying item accessors (`text` into `std::string`, `bytes` into
`std::vector`) retain content independently.

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
any depth. It allocates only when nesting exceeds 32 open containers; a reused
`scan_context`, `wire_cursor` with a supplied context, or navigator reset retains
that validation capacity.

#### Structural navigation

```cpp
enum class position_role { root, array_element, map_key, map_value };

struct navigation_result
{
    navigator first;
    span<const uint8_t> remainder;
};

expected<navigation_result, scan_error> navigate_prefix(
    span<const uint8_t> input,
    int max_nesting_depth = default_max_nesting_depth);

expected<navigator, scan_error> navigate_exact(
    span<const uint8_t> input,
    int max_nesting_depth = default_max_nesting_depth);

class navigator
{
public:
    navigator(navigator&&) noexcept;
    navigator& operator=(navigator&&) noexcept;
    navigator(const navigator&) = delete;

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

    bool enter() noexcept;
    bool next() noexcept;
    bool leave() noexcept;
    void rewind() noexcept;

    item finish_item() noexcept;
    bool extent_known() const noexcept;

    expected<span<const uint8_t>, scan_error> reset_prefix(
        span<const uint8_t> input,
        int max_nesting_depth = default_max_nesting_depth);
    expected<span<const uint8_t>, scan_error> reset_exact(
        span<const uint8_t> input,
        int max_nesting_depth = default_max_nesting_depth);
};
```

A navigator begins at the checked root. `enter()` moves into a nonempty array
or map; maps expose raw children with alternating key/value roles. `next()`
moves to the next raw sibling, skipping the current unopened container if
necessary. At the end of the siblings it returns `false`; `leave()` restores
the completed parent. Calling `leave()` early skips only the unread remainder.
`rewind()` restores the root.

The current position exposes its already parsed head and scalar/string content.
A container child may not yet have a known end. `finish_item()` establishes and
caches that end, walking the current subtree only when necessary, and returns an
ordinary complete `item`. `extent_known()` makes that cost visible. Descent after
`finish_item()` is legal and revisits the subtree.

Validation records the observed peak open-container depth, prepares that many
navigation frames, and retains the validation scratch. Movement uses indexed
frames; subtree skips reuse the retained scratch. Neither allocates after
successful construction. Resets are transactional and retain both capacities.
Both reset forms return the unconsumed remainder; it is empty after a
successful exact reset.

A known parent boundary propagates to its final definite child. Consequently,
the last payload in a definite transport tuple can be finished in O(1), even
when it is a large container. Non-final arbitrary containers have the honest
O(depth) tradeoff: descend immediately without a pre-walk, or establish/capture
the exact span with one subtree walk.


#### The checked item

```cpp
class item
{
    span<const uint8_t> encoded_bytes() const noexcept;  // tags included
    item_kind kind() const noexcept;                     // of the untagged content
    uint64_t argument() const noexcept;                  // RFC 8949 head argument
    bool indefinite() const noexcept;
    tag_range tags() const noexcept;                     // leading tags, outermost first

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

```

An `item` is one complete, structurally well-formed encoded item: its leading
semantic tags, head, and content. `kind()` classifies the content after
tags (`simple` covers major type 7: simple values and floating point);
`argument()` is the head's argument — an integer's value, a string's
length, a container's count, a simple value's number, or the bit
pattern of a floating-point value.

`chunks()` yields the contiguous spans of a string's content, one per chunk
for indefinite-length strings. Structural container traversal is intentionally
provided only by `navigator`, avoiding a second iterator state machine and its
hidden subtree rescans.

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

// Span overloads for raw-span consumers; validate input, then order.
template <typename Order = bytewise_compare>
expected<int, scan_error> compare(span<const uint8_t> a, span<const uint8_t> b,
    Order order = Order(), int max_nesting_depth = default_max_nesting_depth);

template <typename Order = bytewise_compare>
expected<bool, scan_error> map_keys_sorted(span<const uint8_t> input,
    Order order = Order(), int max_nesting_depth = default_max_nesting_depth);
```

The order function objects compare encoded bytes and accept either two items
or two raw spans. The `*_compare` forms return a three-way result; the `*_less`
forms are predicates for sorting and ordered containers. The span-based
`map_keys_sorted` validates once, then walks keys with a navigator; malformed
input returns the code and byte offset, and trailing bytes are tolerated.

#### Low-level tier

```cpp
enum class major_type;   // CBOR major types 0-7

struct item_head { major_type major_type; uint8_t additional_info; uint64_t value; bool indefinite(); };

class wire_cursor
{
public:
    explicit wire_cursor(span<const uint8_t> input) noexcept;

    std::size_t position() const noexcept;
    span<const uint8_t> remaining() const noexcept;
    expected<item_head, scan_error> read_head() noexcept;
    expected<item, scan_error> read_item(scan_context& context);
};
```

`wire_cursor` is the offset-based low-level tier for consumers that need
sub-item head access. It borrows one input span and exposes its position and
remaining bytes without a mutable pointer/end pair. `read_head` advances past
one head only (tags are returned as their own heads); `read_item` validates and
returns one complete checked item. Errors report offsets from the beginning of
the cursor's input.
The former public `(const uint8_t*& p, const uint8_t* end)` overloads have
been removed; raw pointer pairs remain implementation details only.
### Examples

#### Reading a message without copying it

```cpp
#include <jsoncons_ext/cbor/cbor_view.hpp>
#include <iostream>
#include <utility>
#include <vector>

int main()
{
    // {"id": 42, "scores": [1, 2]}
    const std::vector<uint8_t> data = {
        0xa2,
        0x62,'i','d', 0x18,0x2a,
        0x66,'s','c','o','r','e','s', 0x82,0x01,0x02
    };

    auto result = jsoncons::cbor::view::navigate_exact(
        jsoncons::span<const uint8_t>(data));
    if (!result)
    {
        return 1;
    }

    auto nav = std::move(result.value());
    if (!nav.enter())
    {
        return 0;
    }
    for (;;)
    {
        jsoncons::string_view name;
        if (!nav.text(name) || !nav.next())
        {
            break;
        }
        std::cout << name << ":";

        uint64_t number = 0;
        if (nav.uint64_value(number))
        {
            std::cout << " " << number;
        }
        else if (nav.enter())
        {
            do
            {
                if (nav.uint64_value(number))
                {
                    std::cout << " " << number;
                }
            }
            while (nav.next());
            nav.leave();
        }
        std::cout << "\n";
        if (!nav.next())
        {
            break;
        }
    }
}
```

Output:
```
id: 42
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
