### bson extension

The bson extension implements decode from and encode to the [Binary JSON](http://bsonspec.org/) data format.
You can either parse into or serialize from a variant-like data structure, [basic_json](../basic_json.md), or your own
data structures, using [json_type_traits](../json_type_traits.md).

[decode_bson](decode_bson.md)

[basic_bson_cursor](basic_bson_cursor.md)

[encode_bson](encode_bson.md)

[basic_bson_encoder](basic_bson_encoder.md)

[bson_options](bson_options.md)

#### Mappings between BSON and jsoncons data items

BSON data item  | jsoncons data item  |jsoncons tag
----------------|---------------------|------------
 null           | null          |                  
 true or false  | bool          |                  
 int32 or int64 | int64         |                  
 datetime       | int64         | timestamp        
 int32 or int64 | uint64        |                  
 datetime       | uint64        | timestamp        
 double         | double        |                  
 string         | string        |                  
 binary         | byte_string   | ext                  
 0x04 (array )  | array         |                  
 0x03 (document)| object        |                  

### Examples

#### Document with string and binary

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/bson/bson.hpp>
#include <cassert>

int main()
{
    // Create some bson
    std::vector<uint8_t> buffer;
    bson::bson_bytes_encoder encoder(buffer);
    encoder.begin_object(); // The total number of bytes comprising 
                            // the bson document will be calculated
    encoder.key("Hello");
    encoder.string_value("World");
    encoder.key("Data");
    std::vector<uint8_t> bstr = {'f','o','o','b','a','r'};
    encoder.byte_string_value(bstr); // default subtype is user defined
    // or encoder.byte_string_value(bstr, 0x80); 
    encoder.end_object();
    encoder.flush();

    std::cout << "(1)\n" << byte_string_view(buffer) << "\n\n";

    /*
        0x27,0x00,0x00,0x00, // Total number of bytes comprising the document (40 bytes) 
            0x02, // URF-8 string
                0x48,0x65,0x6c,0x6c,0x6f, // Hello
                0x00, // trailing byte 
            0x06,0x00,0x00,0x00, // Number bytes in string (including trailing byte)
                0x57,0x6f,0x72,0x6c,0x64, // World
                0x00, // trailing byte
            0x05, // binary
                0x44,0x61,0x74,0x61, // Data
                0x00, // trailing byte
            0x06,0x00,0x00,0x00, // number of bytes
                0x80, // subtype
                0x66,0x6f,0x6f,0x62,0x61,0x72,
        0x00
    */

    ojson j = bson::decode_bson<ojson>(buffer);

    std::cout << "(2)\n" << pretty_print(j) << "\n\n";
    std::cout << "(3) " << j["Data"].tag() << "("  << j["Data"].ext_tag() << ")\n\n";

    // Get binary value as a std::vector<uint8_t>
    auto bstr2 = j["Data"].as<std::vector<uint8_t>>();
    assert(bstr2 == bstr);

    std::vector<uint8_t> buffer2;
    bson::encode_bson(j, buffer2);
    assert(buffer2 == buffer);
}
```
Output:
```
(1)
27,00,00,00,02,48,65,6c,6c,6f,00,06,00,00,00,57,6f,72,6c,64,00,05,44,61,74,61,00,06,00,00,00,80,66,6f,6f,62,61,72,00

(2)
{
    "Hello": "World",
    "Data": "Zm9vYmFy"
}

(3) ext(128)
```

### See also

[byte_string_view](../byte_string_view.md)
