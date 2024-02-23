## Deprecated Features

As the `jsoncons` library has evolved, names have sometimes changed. To ease transition, jsoncons deprecates the old names but continues to support many of them. The deprecated names can be suppressed by defining macro JSONCONS_NO_DEPRECATED, which is recommended for new code.

Related class|Old name|Replacement
--------|-----------|--------------
__bson__|&nbsp;|&nbsp;
`basic_bson_reader`|&nbsp;|&nbsp;
&nbsp;|`bson_reader`|`bson_stream_reader`
&nbsp;|`bson_buffer_reader`|`bson_bytes_reader`
`basic_bson_cursor`|&nbsp;|&nbsp;
&nbsp;|`read`|`read_to`
`basic_bson_encoder`|&nbsp;|&nbsp;
&nbsp;|`bson_encoder`|`bson_stream_encoder`
&nbsp;|`bson_serializer`|`bson_stream_encoder`
&nbsp;|`bson_buffer_serializer`|`bson_bytes_encoder`
__cbor__|&nbsp;|&nbsp;
`basic_cbor_reader`|&nbsp;|&nbsp;
&nbsp;|`cbor_reader`|`cbor_stream_reader`
&nbsp;|`cbor_buffer_reader`|`cbor_bytes_reader`
`basic_cbor_cursor`|&nbsp;|&nbsp;
&nbsp;|`read`|`read_to`
`basic_cbor_encoder`|&nbsp;|&nbsp;
&nbsp;|`cbor_encoder`|`cbor_stream_encoder`
&nbsp;|`cbor_serializer`|`cbor_stream_encoder`
&nbsp;|`cbor_buffer_serializer`|`cbor_bytes_encoder`
__csv__|&nbsp;|&nbsp;
`basic_csv_reader`|&nbsp;|&nbsp;
&nbsp;|`csv_reader`|`csv_string_reader` or `csv_stream_reader`
&nbsp;|`wcsv_reader`|`wcsv_string_reader` or `wcsv_stream_reader`
`basic_csv_cursor`|&nbsp;|&nbsp;
&nbsp;|`csv_cursor`|`csv_stream_cursor`
&nbsp;|`wcsv_cursor`|`wcsv_stream_cursor`
&nbsp;|`read`|`read_to`
`basic_csv_options`|&nbsp;|&nbsp;
&nbsp;|`enum class mapping_type`|`csv_mapping_kind`
&nbsp;|`enum class mapping_kind`|`csv_mapping_kind`
&nbsp;|`enum class quote_styles`|`quote_style_kind`
&nbsp;|`enum class quote_style_type`|`quote_style_kind`
&nbsp;|`csv_parameters`|`csv_options`
&nbsp;|`wcsv_parameters`|`wcsv_options`
&nbsp;|`csv_serializing_options`|`csv_options`
&nbsp;|`wcsv_serializing_options`|`wcsv_options`
&nbsp;|`mapping`|`mapping_kind`
&nbsp;|`floating_point_format`|`float_format`
`basic_csv_encoder`|&nbsp;|&nbsp;
&nbsp;|`csv_serializer`|`csv_stream_encoder`
&nbsp;|`csv_string_serializer`|`csv_string_encoder`
&nbsp;|`csv_encoder`|`csv_stream_encoder`
&nbsp;|`wcsv_encoder`|`wcsv_stream_encoder`
__msgpack__|&nbsp;|&nbsp;
`basic_msgpack_reader`|&nbsp;|&nbsp;
&nbsp;|`msgpack_reader`|`msgpack_stream_reader`
&nbsp;|`msgpack_buffer_reader`|`msgpack_bytes_reader`
`basic_msgpack_cursor`|&nbsp;|&nbsp;
&nbsp;|`read`|`read_to`
`basic_msgpack_encoder`|&nbsp;|&nbsp;
&nbsp;|`msgpack_encoder`|`msgpack_stream_encoder`
&nbsp;|`msgpack_serializer`|`msgpack_stream_encoder`
&nbsp;|`msgpack_buffer_serializer`|`msgpack_bytes_encoder`
__ubjson__|&nbsp;|&nbsp;
`basic_ubjson_reader`|&nbsp;|&nbsp;
&nbsp;|`ubjson_reader`|`ubjson_stream_reader`
&nbsp;|`ubjson_buffer_reader`|`ubjson_bytes_reader`
`basic_ubjson_cursor`|&nbsp;|&nbsp;
&nbsp;|`read`|`read_to`
`basic_ubjson_encoder`|&nbsp;|&nbsp;
&nbsp;|`ubjson_encoder`|`ubjson_stream_encoder`
&nbsp;|`ubjson_serializer`|`ubjson_stream_encoder`
&nbsp;|`ubjson_buffer_serializer`|`ubjson_bytes_encoder`
__jsonpointer__|&nbsp;|&nbsp;
`basic_json_pointer`|&nbsp;|&nbsp;
&nbsp;|`address`|`json_pointer`
&nbsp;|`json_ptr`|`json_pointer`
&nbsp;|`wjson_ptr`|`wjson_pointer`
&nbsp;|`unflatten_options::object`|`unflatten_options::assume_object`
&nbsp;|`insert_or_assign`|`add`
&nbsp;|`insert`|`add_if_absent`
&nbsp;|``|``
&nbsp;|``|``
&nbsp;|``|``
&nbsp;|``|``
&nbsp;|``|``
&nbsp;|``|``
&nbsp;|``|``
&nbsp;|``|``
&nbsp;|``|``
&nbsp;|``|``

