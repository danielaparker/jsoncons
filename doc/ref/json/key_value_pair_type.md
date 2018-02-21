### jsoncons::json::key_value_pair_type 

`key_value_pair_type ` stores a key (name) and a json value

#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`string_view_type`|A non-owning view of a string, holds a pointer to character data and length. Supports conversion to and from strings. Will be typedefed to the C++ 17 [string view](http://en.cppreference.com/w/cpp/string/basic_string_view) if `JSONCONS_HAS_STRING_VIEW` is defined in `jsoncons_config.hpp`, otherwise proxied.  

#### Accessors
    
    string_view_type key() const

    const json& value() const

    json& value()


