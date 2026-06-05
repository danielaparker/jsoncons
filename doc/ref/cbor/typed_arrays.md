### Typed Arrays

jsoncons supports tags for [typed arrays](https://tools.ietf.org/html/rfc8746). 

### Examples 

Reading a CBOR multi-dimensional array and reading a typed array using a cursor have changed in version 1.8.0.

#### Read a typed array

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <iostream>
#include <cassert>

namespace cbor = jsoncons::cbor;

int main()
{
    std::vector<uint8_t> data = {
        0xd8,       // Tag
        0x56,       // Tag 86, float64, little endian, Typed Array
        0x58, 0x20, // Byte string value of length 32
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x40,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x40,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x40,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x40
    };

    // Read typed array using a reader (since 0.171.0)
    jsoncons::json_decoder<jsoncons::json> decoder;
    cbor::cbor_bytes_reader reader(data, decoder);
    std::error_code ec;
    reader.read(ec);
    auto jval = decoder.get_result();
    std::cout << "(1) " << jval << "\n\n";

    // Decode typed array (since 0.171.0)
    auto u = cbor::decode_cbor<std::vector<double>>(data);
    std::cout << "(2) [";
    for (std::size_t i = 0; i < u.size(); ++i)
    {
        if (i > 0) std::cout << ',';
        std::cout << u[i];
    }
    std::cout << "]\n\n";

    // Read typed array using a cursor (since 1.8.0)
    cbor::cbor_bytes_cursor cursor(data);
    assert(jsoncons::staj_events::begin_array == cursor.current().event_type());
    assert(cursor.is_typed_array());

    std::vector<double> v;
    cursor.read_typed_array(v);
    assert(jsoncons::staj_events::end_array == cursor.current().event_type());

    std::cout << "(3) [";
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        if (i > 0) std::cout << ',';
        std::cout << v[i];
    }
    std::cout << "]\n\n";

    cursor.next();
    assert(cursor.done());
}
```
Output:
```
(1) [10.0,20.0,30.0,40.0]

(2) [10,20,30,40]

(3) [10,20,30,40]
```

#### Read a 3D typed array with row-major storage

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <iostream>
#include <cassert>

namespace cbor = jsoncons::cbor;

int main() 
{
    // A 3D typed array 2 x 3 x 2 with row-major storage
    std::vector<uint8_t> data = {
        0xD8, 0x28,                         // tag(40) row major storage 
        0x82,                               // array(2)
        0x83,                               // shape array(3)
        0x02, 0x03, 0x02,                   // [2, 3, 2]
        0xD8, 0x40,                         // tag(64) uint8 typed array
        0x4C,                               // bytes(12)
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 
        0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C
    };

    // Read 3D typed array using a reader (since 1.8.0)
    jsoncons::json_decoder<jsoncons::json> decoder;
    cbor::cbor_bytes_reader reader(data, decoder);
    std::error_code ec;
    reader.read(ec);
    jsoncons::json result = decoder.get_result();
    std::cout << "(1) " << result << "\n\n";

    // Read 3D typed array using a cursor (since 1.8.0)
    cbor::cbor_bytes_cursor cursor(data);

    assert(jsoncons::staj_events::begin_array == cursor.current().event_type());
    assert(true == cursor.is_multi_dim());
    assert(jsoncons::mdarray_order::row_major == cursor.order());
    assert(true == cursor.is_typed_array());
    assert(jsoncons::typed_array_tags::uint8 == cursor.array_tag());

    auto extents = cursor.extents();
    std::cout << "(2) ";
    for (std::size_t i = 0; i < extents.size(); ++i)
    {
        if (i > 0) std::cout << " x ";
        std::cout << extents[i];
    }
    std::cout << "\n\n";

    std::vector<int> v;
    cursor.read_typed_array(v);
    std::cout << "(3) [";
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        if (i > 0) std::cout << ',';
        std::cout << v[i];
    }
    std::cout << "]\n\n";

    assert(jsoncons::staj_events::end_array == cursor.current().event_type());
    cursor.next();
    assert(cursor.done());
}
```

Output:

```
(1) [[[1,2],[3,4],[5,6]],[[7,8],[9,10],[11,12]]]

(2) 2 x 3 x 2

(3) [1,2,3,4,5,6,7,8,9,10,11,12]
```

#### Read a 3D typed array with column-major storage

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <iostream>
#include <cassert>

namespace cbor = jsoncons::cbor;

