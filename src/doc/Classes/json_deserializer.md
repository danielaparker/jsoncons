    jsoncons::json_deserializer

    typedef basic_json_deserializer<char,std::allocator<void>> json_deserializer

The `json_deserializer` class is an instantiation of the `basic_json_deserializer` class template that uses `char` as the character type.

### Header

    #include "jsoncons/json_deserializer.hpp"

### Base classes

[json_input_handler](json_input_handler)

### Constructors

    json_deserializer()

### Accessors

    json& root()
Returns a reference to the root of the json value read from the stream
