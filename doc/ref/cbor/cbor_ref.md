### jsoncons::cbors::cbor_ref

A `cbor_ref` is a non-owning reference to a contiguous sequence of bytes within a packed cbor object.
The first byte in the sequence contains the type of the CBOR value.

#### Header
```c++
#include <jsoncons_ext/cbor/cbor.hpp>

class cbor_ref
```

Member type          |Definition
---------------------|------------------------------
`value_type`         |`cbor_ref`
`reference`          |`cbor_ref&`
`const_reference`    |`const cbor_ref&`
`pointer`            |`cbor_ref*`
`const_pointer`      |`const cbor_ref*`
`string_type`        |`std::string`
`key_value_pair_type`|[key_value_pair_view<cbor_ref>](cbor_ref/key_value_pair_view.md)
`string_view_type`   |A non-owning view of a string, holds a pointer to character data and length. Supports conversion to and from strings. Will be typedefed to the C++ 17 [string view](http://en.cppreference.com/w/cpp/string/basic_string_view) if `JSONCONS_HAS_STRING_VIEW` is defined in `jsoncons_config.hpp`, otherwise proxied. 
`const_object_iterator`|A const [ForwardIterator](http://en.cppreference.com/w/cpp/concept/ForwardIterator) to const key_value_pair_type
`object_iterator`|Same as `const_object_iterator`
`const_array_iterator`|A const [ForwardIterator](http://en.cppreference.com/w/cpp/concept/ForwardIterator) to `cbor_ref`
`array_iterator`|Same as `const_array_iterator`

#### Constructors

```c++
cbor_ref(); // (1)

cbor_ref(const uint8_t* buffer, size_t buflen); // (2)

cbor_ref(const std::vector<uint8_t>& buffer); // (3)

cbor_ref(const cbor_ref& other); // (4)
```

(1) Constructs an empty `cbor_ref` with the result that `buffer()` is `nullptr` and `buflen()` is 0.

(2) Constructs a `cbor_ref` on the first `buflen` bytes of the buffer.

(3) Constructs a `cbor_ref` on `buffer`.

(4) Constructs a `cbor_ref` on the same content as `other`.

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
    <td><a href="cbor_ref/array_range.md">array_range</a></td>
    <td>Returns a "range" that supports a range-based for loop over the elements of a <code>cbor_ref</code> array.</td> 
  </tr>
  <tr>
    <td><a href="cbor_ref/object_range.md">obect_range</a></td>
    <td>Returns a "range" that supports a range-based for loop over the key-value pairs of a <code>cbor_ref</code> object.</td> 
  </tr>
</table>

#### Capacity

<table border="0">
  <tr>
    <td><code>size_t size() const</code></td>
    <td>Returns the length of the array or map if the first byte in the CBOR buffer is a CBOR tag that indicates an array or map, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><a>bool empty() const</a></td>
    <td>Returns <code>true</code> if a CBOR string, object or array has no elements, otherwise <code>false</code></td> 
  </tr>
</table>

#### Accessors

<table border="0">
  <tr>
    <td><a href="cbor_ref/is.md">is</a></td>
    <td>Checks if a CBOR packed value matches a type.</td> 
  </tr>
  <tr>
    <td><a href="cbor_ref/as.md">as</a></td>
    <td>Attempts to convert a CBOR packed value to a value of a type.</td> 
  </tr>
  <tr>
    <td><code>cbor_ref at(size_t pos) const</code></td>
    <td>Returns a view of the CBOR array element at specified index <code>pos</code>.</td> 
  </tr>
  <tr>
    <td><code>cbor_ref at(const string_view_type& key) const</code></td>
    <td>Returns a view of the CBOR object member value with key equivalent to <code>key</code>.</td> 
  </tr>
</table>

#### Select values from `cbor_ref` object

A `cbor_ref` satisfies the requirements for [jsonpointer::get](../jsonpointer/get.md).

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>

using namespace jsoncons;

int main()
{
    ojson j = ojson::parse(R"(
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

    std::vector<uint8_t> data;
    cbors::encode_cbor(j, data);

    std::error_code ec;
    cbors::cbor_ref rated = jsonpointer::get(cbors::cbor_ref(data), "/reputons/0/rated", ec);
    cbors::cbor_ref rating = jsonpointer::get(cbors::cbor_ref(data), "/reputons/0/rating", ec);

    std::cout << rated.as_string() << ", " << rating.as_double() << std::endl;
}
```

Output:

```
sk, 0.9
```

#### See also

- [jsonpointer::get](../jsonpointer/get.md)

