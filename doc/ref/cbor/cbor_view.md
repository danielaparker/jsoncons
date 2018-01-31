### jsoncons::cbor::cbor_view

A `cbor_view` object refers to a constant contiguous sequence of bytes within a packed cbor object.

#### Header
```c++
#include <jsoncons_ext/cbor/cbor.hpp>

class cbor_view
```

Member type          |Definition
---------------------|------------------------------
`value_type`         |`cbor_view`
`reference`          |`cbor_view&`
`const_reference`    |`const cbor_view&`
`pointer`            |`cbor_view*`
`const_pointer`      |`const cbor_view*`
`string_type`        |`std::string`
`string_view_type`   |A non-owning view of a string, holds a pointer to character data and length. Supports conversion to and from strings. Will be typedefed to the C++ 17 [string view](http://en.cppreference.com/w/cpp/string/basic_string_view) if `JSONCONS_HAS_STRING_VIEW` is defined in `jsoncons_config.hpp`, otherwise proxied. 

#### Constructors

```c++
cbor_view(); // (1)

cbor_view(cbor_view(const uint8_t* buffer, size_t buflen)); // (2)

cbor_view(const cbor_view& val); // (3)
```

#### CBOR buffer view

<table border="0">
  <tr>
    <td><code>const uint8_t* buffer() const</code></td>
    <td>Returns a pointer to the first byte of the CBOR buffer view.</td> 
  </tr>
  <tr>
    <td><code>size_t buflen() const</code></td>
    <td>Returns length of CBOR buffer view.</td> 
  </tr>
</table>

#### Accessors

<table border="0">
  <tr>
    <td><code>bool is_array() const</code></td>
    <td>Returns <code>true</code> if the first byte in the CBOR buffer is a CBOR tag that indicates an array, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><code>bool is_object() const</code></td>
    <td>Returns <code>true</code> if the first byte in the CBOR buffer is a CBOR tag that indicates a map, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><code>size_t size() const</code></td>
    <td>Returns the length of the array or map if the first byte in the CBOR buffer is a CBOR tag that indicates an array or map, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><code>cbor_view at(size_t pos) const</code></td>
    <td>Returns a view of the CBOR array element at specified index <code>pos</code>.</td> 
  </tr>
  <tr>
    <td><code>cbor_view at(const std::string& key) const</code></td>
    <td>Returns a view of the CBOR object member value with key equivalent to <code>key</code>.</td> 
  </tr>
</table>

#### Select values from `cbor_view` object

A `cbor_view` satisfies the requirements for [jsonpointer::get](../jsonpointer/get.md).

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>

using namespace jsoncons;

int main()
{
    ojson j1 = ojson::parse(R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum.example.com",
           "assertion": "is-good",
           "rated": "sk",
           "rating": 0.90
         }
       ]
    }
    )");

    std::vector<uint8_t> buffer;
    cbor::encode_cbor(j1, buffer);

    cbor::cbor_view b1(buffer); 

    std::error_code ec;
    cbor::cbor_view b2 = jsonpointer::get(b1,"/reputons/0/rated", ec);

    ojson j2 = cbor::decode_cbor<ojson>(b2);

    std::cout << j2 << std::endl;
}
```

Output:

```
"sk"
```

#### See also

- [jsonpointer::get](../jsonpointer/get.md)

