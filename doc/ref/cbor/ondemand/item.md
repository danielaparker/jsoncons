### jsoncons::cbor::ondemand::item

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>
```

An `item` is one complete, structurally valid encoding. 

#### Accessors

<table border="0">
  <tr>
    <td>cbor_major_type major_type() const noexcept</td>
    <td>Returns a cbor_major_type</td> 
  </tr>
  <tr>
    <td>uint8_t additional_info() const noexcept</td>
    <td>Returns CBOR additional information.</td> 
  </tr>
  <tr>
    <td>item_type type() const noexcept</td>
    <td>Returns a <a href="item_type.md">item_type</a>.</td> 
  </tr>
  <tr>
    <td>uint64_t argument() const noexcept</td>
    <td>Returns integer's value, a string's length, a container's count,
or a simple value's number (the bit pattern of a float).</td> 
  </tr>
  <tr>
    <td>bool indefinite() const noexcept</td>
    <td>Returns true if a byte string or text string has indefinite length, otherwise false.</td> 
  </tr>
  <tr>
    <td>tag_range tags() const noexcept</td>
    <td>Returns the the leading tag numbers, outermost first.</td> 
  </tr>
  <tr>
    <td>child_range children() const noexcept</td>
    <td>Returns the data items an item is made of, as checked items: an
array's elements, a map's keys and values alternating, or the definite-length
chunks of an indefinite-length string (RFC 8949 section 3.2.3). It is empty
for anything else. Each child is measured once as the iteration reaches it;
the last child of a definite-length container is not measured at all, since
it ends where its parent does.</td> 
  </tr>
  <tr>
    <td>access_result&lt;uint64_t&gt; get_uint64() const noexcept</td>
    <td>Returns an <a href="access_result.md">access_result&lt;uint64_t&gt;</a>.</td> 
  </tr>
  <tr>
    <td>access_result&lt;int64_t&gt; get_int64() const noexcept</td>
    <td>Returns an <a href="access_result.md">access_result&lt;int64_t&gt;</a>.</td> 
  </tr>
  <tr>
    <td>access_result&lt;bool> get_bool() const noexcept</td>
    <td>Returns an <a href="access_result.md">access_result&lt;bool&gt;</a>.</td> 
  </tr>
  <tr>
    <td>access_result&lt;double> get_double() const noexcept</td>
    <td>Returns an <a href="access_result.md">access_result&lt;double&gt;</a>.</td> 
  </tr>
  <tr>
    <td>access_result&lt;jsoncons::string_view> get_string() const noexcept</td>
    <td>Returns an <a href="access_result.md">access_result&lt;jsoncons::string_view&gt;</a>
    guaranteed to contain valid UTF-8.</td> 
  </tr>
  <tr>
    <td>access_result&lt;jsoncons::u8string_view> get_u8string() const noexcept</td>
    <td>Returns an <a href="access_result.md">access_result&lt;jsoncons::string_view&gt;</a> 
    guaranteed to contain valid UTF-8. This function template participates in overload resolution 
    only if **__cpp_char8_t** is defined.</td> 
  </tr>
  <tr>
    <td>access_result&lt;jsoncons::span&lt;const uint8_t&gt;> get_byte_string() const noexcept</td>
    <td>Returns an <a href="access_result.md">access_result&lt;jsoncons::span&lt;const uint8_t&gt;&gt;</a>.</td> 
  </tr>
  <tr>
    <td>template <typename CharBuffer><br/> access_result&lt;void> get_string(CharBuffer& buffer) const</td>
    <td>Copying get_string overload guaranteed to produce valid UTF-8, assembles
indefinite-length strings into the provided buffer.</td> 
  </tr>
  <tr>
    <td>template <typename ByteBuffer><br/>access_result&lt;void> get_byte_string(ByteBuffer& buffer) const</td>
    <td>Copying get_byte_string overload, assembles
indefinite-length strings into the provided buffer.</td> 
  </tr>
</table>

#### Non-member functions

```cpp
bool validate_text(const item&) noexcept;
```

`validate_text` checks that a text item is well-formed UTF-8. Each chunk of an
indefinite-length text string is checked independently, as RFC 8949 section
3.2.3 requires.

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
