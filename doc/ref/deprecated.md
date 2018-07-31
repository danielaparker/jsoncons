## Deprecated Features

As the `jsoncons` library has evolved, names have sometimes changed. To ease transition, jsoncons deprecates the old names but continues to support many of them. The deprecated names can be suppressed by defining macro JSONCONS_NO_DEPRECATED, which is recommended for new code.

In the table, <em>&#x2713;</em> indicates that the old name is still supported.

Category|Old name, now deprecated|<em>&#x2713;</em>|Recommendation
--------|-----------|--------------|------------------------
json|add(size_t index, const json& val)|<em>&#x2713;</em>|Use add(array_iterator pos, const json& val)
&#160;|add(size_t index, json&& val)|<em>&#x2713;</em>|Use add(array_iterator pos, json&& val)
&#160;|dump_body|<em>&#x2713;</em>|dump_fragment
&#160;|remove_range(size_t from_index, size_t to_index)|<em>&#x2713;</em>|erase(array_iterator first, array_iterator last)
&#160;|remove(const std::string& name)|<em>&#x2713;</em>|Use erase(const std::string& name)
&#160;|parse_stream(std::istream& is)|<em>&#x2713;</em>|Use parse(std::istream& is)
&#160;|parse_stream(std::istream& is, parse_error_handler& err_handler)|<em>&#x2713;</em>|Use parse(std::istream& is, parse_error_handler& err_handler)
&#160;|as_int() const|<em>&#x2713;</em>|Use `as<int>`
&#160;|as_uint() const|<em>&#x2713;</em>|Use `as<unsigned int>`
&#160;|as_long() const|<em>&#x2713;</em>|Use `as<long>`
&#160;|as_ulong() const|<em>&#x2713;</em>|Use `as<unsigned long>`
&#160;|as_longlong() const|<em>&#x2713;</em>|Use as_integer or `as<long long>`
&#160;|as_ulonglong() const|<em>&#x2713;</em>|Use as_uinteger or `as<unsigned long long>`
&#160;|is_longlong() const|<em>&#x2713;</em>|Use is_integer() or is<long long>()
&#160;|is_ulonglong() const|<em>&#x2713;</em>|Use is_uinteger() or is<unsigned long long>()
&#160;|is_numeric() const|<em>&#x2713;</em>|Use `is_number()`
&#160;|remove_member(const std::string& name)|<em>&#x2713;</em>|Use erase(const std::string& name)
&#160;|const json& get(const std::string& name) const|<em>&#x2713;</em>|Use const json get(const std::string& name, T default_val) const with default `json::null_type()`
&#160;|has_member(const std::string& name) const|<em>&#x2713;</em>|Use has_name(const std::string& name)
&#160;|members()|<em>&#x2713;</em>|Use object_range()
&#160;|elements()|<em>&#x2713;</em>|Use array_range()
&#160;|begin_members()|<em>&#x2713;</em>|Use object_range().begin()
&#160;|end_members()|<em>&#x2713;</em>|Use object_range().end()
&#160;|begin_elements()|<em>&#x2713;</em>|Use array_range().begin()
&#160;|end_elements()|<em>&#x2713;</em>|Use array_range().end()
&#160;|is_empty() const|<em>&#x2713;</em>|Use `empty()`
&#160;|parse_string(const std::string& s)|<em>&#x2713;</em>|parse(const std::string& s)
&#160;|parse_string(const std::string& s,parse_error_handler& err_handler)|<em>&#x2713;</em>|Use parse(const std::string& s,parse_error_handler& err_handler)
&#160;|resize_array(size_t n)|<em>&#x2713;</em>|resize(size_t n)
&#160;|resize_array(size_t n, const json& val)|<em>&#x2713;</em>|Use resize(size_t n, const json& val)
&#160;|to_stream|<em>&#x2713;</em>|Use dump
&#160;|write|<em>&#x2713;</em>|Use dump
&#160;|`json` initializer-list constructor||Construct from `json::array` with initializer-list
&#160;|json_deserializer|<em>&#x2713;</em>|Use json_decoder<json>`
&#160;|wjson_deserializer|<em>&#x2713;</em>|Use `json_decoder<wjson>`
&#160;|ojson_deserializer|<em>&#x2713;</em>|Use `json_decoder<ojson>`
&#160;|wojson_deserializer|<em>&#x2713;</em>|Use `json_decoder<wojson>`
&#160;|owjson|<em>&#x2713;</em>|Use wojson`
&#160;|member_type name()|<em>&#x2713;</em>|Use key()
&#160;|rename_name_filter|<em>&#x2713;</em>|Use rename_object_member_filter`
json member types|any||
&#160;|member_type|<em>&#x2713;</em>|Use key_value_pair_type
&#160;|kvp_type|<em>&#x2713;</em>|Use key_value_pair_type
json member constants|null||Use static member function `json::null()`
&#160;|an_object||Use the default constructor `json()` instead.
&#160;|an_array||Use assignment to `json::array()` or `json::make_array()` instead.
json_decoder|json& root()|<em>&#x2713;</em>|json get_result()
json_content_handler|begin_json|<em>&#x2713;</em>|`begin_document`
&#160;|end_json|<em>&#x2713;</em>|`end_document`
&#160;|do_begin_json||`do_begin_document`
&#160;|do_end_json||`do_end_document`
serialization|`output_format`|<em>&#x2713;</em>|Use `json_serializing_options`
&#160;|`serialization_options`|<em>&#x2713;</em>|Use `json_serializing_options`
json_reader|max_depth(),max_depth(value)|<em>&#x2713;</em>|Use `json_serializing_options::max_nesting_depth`
&#160;|max_nesting_depth(),max_nesting_depth(value)|<em>&#x2713;</em>|Use `json_serializing_options::max_nesting_depth`
&#160;|json_input_handler& parent()|<em>&#x2713;</em>|Use json_input_handler& input_handler()
json_input_handler class|do_longlong_value(long long value, const parsing_context& context)||Override do_integer_value(int64_t value, const parsing_context& context)
&#160;|do_ulonglong_value(unsigned long long value, const parsing_context& context)||Removed, override do_uinteger_value(uint64_t value, const parsing_context& context)
&#160;|do_double_value(double value, const basic_parsing_context<CharT>& context)||Removed, override do_double_value(double value, uint8_t precision, const basic_parsing_context<CharT>& context)
&#160;|`value(value,context)`|<em>&#x2713;</em>|Use `string_value(value,context)`, `integer_value(value,context)`, `uinteger_value(value,context)`, `double_value(value,precision,context)`, `bool_value(value,context)`, `null_value(context)`
json_output_handler class|do_longlong_value(long long value)||Removed, override do_integer_value(int64_t value)
&#160;|do_ulonglong_value(unsigned long long value)||Removed, override do_uinteger_value(uint64_t value)
&#160;|do_double_value(double value)||Removed, override do_double_value(double value, uint8_t precision)
&#160;|`value(value)`|<em>&#x2713;</em>|Use `string_value(value)`, `integer_value(value)`, `uinteger_value(value)`, `double_value(value,precision=0)`, `bool_value(value)`, `null_value(context)`
basic_parsing_context|last_char()|<em>&#x2713;</em>|Use current_char()
json_filter|parent()|<em>&#x2713;</em>|Use downstream_handler()
&#160;|input_handler()|<em>&#x2713;</em>|Use downstream_handler()
csv|`csv_parameters`||Use `csv_serializing_options`
&#160;|`csv_parameters::header(std::string value)`|&#160;|Use `csv_serializing_options::column_names(const std::string& value)`
&#160;|`column_names(std::vector<std::string>> value)`|<em>&#x2713;</em>|Use `column_names(const std::string& value)`
&#160;|`data_types(std::string value)`||Use `column_types(const std::string& value)`
&#160;|`column_types(std::vector<std::string>> value)`|<em>&#x2713;</em>|Use `column_types(const std::string& value)`
&#160;|`column_defaults(std::vector<std::string>> value)`|<em>&#x2713;</em>|Use `column_defaults(const std::string& value)`
output_format|`array_array_block_option accessor and modifier` accessor and modifier|<em>&#x2713;</em>|Use `array_array_split_lines` accessor and modifier
&#160;|`array_object_block_option accessor and modifier`|<em>&#x2713;</em>|Use `array_object_split_lines` accessor and modifier
&#160;|`object_array_block_option accessor and modifier`|<em>&#x2713;</em>|Use `object_array_split_lines` accessor and modifier
&#160;|`object_object_block_option accessor and modifier`|<em>&#x2713;</em>|Use `object_object_split_lines` accessor and modifier
msgpack|`jsoncons_ext/msgpack/message_pack.hpp` header file|<em>&#x2713;</em>|Use `jsoncons_ext/msgpack/msgpack.hpp`
&#160;|`encode_message_pack`|<em>&#x2713;</em>|Use `encode_msgpack`
&#160;|`decode_message_pack`|<em>&#x2713;</em>|Use `decode_msgpack`

