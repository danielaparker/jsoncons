### ubjson extension

The ubjson extension implements decode from and encode to the [Universal Binary JSON Specification](http://ubjson.org/) data format.

[decode_ubjson](decode_ubjson.md)

[encode_ubjson](encode_ubjson.md)

[ubjson_serializer](ubjson_serializer.md)

#### jsoncons-ubjson mappings

jsoncons data item|jsoncons tag|UBJSON data item
--------------|------------------|---------------
null          |                  | null
bool          |                  | true or false
int64         |                  | uint8_t or integer
uint64        |                  | uint8_t or integer
double        |                  | float 32 or float 64
string        |                  | string
string        | big_integer      | high precision number type
string        | big_decimal      | high precision number type
byte_string   |                  | array of uint8_t
array         |                  | array 
object        |                  | object





