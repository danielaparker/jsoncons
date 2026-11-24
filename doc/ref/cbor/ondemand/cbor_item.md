### jsoncons::cbor::ondemand::cbor_item

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>
```

An `cbor_item` is one complete, structurally valid encoding. 

#### Accessors

<table border="0">
  <tr>
    <td><code>cbor_major_type major_type() const noexcept</code></td>
    <td>Returns a <a href="../cbor_major_type.md">cbor_major_type</></td> 
  </tr>
  <tr>
    <td><code>uint8_t additional_info() const noexcept</code></td>
    <td>Returns CBOR additional information.</td> 
  </tr>
  <tr>
    <td><code>cbor_type type() const noexcept</code></td>
    <td>Returns a <a href="cbor_type.md">cbor_type</a>.</td> 
  </tr>
  <tr>
    <td><code>uint64_t argument() const noexcept</code></td>
    <td>Returns integer's value, a string's length, a container's count,
or a simple value's number (the bit pattern of a float).</td> 
  </tr>
  <tr>
    <td><code>bool indefinite() const noexcept</code></td>
    <td>Returns true if a byte string or text string has indefinite length, otherwise false.</td> 
  </tr>
  <tr>
    <td><code>tag_range tags() const noexcept</code></td>
    <td>Returns the the leading tag numbers, outermost first.</td> 
  </tr>
  <tr>
    <td><code>child_range children() const noexcept</code></td>
    <td>Returns the data items an item is made of, as checked items: an
array's elements, a map's keys and values alternating, or the definite-length
chunks of an indefinite-length string (RFC 8949 section 3.2.3). It is empty
for anything else. Each child is measured once as the iteration reaches it;
the last child of a definite-length container is not measured at all, since
it ends where its parent does.</td> 
  </tr>
  <tr>
    <td><code>access_result&lt;uint64_t&gt; get_uint64() const noexcept</code></td>
    <td>Returns an <a href="access_result.md">access_result&lt;uint64_t&gt;</a>.</td> 
  </tr>
  <tr>
    <td><code>access_result&lt;int64_t&gt; get_int64() const noexcept</code></td>
    <td>Returns an <a href="access_result.md">access_result&lt;int64_t&gt;</a>.</td> 
  </tr> 
  <tr>
    <td><code>access_result&lt;bool> get_bool() const noexcept</code></td>
    <td>Returns an <a href="access_result.md">access_result&lt;bool&gt;</a>.</td> 
  </tr>
  <tr>
    <td><code>access_result&lt;double> get_double() const noexcept</code></td>
    <td>Returns an <a href="access_result.md">access_result&lt;double&gt;</a>.</td> 
  </tr>
  <tr>
    <td><code>access_result&lt;jsoncons::string_view> get_string() const noexcept</code></td>
    <td>Validates UTF-8 if not already validated. Returns an <a href="access_result.md">access_result&lt;jsoncons::string_view&gt;</a>
    guaranteed to contain valid UTF-8.</td> 
  </tr>
  <tr>
    <td><code>access_result&lt;jsoncons::u8string_view> get_u8string() const noexcept</code></td>
    <td>Returns an <a href="access_result.md">access_result&lt;jsoncons::string_view&gt;</a> 
    guaranteed to contain valid UTF-8. This function participates in overload resolution 
    only if **__cpp_char8_t** is defined.</td> 
  </tr>
  <tr>
    <td><code>access_result&lt;jsoncons::span&lt;const uint8_t&gt;> get_byte_string() const noexcept</code></td>
    <td>Returns an <a href="access_result.md">access_result&lt;jsoncons::span&lt;const uint8_t&gt;&gt;</a>.</td> 
  </tr>
  <tr>
    <td><code>template &lt;typename CharBuffer&gt;<br/> access_result&lt;void> get_string(CharBuffer& buffer) const</code></td>
    <td>Copying get_string overload guaranteed to produce valid UTF-8, assembles
indefinite-length strings into the provided buffer.</td> 
  </tr>
  <tr>
    <td><code>template &lt;typename ByteBuffer&gt;<br/>access_result&lt;void> get_byte_string(ByteBuffer& buffer) const</code></td>
    <td>Copying get_byte_string overload, assembles
indefinite-length strings into the provided buffer.</td> 
  </tr>
  <tr>
    <td><code>template &lt;typename StringViewLike&gt;<br/>access_result&lt;cbor_view> at(StringViewLike key) const</code></td>
    <td>Returns item if found, otherwise error.</td> 
  </tr>
  <tr>
    <td>access_result&lt;cbor_view> at(const char* key) const</code></td>
    <td>Returns item if found, otherwise error.</td> 
  </tr>
</table>

### Examples

#### Try to access indefinite length byte string

```cpp

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>
#include <jsoncons/utility/byte_string.hpp>
#include <iostream>
#include <vector>

