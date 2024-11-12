## Deprecated Features

As the `jsoncons` library has evolved, names have sometimes changed. To ease transition, jsoncons deprecates the old names but continues to support many of them. The deprecated names can be suppressed by defining macro JSONCONS_NO_DEPRECATED, which is recommended for new code.

Category/class|Old name|Replacement
--------|-----------|--------------
__corelib__|&nbsp;|&nbsp;
`json_errc`|&nbsp;|&nbsp;
&nbsp;|`json_parser_errc`|`json_errc`
&nbsp;|`json_parse_errc`|`json_errc`
`basic_json`|&nbsp;|&nbsp;
&nbsp;|`owjson`|`wojson`
&nbsp;|`preserve_order_policy`|`order_preserving_policy`
&nbsp;|`value_type`|No replacement
&nbsp;|`kvp_type`|`key_value_type`
&nbsp;|`member_type`|`key_value_type`
&nbsp;|`get_semantic_tag`|`tag`
&nbsp;|`is_datetime`|`tag() == semantic_tag::datetime`
&nbsp;|`is_epoch_time`|`tag() == semantic_tag::epoch_second`
&nbsp;|`add`|`push_back, emplace_back, emplace, insert`
&nbsp;|`set`|`insert_or_assign, try_emplace`
&nbsp;|`has_key`|`contains`
&nbsp;|`has_member`|`contains`
&nbsp;|`is_ulonglong`|`is<unsigned long long>()`
&nbsp;|`is_longlong`|`is<long long>()`
&nbsp;|`as_int`|`as<int>()`
&nbsp;|`as_uint`|`as<unsigned int>()`
&nbsp;|`as_long`|`as<long>()`
&nbsp;|`as_ulong`|`as<unsigned long>()`
&nbsp;|`as_longlong`|`as<long long>()`
&nbsp;|`as_ulonglong`|`as<unsigned long long>()`
&nbsp;|`as_uinteger`|`as<uint64_t>()`
&nbsp;|`is_numeric`|`is_number`
&nbsp;|`resize_array`|`resize`
&nbsp;|`members`|`object_range`
&nbsp;|`elements`|`array_range`
&nbsp;|`begin_members`|`object_range().begin()`
&nbsp;|`end_members`|`object_range().end()`
&nbsp;|`begin_elements`|`array_range().begin()`
&nbsp;|`end_elements`|`array_range().end()`
&nbsp;|`get_with_default(const string_view_type&)`|`at_or_null(const string_view_type&)`
&nbsp;|`get_with_default(const string_view_type&, U&&)`|`get_value_or(const string_view_type&,U&&)`
&nbsp;|`get(const string_view_type&)`|`at_or_null(const string_view_type&)`
&nbsp;|`get(const string_view_type& name, T&& default_value)`|`get_with_default(const string_view_type&, T&&)`
&nbsp;|`remove_range`|`erase`
&nbsp;|`remove`|`erase`
&nbsp;|`remove_member`|`erase`
&nbsp;|`is_empty`|`empty`
&nbsp;|`basic_json(const byte_string_view&, semantic_tag, const Allocator&)`|`basic_json(byte_string_arg_t, const Source&, semantic_tag,const Allocator&)`
&nbsp;|`parse(const char_type* s, std::size_t length)`|`parse(const string_view_type&)`
&nbsp;|`parse_file`|`parse(std::basic_istream<char_type>&, const basic_json_options<char_type>&)>)`
&nbsp;|`parse_stream`|`parse`
&nbsp;|`parse_string`|`parse`
&nbsp;|`dump_fragment`|`dump`
&nbsp;|`dump_body`|`dump`
&nbsp;|`dump(std::basic_ostream<char_type>&, bool)`|`dump(std::basic_ostream<char_type>&, indenting)`
&nbsp;|`write_body`|`dump`
&nbsp;|`write`|`dump`
&nbsp;|`to_stream`|`dump`
&nbsp;|`precision`|No replacement
&nbsp;|`decimal_places`|No replacement
&nbsp;|`get_stor_type`|`storage_kind`
`bigint_chars_format`|&nbsp;|&nbsp;
&nbsp;|`bignum_chars_format`|`bigint_chars_format`
&nbsp;|`bigint_chars_format::integer`|`bigint_chars_format::number`
`basic_json_options`|&nbsp;|&nbsp;
&nbsp;|`enum class chars_format`|`float_chars_format`
`basic_json_parser`|&nbsp;|&nbsp;
&nbsp;|`end_parse`|`finish_parse`
&nbsp;|`set_source`|`update`
`json_type_traits`|&nbsp;|&nbsp;
&nbsp;|`is_json_type_traits_impl`|`is_json_type_traits_declared`
&nbsp;|`set_source`|`update`
`json_filter`|&nbsp;|&nbsp;
&nbsp;|`to_handler`|`destination`
&nbsp;|`input_handler`|`destination`
&nbsp;|`downstream_handler`|`destination`
&nbsp;|`destination_handler`|`destination`
&nbsp;|`json_content_filter`|`json_filter`
&nbsp;|`wjson_content_filter`|`wjson_filter`
&nbsp;|`rename_name_filter`|`rename_object_key_filter`
&nbsp;|`wrename_name_filter`|`wrename_object_key_filter`
&nbsp;|`rename_object_member_filter`|`rename_object_key_filter`
&nbsp;|`wrename_object_member_filter`|`wrename_object_key_filter`
&nbsp;|`default_parse_error_handler`|`default_json_parsing`
&nbsp;|`strict_parse_error_handler`|`strict_json_parsing`
&nbsp;|`can_read_nan_replacement`|`enable_nan_to_num() or enable_nan_to_str()`
&nbsp;|`can_read_pos_inf_replacement`|`enable_inf_to_num() or enable_inf_to_str()`
&nbsp;|`can_read_neg_inf_replacement`|`enable_neginf_to_num() or enable_neginf_to_str()`
&nbsp;|`nan_replacement`|`nan_to_num() or nan_to_str()`
&nbsp;|`indent`|`indent_size`
&nbsp;|`object_object_split_lines`|`object_object_line_splits`
&nbsp;|`array_object_split_lines`|`array_object_line_splits`
&nbsp;|`object_array_split_lines`|`object_array_line_splits`
&nbsp;|`array_array_split_lines`|`array_array_line_splits`
&nbsp;|`replace_inf`|`inf_to_num or inf_to_str`
&nbsp;|`replace_pos_inf`|`inf_to_num or inf_to_str`
&nbsp;|`replace_neg_inf`|`neginf_to_num or neginf_to_str`
&nbsp;|`nan_replacement`|`nan_to_num or nan_to_str`
&nbsp;|`pos_inf_replacement`|`inf_to_num() or inf_to_str()`
&nbsp;|`neg_inf_replacement`|`neginf_to_num() or neginf_to_str()`
&nbsp;|`big_integer_format`|`bigint_format`
&nbsp;|`floating_point_format`|`float_format`
&nbsp;|`dec_to_str`|`lossless_number`
&nbsp;|`output_format`|`json_options`
&nbsp;|`woutput_format`|`wjson_options`
&nbsp;|`serialization_options`|`json_options`
&nbsp;|`wserialization_options`|`wjson_options`
&nbsp;|`json_serializing_options`|`json_options`
&nbsp;|`wjson_serializing_options`|`wjson_options`
`basic_json_visitor`|&nbsp;|&nbsp;
&nbsp;|`basic_json_content_handler`|`basic_json_visitor`
&nbsp;|`json_content_handler`|`json_visitor`
&nbsp;|`wjson_content_handler`|`wjson_visitor`
&nbsp;|`default_json_content_handler`|`default_json_visitor`
&nbsp;|`default_wjson_content_handler`|`wdefault_json_visitor`
&nbsp;|`name`|`key`
&nbsp;|`byte_string_value(const uint8_t*, std::size_t,  semantic_tag,  const ser_context&)`|`byte_string_value(const Source&,semantic_tag, const ser_context&)`
&nbsp;|`byte_string_value(const uint8_t*, std::size_t,  semantic_tag,  const ser_context&, std::error_code&)`|`byte_string_value(const Source&,semantic_tag,  const ser_context&, std::error_code&)`
&nbsp;|`byte_string_value(const byte_string_view&, byte_string_chars_format, semantic_tag, const ser_context&)`|`byte_string_value(const byte_string_view&, semantic_tag, const ser_context&)`
&nbsp;|`byte_string_value(const uint8_t*,  std::size_t,byte_string_chars_format, semantic_tag tag, const ser_context&)`|`byte_string_value(const byte_string_view&, semantic_tag, const ser_context&)`
&nbsp;|`big_integer_value`|`string_value` with `semantic_tag::bigint`
&nbsp;|`bignum_value`|`string_value` with `semantic_tag::bigint`
&nbsp;|`big_decimal_value`|`string_value` with `semantic_tag::bigdec`
&nbsp;|`decimal_value`|`string_value` with `semantic_tag::bigdec`
&nbsp;|`date_time_value`|`string_value` with `semantic_tag::datetime`
&nbsp;|`timestamp_value`|`string_value` with `semantic_tag::epoch_second`
&nbsp;|`begin_document`|No replacement
&nbsp;|`end_document`|No replacement
&nbsp;|`begin_json`|No replacement
&nbsp;|`end_json`|No replacement
&nbsp;|`integer_value`|`int64_value`
&nbsp;|`uinteger_value`|`uint64_value`
`basic_json_reader`|&nbsp;|&nbsp;
&nbsp;|`json_reader`|`json_string_reader` or `json_stream_reader`
&nbsp;|`wjson_reader`|`wjson_string_reader` or `wjson_stream_reader`
&nbsp;|`max_nesting_depth`|Use `max_nesting_depth` in options
`basic_staj_event`|&nbsp;|&nbsp;
&nbsp;|`stream_event`|`staj_event`
&nbsp;|`wstream_event`|`wstaj_event`
`basic_json_cursor`|&nbsp;|&nbsp;
&nbsp;|`class basic_json_pull_reader`|`basic_json_cursor`
&nbsp;|`json_pull_reader`|`json_stream_cursor`
&nbsp;|`wjson_pull_reader`|`wjson_stream_cursor`
&nbsp;|`json_staj_cursor`|`json_stream_cursor`
&nbsp;|`wjson_staj_cursor`|`wjson_stream_cursor`
&nbsp;|`json_cursor`|`json_stream_cursor`
&nbsp;|`wjson_cursor`|`wjson_stream_cursor`
`json_decoder`|&nbsp;|&nbsp;
&nbsp;|`json_deserializer`|`json_decoder<json>`
&nbsp;|`wjson_deserializer`|`json_decoder<wjson>`
&nbsp;|`ojson_deserializer`|`json_decoder<ojson>`
&nbsp;|`wojson_deserializer`|`json_decoder<wojson>`
&nbsp;|`json_decoder(result_allocator_arg_t, const allocator_type& alloc, const temp_allocator_type& temp_alloc)`|`json_decoder(const allocator_type& alloc, const temp_allocator_type& temp_alloc)`
&nbsp;|`root()`|`get_result()`
`basic_json_encoder`|&nbsp;|&nbsp;
&nbsp;|`basic_json_serializer`|`basic_json_encoder`
&nbsp;|`json_encoder`|`json_stream_encoder`
&nbsp;|`wjson_encoder`|`wjson_stream_encoder`
&nbsp;|`json_serializer`|`json_stream_encoder`
&nbsp;|`wjson_serializer`|`wjson_stream_encoder`
&nbsp;|`json_compressed_serializer`|`compact_json_stream_encoder`
&nbsp;|`wjson_compressed_serializer`|`compact_wjson_stream_encoder`
&nbsp;|`json_string_serializer`|`json_string_encoder`
&nbsp;|`wjson_string_serializer`|`wjson_string_encoder`
&nbsp;|`json_compressed_stream_encoder`|`compact_json_stream_encoder`
&nbsp;|`wjson_compressed_stream_encoder`|`compact_wjson_stream_encoder`
&nbsp;|`json_compressed_string_encoder`|`compact_json_string_encoder`
&nbsp;|`wjson_compressed_string_encoder`|`compact_wjson_string_encoder`
&nbsp;|`json_compressed_string_serializer`|`compact_json_string_encoder`
&nbsp;|`wjson_compressed_string_serializer`|`wcompact_json_string_encoder`
&nbsp;|`compact_json_encoder`|`compact_json_stream_encoder`
&nbsp;|`wcompact_json_encoder`|`compact_wjson_stream_encoder`
`ser_error`|&nbsp;|&nbsp;
&nbsp;|`line_number()`|`line()`
&nbsp;|`column_number()`|`column()`
&nbsp;|`serialization_error`|`ser_error`
&nbsp;|`json_parse_exception`|`ser_error`
&nbsp;|`parse_exception`|`ser_error`
&nbsp;|`parse_error`|`ser_error`
`conv_error.hpp`|&nbsp;|&nbsp;
&nbsp;|`convert_error`|`conv_error`
`semantic_tag`|&nbsp;|&nbsp;
&nbsp;|`semantic_tag::big_integer`|`semantic_tag::bigint`
&nbsp;|`semantic_tag::big_decimal`|`semantic_tag::bigdec`
&nbsp;|`semantic_tag::big_float`|`semantic_tag::bigfloat`
&nbsp;|`semantic_tag::date_time`|`semantic_tag::datetime`
&nbsp;|`semantic_tag::timestamp`|`semantic_tag::epoch_second`
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
`cbor_options`|&nbsp;|&nbsp;
&nbsp;|`enable_typed_arrays`|`use_typed_arrays`
`cbor_error.hpp`|&nbsp;|&nbsp;
&nbsp;|`cbor_error`|`ser_error`
&nbsp;|`cbor_decode_error`|`ser_error`
__csv__|&nbsp;|&nbsp;
`csv_errc`|&nbsp;|&nbsp;
&nbsp;|`csv_parser_errc`|`csv_errc`
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
`jsonpath.hpp`|&nbsp;|&nbsp;
&nbsp;|`result_type`|`result_options`

