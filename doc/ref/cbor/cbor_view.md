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
`key_value_pair_type`|`key_value_pair_type` is a class that has `std::string key()` and `cbor_view value()` member functions
`string_view_type`   |A non-owning view of a string, holds a pointer to character data and length. Supports conversion to and from strings. Will be typedefed to the C++ 17 [string view](http://en.cppreference.com/w/cpp/string/basic_string_view) if `JSONCONS_HAS_STRING_VIEW` is defined in `jsoncons_config.hpp`, otherwise proxied. 
`object_iterator`|A const [ForwardIterator](http://en.cppreference.com/w/cpp/concept/ForwardIterator) to key_value_pair_type
`const_object_iterator`|A const [ForwardIterator](http://en.cppreference.com/w/cpp/concept/ForwardIterator) to const key_value_pair_type
`array_iterator`|A const [ForwardIterator](http://en.cppreference.com/w/cpp/concept/ForwardIterator) to `cbor_view`
`const_array_iterator`|A const [ForwardIterator](http://en.cppreference.com/w/cpp/concept/ForwardIterator) to `cbor_view`

#### Constructors

```c++
cbor_view(); // (1)

cbor_view(const uint8_t* buffer, size_t buflen); // (2)

cbor_view(const std::vector<uint8_t>& buffer); // (3)

cbor_view(const cbor_view& other); // (4)
```

(1) Constructs an empty `cbor_view` with the result that `buffer()` is `nullptr` and `buflen()` is 0.

(2) Constructs a `cbor_view` on the first `buflen` bytes of the buffer.

(3) Constructs a `cbor_view` on `buffer`.

(4) Constructs a `cbor_view` on the same content as `other`.

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

#### Ranges and Iterators

<table border="0">
  <tr>
    <td><a href="cbor_view/array_range.md">array_range</a></td>
    <td>Returns a "range" that supports a range-based for loop over the elements of a <code>cbor_view</code> array.</td> 
  </tr>
  <tr>
    <td><a href="cbor_view/object_range.md">obect_range</a></td>
    <td>Returns a "range" that supports a range-based for loop over the key-value pairs of a <code>cbor_view</code> object.</td> 
  </tr>
</table>

#### Accessors

```
bool as_bool() const; // (1)

int64_t as_integer() const; // (2)

uint64_t as_uinteger() const; // (3)

double as_double() const; // (4)

std::string as_string() const; // (5)
```

<table border="0">
  <tr>
    <td><code>bool is_null() const</code></td>
    <td>Returns <code>true</code> if the first byte in the CBOR buffer is a null tag, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><code>bool is_string() const</code></td>
    <td>Returns <code>true</code> if the first byte in the CBOR buffer is a text string tag, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><code>bool is_byte_string() const</code></td>
    <td>Returns <code>true</code> if the first byte in the CBOR buffer is a byte string tag, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><code>bool is_integer() const</code></td>
    <td>Returns <code>true</code> if the first byte in the CBOR buffer is a tag compatible with int64_t, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><code>bool is_uinteger() const</code></td>
    <td>Returns <code>true</code> if the first byte in the CBOR buffer is a tag compatible with uint84_t, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><code>bool is_double() const</code></td>
    <td>Returns <code>true</code> if the first byte in the CBOR buffer is a tag that represents a floating point, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><code>bool is_bool() const</code></td>
    <td>Returns <code>true</code> if the first byte in the CBOR buffer is a boolean tag, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><code>bool is_array() const</code></td>
    <td>Returns <code>true</code> if the first byte in the CBOR buffer is a tag for a CBOR array, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><code>bool is_object() const</code></td>
    <td>Returns <code>true</code> if the first byte in the CBOR buffer is a tag for a CBOR map, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><code>bool as_bool() const</code></td>
    <td></td> 
  </tr>
  <tr>
    <td><code>int64_t as_integer() const</code></td>
    <td></td> 
  </tr>
  <tr>
    <td><code>uint64_t as_uinteger() const</code></td>
    <td></td> 
  </tr>
  <tr>
    <td><code>double as_double() const</code></td>
    <td></td> 
  </tr>
  <tr>
    <td><code>std::string as_string() const</code></td>
    <td></td> 
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
    <td><code>cbor_view at(const string_view_type& key) const</code></td>
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

    std::error_code ec;
    cbor::cbor_view v2 = jsonpointer::get(cbor::cbor_view(buffer), "/reputons/0/rated", ec);

    ojson j2 = cbor::decode_cbor<ojson>(v2);

    std::cout << j2 << std::endl;
}
```

Output:

```
"sk"
```

#### See also

- [jsonpointer::get](../jsonpointer/get.md)

