## Deprecated Features

As the `jsoncons` library has evolved, names have sometimes changed. To ease transition, jsoncons deprecates the old names but continues to support many of them. The deprecated names can be suppressed by defining macro JSONCONS_NO_DEPRECATED, which is recommended for new code.

In the table, <em>&#x2713;</em> indicates that the old name is still supported.

Component or location|Old name, now deprecated|<em>&#x2713;</em>|Replacement
--------|-----------|--------------|------------------------
class basic_json|`value_type`|<em>&#x2713;</em>|No replacement
class basic_json|`json_type`|<em>&#x2713;</em>|No replacement
class basic_json|`string_type`|<em>&#x2713;</em>|No replacement
class basic_json|`kvp_type`|<em>&#x2713;</em>|`key_value_type`
class basic_json|`member_type`|<em>&#x2713;</em>|`key_value_type`
class basic_json|`as_integer() with default template argument|<em>&#x2713;</em>|`as<int64_t>()`
class basic_json|`get_semantic_tag()`|<em>&#x2713;</em>|`tag()`
class basic_json|`is_datetime()`|<em>&#x2713;</em>|`tag() == semantic_tag::datetime`
class basic_json|`is_epoch_time()`|<em>&#x2713;</em>|`tag() == semantic_tag::timestamp`
class basic_json|`add(T&&)`|<em>&#x2713;</em>|`push_back(T&&)`
class basic_json|`add(const_array_iterator, T&&)`|<em>&#x2713;</em>|`insert(const_array_iterator, T&&)`
class basic_json|`set(const string_view_type&, T&&)`|<em>&#x2713;</em>|`insert_or_assign(const string_view_type&, T&&)`
class basic_json|`set(object_iterator, const string_view_type&, T&&)`|<em>&#x2713;</em>|`insert_or_assign(object_iterator, const string_view_type&, T&&)`
class basic_json|`is_integer()`|<em>&#x2713;</em>|`is<int64_t>()`
class basic_json|`is_uinteger()`|<em>&#x2713;</em>|`is<uint64_t>()`
class basic_json|`is_ulonglong()`|<em>&#x2713;</em>|`is<unsigned long long>()`
class basic_json|`is_longlong()`|<em>&#x2713;</em>|`is<long long>()`
class basic_json|`as_int()`|<em>&#x2713;</em>|`as<int>()`
class basic_json|`as_uint()`|<em>&#x2713;</em>|`as<unsigned int>()`
class basic_json|`as_long()`|<em>&#x2713;</em>|`as<long>()`
class basic_json|`as_ulong()`|<em>&#x2713;</em>|`as<unsigned long>()`
class basic_json|`as_longlong()`|<em>&#x2713;</em>|`as<long long>()`
class basic_json|`as_ulonglong()`|<em>&#x2713;</em>|`as<unsigned long long>()`
class basic_json|`as_uinteger()`|<em>&#x2713;</em>|`as<uint64_t>()`
class basic_json|`dump(std::basic_ostream<char_type>&, const basic_json_options<char_type>&, bool)`|<em>&#x2713;</em>|`dump(std::basic_ostream<char_type>&, const basic_json_options<char_type>&, indenting)`
class basic_json|`dump(std::basic_ostream<char_type>&, bool)`|<em>&#x2713;</em>|`dump(std::basic_ostream<char_type>&, indenting)`
class basic_json|`to_string(const SAllocator& = SAllocator()))`|<em>&#x2713;</em>|`dump(std::basic_string<char_type,char_traits_type,SAllocator>&)`
class basic_json|`write(basic_json_content_handler<char_type>&)`|<em>&#x2713;</em>|`dump(basic_json_content_handler<char_type>&)`
class basic_json|`write(std::basic_ostream<char_type>&, const basic_json_options<char_type>&)`|<em>&#x2713;</em>|`dump(std::basic_ostream<char_type>&, const basic_json_options<char_type>&)`
class basic_json|`write(std::basic_ostream<char_type>&, const basic_json_options<char_type>&, bool)`|<em>&#x2713;</em>|`dump(std::basic_ostream<char_type>&, const basic_json_options<char_type>&, indenting)`
class basic_json|`to_string(const basic_json_options<char_type>&, char_allocator_type& = char_allocator_type())`|<em>&#x2713;</em>|`dump(std::basic_ostream<char_type>&, const basic_json_options<char_type>&)`
class basic_json|`to_stream(basic_json_content_handler<char_type>&)`|<em>&#x2713;</em>|`dump(basic_json_content_handler<char_type>&)`
class basic_json|`to_stream(std::basic_ostream<char_type>&)`|<em>&#x2713;</em>|`dump(std::basic_ostream<char_type>&)`
class basic_json|`to_stream(std::basic_ostream<char_type>&, const basic_json_options<char_type>&)`|<em>&#x2713;</em>|`dump(std::basic_ostream<char_type>&, const basic_json_options<char_type>&)`
class basic_json|`to_stream(std::basic_ostream<char_type>&, const basic_json_options<char_type>&, bool)`|<em>&#x2713;</em>|`dump(std::basic_ostream<char_type>&, const basic_json_options<char_type>&, indenting)`
class basic_json|`members()`|<em>&#x2713;</em>|`object_range()`
class basic_json|`elements()`|<em>&#x2713;</em>|`array_range()`
class basic_json|`begin_members()`|<em>&#x2713;</em>|`object_range().begin()`
class basic_json|`end_members()`|<em>&#x2713;</em>|`object_range().end()`
class basic_json|`begin_elements()`|<em>&#x2713;</em>|`array_range().begin()`
class basic_json|`end_elements()`|<em>&#x2713;</em>|`array_range().end()`
class basic_json|`resize_array(size_t)`|<em>&#x2713;</em>|`resize(size_t)`
class basic_json|`resize_array(size_t, T)`|<em>&#x2713;</em>|`resize(size_t, T)`
class basic_json|`get(const string_view_type&, T&&)`|<em>&#x2713;</em>|`get_with_default(const string_view_type&, T&&)`
class basic_json|`get(const string_view_type&)`|<em>&#x2713;</em>|`at(const string_view_type&)`
class basic_json|`has_key(const string_view_type&)`|<em>&#x2713;</em>|`contains(const string_view_type&)`
class basic_json|`has_member(const string_view_type&)`|<em>&#x2713;</em>|`contains(const string_view_type&)`
class basic_json|`remove_range(size_t, size_t)`|<em>&#x2713;</em>|`erase(const_object_iterator, const_object_iterator)`
class basic_json|`remove(const string_view_type&)`|<em>&#x2713;</em>|`erase(const string_view_type& name)`
class basic_json|`remove_member(const string_view_type&)`|<em>&#x2713;</em>|`erase(const string_view_type& name)`
class basic_json|`is_empty()`|<em>&#x2713;</em>|`empty()`
class basic_json|`is_numeric()`|<em>&#x2713;</em>|`is_number()`
class basic_json|`parse(const char_type*, size_t)`|<em>&#x2713;</em>|`parse(const string_view_type&)`
class basic_json|`parse_file(const std::basic_string<char_type,char_traits_type>&)`|<em>&#x2713;</em>|`parse(std::basic_istream<char_type>&)`
class basic_json|`parse_stream(std::basic_istream<char_type>&)`|<em>&#x2713;</em>|`parse(std::basic_istream<char_type>&)`
class basic_json|`parse_string(const string_view_type&)`|<em>&#x2713;</em>|`parse(const string_view_type&)`
class basic_json|`parse_string(const string_view_type&, std::function<bool(json_errc,const ser_context&)>)`|<em>&#x2713;</em>|`parse(parse(const string_view_type&, std::function<bool(json_errc,const ser_context&)`
class basic_json|`basic_json(double, uint8_t)`|<em>&#x2713;</em>|`basic_json(double)`
class basic_json|`basic_json(double, const floating_point_options&, semantic_tag)`| |`basic_json(double,semantic_tag)`
class floating_point_options|`floating_point_options`| |No replacement
class basic_json_fragment_filter|`basic_json_fragment_filter`| |No replacement