int main() 
{
    // A 3D typed array 2 x 3 x 2 with column-major storage
    std::vector<uint8_t> data = {
        0xD9, 0x04, 0x10,                   // tag(1040) column-major storage 
        0x82,                               // array(2)
        0x83,                               // shape array(3)
        0x02, 0x03, 0x02,                   // [2, 3, 2]
        0xD8, 0x40,                         // tag(64) uint8 typed array
        0x4C,                               // bytes(12)
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C
    };

    // Read 3D typed array using a reader (since 1.8.0)
    jsoncons::json_decoder<jsoncons::json> decoder;
    cbor::cbor_bytes_reader reader(data, decoder);
    std::error_code ec;
    reader.read(ec);
    jsoncons::json result = decoder.get_result();
    std::cout << "(1) " << result << "\n\n";

    // Read 3D typed array using a cursor (since 1.8.0)
    cbor::cbor_bytes_cursor cursor(data);

    assert(jsoncons::staj_events::begin_array == cursor.current().event_type());
    assert(true == cursor.is_multi_dim());
    assert(jsoncons::mdarray_order::column_major == cursor.order());
    assert(true == cursor.is_typed_array());
    assert(jsoncons::typed_array_tags::uint8 == cursor.array_tag());

    auto extents = cursor.extents();
    std::cout << "(2) ";
    for (std::size_t i = 0; i < extents.size(); ++i)
    {
        if (i > 0) std::cout << " x ";
        std::cout << extents[i];
    }
    std::cout << "\n\n";

    std::vector<int> v;
    cursor.read_typed_array(v);
    std::cout << "(3) [";
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        if (i > 0) std::cout << ',';
        std::cout << v[i];
    }
    std::cout << "]\n\n";

    assert(jsoncons::staj_events::end_array == cursor.current().event_type());
    cursor.next();
    assert(cursor.done());
}
```

Output:

```
(1) [[[1,7],[3,9],[5,11]],[[2,8],[4,10],[6,12]]]

(2) 2 x 3 x 2

(3) [1,2,3,4,5,6,7,8,9,10,11,12]
```

#### Read a 3D classical array with row-major storage (since 1.8.0)

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <iostream>
#include <cassert>

namespace cbor = jsoncons::cbor;

int main() 
{
    // A 3D classical array 2 x 3 x 2 with row-major storage
    std::vector<uint8_t> data = {
        0xD8, 0x28,                         // tag(40) row major storage 
        0x82,                               // array(2)
        0x83,                               // shape array(3)
        0x02, 0x03, 0x02,                   // [2, 3, 2]
        0x8C,                               // data array(12)
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C
    };

    // Read 3D classical array using a reader (since 1.8.0)
    jsoncons::json_decoder<jsoncons::json> decoder;
    cbor::cbor_bytes_reader reader(data, decoder);
    std::error_code ec;
    reader.read(ec);
    jsoncons::json result = decoder.get_result();
    std::cout << "(1) " << result << "\n\n";

    // Read 3D classical array using a cursor (since 1.8.0)
    cbor::cbor_bytes_cursor cursor(data);

    assert(jsoncons::staj_events::begin_array == cursor.current().event_type());
    assert(true == cursor.is_multi_dim());
    assert(jsoncons::mdarray_order::row_major == cursor.order());
    assert(false == cursor.is_typed_array());

    auto extents = cursor.extents();
    std::cout << "(2) ";
    for (std::size_t i = 0; i < extents.size(); ++i)
    {
        if (i > 0) std::cout << " x ";
        std::cout << extents[i];
    }
    std::cout << "\n\n";

    jsoncons::json_decoder<jsoncons::json> sub_decoder;
    cursor.read_to(sub_decoder);
    assert(sub_decoder.is_valid());
    auto jval = sub_decoder.get_result();
    assert(jval.is_array());
    std::cout << "(3) [";
    for (std::size_t i = 0; i < jval.size(); ++i)
    {
        if (i > 0) std::cout << ',';
        std::cout << jval[i];
    }
    std::cout << "]\n\n";

    assert(jsoncons::staj_events::end_array == cursor.current().event_type());
    cursor.next();
    assert(cursor.done());
}
```

Output:

```
(1) [[[1,2],[3,4],[5,6]],[[7,8],[9,10],[11,12]]]

(2) 2 x 3 x 2

(3) [1,2,3,4,5,6,7,8,9,10,11,12]
```

#### Write a typed array of half precision floating-point

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <iostream>
#include <iomanip>

int main()
{
    std::vector<uint8_t> buffer;

    auto encode_options = cbor::cbor_options{}
        .use_typed_arrays(true);
    cbor::cbor_bytes_encoder encoder(buffer, encode_options);

    std::vector<uint16_t> values = {0x3bff, 0x3c00, 0x3c01, 0x3555};
    encoder.typed_array(jsoncons::half_arg, values);

    // buffer contains a half precision floating-point, native endian, typed array 
    std::cout << "(1) ";
    jsoncons::print_bytes(buffer, std::cout);   // (since 1.8.0)
    std::cout << "\n\n";

    auto j = cbor::decode_cbor<jsoncons::json>(buffer);

    auto format_options = jsoncons::json_options{}.line_splits(jsoncons::line_split_kind::same_line).
        spaces_around_comma(jsoncons::spaces_option::space_after);
    std::cout << "(2) " << pretty_print(j, format_options) << "\n\n";
}
```
Output
```
(1) d8 54 48 ff 3b 00 3c 01 3c 55 35

(2) [0.99951171875, 1.0, 1.0009765625, 0.333251953125]
```