int main()
{
    std::vector<uint8_t> data = {
        0x5F,                   // Start indefinite-length byte string
        0x42, 0x01, 0x02,       // Chunk 1: 2 bytes
        0x43, 0x03, 0x04, 0x05, // Chunk 2: 3 bytes
        0xFF                    // Break
    };

    auto item = cbor::ondemand::read_item(data);
    if (!item)
    {
        return 1;
    }
    if (auto bstr = (*item).get_byte_string(); bstr)
    {
        std::cout << "(1) ";
        jsoncons::print_bytes(*bstr, std::cout);
        std::cout << "\n";
    }
    else
    {
        std::cout << "(1) " << bstr.error().message() << "\n";
    }
    std::cout << "\n";

    std::vector<uint8_t> buf;
    if (auto bstr = (*item).get_byte_string(buf); bstr)
    {
        std::cout << "(2) ";
        jsoncons::print_bytes(buf, std::cout);
        std::cout << "\n";
    }
    else
    {
        std::cout << "(2) " << bstr.error().message() << "\n";
    }
    return 0;
}
```
Output:

```
(1) Indefinite length at position 0

(2) 01 02 03 04 05
```

#### Try to access definite length string containing invalid UTF-8

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>
#include <iostream>
#include <vector>
#include <string>

int main()
{
    std::vector<uint8_t> data = {
        0x63,             // CBOR text string, length 3
        0x61, 0xFF, 0x62  // 'a', invalid UTF-8 byte, 'b'
    };

    auto item = cbor::ondemand::read_item(data);
    if (!item)
    {
        return 1;
    }
    std::string buf;
    if (auto str = (*item).get_string(buf); str)
    {
        std::cout << buf << "\n";
    }
    else
    {
        std::cout << str.error().message() << "\n";
    }
    return 0;
}
```
Output:

```
Invalid UTF-8 at position 0
```

#### Try to access indefinite length string containing invalid UTF-8

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>
#include <iostream>
#include <vector>
#include <string>

int main()
{
    std::vector<uint8_t> data = {
        0x7F,             // Indefinite-length CBOR text string
        0x62, 0x61, 0xC3, // Chunk 1: 'a' + first byte of UTF-8 sequence
        0x62, 0xA9, 0x62, // Chunk 2: second byte of UTF-8 sequence + 'b'
        0xFF              // Break
    };

    auto item = cbor::ondemand::read_item(data);
    if (!item)
    {
        return 1;
    }
    std::string buf;
    if (auto str = (*item).get_string(buf); str)
    {
        std::cout << buf << "\n";
    }
    else
    {
        std::cout << str.error().message() << "\n";
    }
    return 0;
}
```
Output:

```
Invalid UTF-8 at position 0
```

#### Access mapped value by key

```cpp
    const std::vector<uint8_t> data = {
0xA3,                         // Map with 3 key-value pairs
0x63, 0x66, 0x6F, 0x6F,       // Key: "foo"
0x01,                         // Value: 1
0x63, 0x62, 0x61, 0x72,       // Key: "bar"
0x02,                         // Value: 2
0x63, 0x62, 0x61, 0x7A,       // Key: "baz"
0x03                          // Value: 3
    };

    auto item = cbor::ondemand::read_item(data);   
    if (!item)
    {
        return 1;
    }
    if (auto bar = (*item).at("bar"); bar)
    {
        if (auto val = (*bar).get_uint64(); val)
        {
            std::cout << "(1) " << *val << "\n";
        }
    }
    else
    {
        std::cout << "(1) " << bar.error().message() << "\n";
    }
    if (auto qux = (*item).at("qux"); qux)
    {
        if (auto val = (*qux).get_uint64(); val)
        {
            std::cout << "(2) " << *val << "\n";
        }
    }
    else
    {
        std::cout << "(2) " << qux.error().message() << "\n";
    }
    return 0;
}
```
Output:

```
(1) 2
(2) Key not found at position 0
```
