### bson extension

The bson extension implements decode from and encode to the [Binary JSON](http://bsonspec.org/) data format.
You can either parse into or serialize from a variant-like data structure, [basic_json](../basic_json.md), or your own
data structures, using [json_type_traits](../json_type_traits.md).

[decode_bson](decode_bson.md)

[basic_bson_cursor](basic_bson_cursor.md)

[encode_bson](encode_bson.md)

[basic_bson_encoder](basic_bson_encoder.md)

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
 binary         | byte_string   |                  
 0x04 (array )  | array         |                  
 0x03 (document)| object        |                  


