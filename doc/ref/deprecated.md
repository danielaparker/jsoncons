## Deprecated Features

As the `jsoncons` library has evolved, names have sometimes changed. To ease transition, jsoncons deprecates the old names but continues to support many of them. The deprecated names can be suppressed by defining macro JSONCONS_NO_DEPRECATED, which is recommended for new code.

In the table, <em>&#x2713;</em> indicates that the old name is still supported.

Component or location|Old name, now deprecated|<em>&#x2713;</em>|Replacement
--------|-----------|--------------|------------------------
class basic_json|value_type|<em>&#x2713;</em>|No replacement
class basic_json|json_type|<em>&#x2713;</em>|No replacement
class basic_json|string_type|<em>&#x2713;</em>|No replacement
class basic_json|kvp_type|<em>&#x2713;</em>|key_value_type
class basic_json|member_type|<em>&#x2713;</em>|key_value_type
class basic_json|as_integer() with default template argument|<em>&#x2713;</em>|as<int64_t>()
class basic_json|get_semantic_tag()|<em>&#x2713;</em>|tag()
class basic_json|is_datetime()|<em>&#x2713;</em>|tag() == semantic_tag::datetime
class basic_json|is_epoch_time()|<em>&#x2713;</em>|tag() == semantic_tag::timestamp
class basic_json|add(T&&)|<em>&#x2713;</em>|push_back(T&&)
class basic_json|add(const_array_iterator, T&&)|<em>&#x2713;</em>|insert(const_array_iterator, T&&)


