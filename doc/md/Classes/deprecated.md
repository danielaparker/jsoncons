# Deprecated Features

As the `jsoncons` library has evolved, names have sometimes changed. To ease transition, jsoncons deprecates the old names but continues to support many of them. The deprecated names can be suppressed by defining macro JSONCONS_NO_DEPRECATED, which is recommended for new code.

In the table, <em>&#x2713;</em> indicates that the old name is still supported.

Category|Old name, now deprecated|<em>&#x2713;</em>|Recommendation
--------|-----------|--------------|------------------------
json|add(size_t index, const json& val)|<em>&#x2713;</em>|Use add(array_iterator pos, const json& val)
    |add(size_t index, json&& val)|<em>&#x2713;</em>|Use add(array_iterator pos, json&& val)
    |remove_range(size_t from_index, size_t to_index)|<em>&#x2713;</em>|erase(array_iterator first, array_iterator last)
    |remove(const std::string& name)|<em>&#x2713;</em>|Use erase(const std::string& name)
    |parse(std::istream& is)|<em>&#x2713;</em>|Use parse_stream(std::istream& is)
    |parse(std::istream& is, parse_error_handler& err_handler)|<em>&#x2713;</em>|Use parse_stream(std::istream& is, parse_error_handler& err_handler)
    |as_int() const|<em>&#x2713;</em>|Use `as<int>`
    |as_uint() const|<em>&#x2713;</em>|Use `as<unsigned int>`
    |as_long() const|<em>&#x2713;</em>|Use `as<long>`
    |as_ulong() const|<em>&#x2713;</em>|Use `as<unsigned long>`
    |as_longlong() const|<em>&#x2713;</em>|Use as_integer or `as<long long>`
    |as_ulonglong() const|<em>&#x2713;</em>|Use as_uinteger or `as<unsigned long long>`
    |is_longlong() const|<em>&#x2713;</em>|Use is_integer() or is<long long>()
    |is_ulonglong() const|<em>&#x2713;</em>|Use is_uinteger() or is<unsigned long long>()
    |is_numeric() const|<em>&#x2713;</em>|Use `is_number()`
    |remove_member(const std::string& name)|<em>&#x2713;</em>|Use erase(const std::string& name)
    |const json& get(const std::string& name) const|<em>&#x2713;</em>|Use const json get(const std::string& name, T default_val) const with default `json::null_type()`
    |has_member(const std::string& name) const|<em>&#x2713;</em>|Use count(const std::string& name)
    |begin_members()|<em>&#x2713;</em>|Use members().begin()
    |begin_members() const|<em>&#x2713;</em>|Use members().begin()
    |end_members()|<em>&#x2713;</em>|Use members().end()
    |end_members() const|<em>&#x2713;</em>|Use members().end()
    |begin_elements()|<em>&#x2713;</em>|Use elements().begin()
    |begin_elements() const|<em>&#x2713;</em>|Use elements().begin()
    |end_elements()|<em>&#x2713;</em>|Use elements().end()
    |end_elements() const|<em>&#x2713;</em>|Use elements().end()
    |is_empty() const|<em>&#x2713;</em>|Use `empty()`
    |parse_string(const std::string& s)|<em>&#x2713;</em>|parse(const std::string& s)
    |parse_string(const std::string& s,parse_error_handler& err_handler)|<em>&#x2713;</em>|Use parse(const std::string& s,parse_error_handler& err_handler)
    |resize_array(size_t n)|<em>&#x2713;</em>|resize(size_t n)
    |resize_array(size_t n, const json& val)|<em>&#x2713;</em>|Use resize(size_t n, const json& val)
    |to_stream|<em>&#x2713;</em>|Use write
    |`json` initializer-list constructor||Construct from `json::array` with initializer-list
    |json_deserializer|<em>&#x2713;</em>|Use `json_encoder`
    |wjson_deserializer|<em>&#x2713;</em>|Use `wjson_encoder`
    |ojson_deserializer|<em>&#x2713;</em>|Use `ojson_encoder`
    |wojson_deserializer|<em>&#x2713;</em>|Use `wojson_encoder`
json member types|any|<em>&#x2713;</em>|
json member constants|null||Use static member function `json::null()`
    |an_object||Use the default constructor `json()` instead.
    |an_array||Use assignment to `json::array()` or `json::make_array()` instead.
json_encoder|json& root()|<em>&#x2713;</em>|json get_result()
serialization|`output_format`|<em>&#x2713;</em>|Use `serialization_options`
json_reader|read()|<em>&#x2713;</em>|Use read_next()
    |max_depth() const|<em>&#x2713;</em>|Use max_nesting_depth() const
    |max_depth(depth)|<em>&#x2713;</em>|Use max_nesting_depth() const
    |json_input_handler& parent()|<em>&#x2713;</em>|Use json_input_handler& input_handler()
json_input_handler class|do_longlong_value(long long value, const parsing_context& context)||Override do_integer_value(int64_t value, const parsing_context& context)
    |do_ulonglong_value(unsigned long long value, const parsing_context& context)||Removed, override do_uinteger_value(uint64_t value, const parsing_context& context)
    |do_double_value(double value, const basic_parsing_context<CharT>& context)||Removed, override do_double_value(double value, uint8_t precision, const basic_parsing_context<CharT>& context)
json_output_handler class|do_longlong_value(long long value)||Removed, override do_integer_value(int64_t value)
    |do_ulonglong_value(unsigned long long value)||Removed, override do_uinteger_value(uint64_t value)
    |do_double_value(double value)||Removed, override do_double_value(double value, uint8_t precision)
basic_parsing_context|last_char()|<em>&#x2713;</em>|Use current_char()
json_filter|parent()|<em>&#x2713;</em>|Use input_handler()
csv_parameters|`header(std::string value)`|<em>&#x2713;</em>|Use `column_names(std::vector<std::string>> value)`
    |`data_types()`|<em>&#x2713;</em>|Use `column_types()`
    |`data_types(std::string value)`|<em>&#x2713;</em>|Use `column_types(std::vector<std::string>> value)`
output_format|`array_array_block_option accessor and modifier` accessor and modifier|<em>&#x2713;</em>|Use `array_array_split_lines` accessor and modifier
    |`array_object_block_option accessor and modifier`|<em>&#x2713;</em>|Use `array_object_split_lines` accessor and modifier
    |`object_array_block_option accessor and modifier`|<em>&#x2713;</em>|Use `object_array_split_lines` accessor and modifier
    |`object_object_block_option accessor and modifier`|<em>&#x2713;</em>|Use `object_object_split_lines` accessor and modifier
