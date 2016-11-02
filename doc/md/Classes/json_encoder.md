```c++
jsoncons::json_encoder

template <class Json>
json_encoder
```

### Header

    #include <jsoncons/json_encoder.hpp>

### Base classes

[json_input_handler](json_input_handler)

### Member types

Member type                         |Definition
------------------------------------|------------------------------
`json_type`|Json
`allocator_type`|Json::allocator_type

### Constructors

    json_encoder(const allocator_type allocator = allocator_type())

### Member functions

    allocator_type get_allocator() const
Returns the allocator associated with the json value.

    bool is_valid() const
Checks if the `deserializer` contains a valid json_type value. The initial `is_valid()` is true (the value is an empty object), becomes false when a `do_begin_json` event is received, becomes `true` when a `do_end_json` event is received, and becomes false when `get_result()` is called.

    json_type get_result()
Returns the json value `v` stored in the `deserializer` as `std::move(v)`. If before calling this function `is_valid()` is false, the behavior is undefined. After `get_result()` is called, 'is_valid()' becomes false.

### Deprecated methods

    json_type& root()
Use the public member function `get_result()` instead
