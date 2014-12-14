    jsoncons::json_filter

    typedef basic_json_filter<char> json_filter

The `json_filter` class is an instantiation of the `basic_json_filter` class template that uses `char` as the character type.

### Header

    #include "jsoncons/json_filter.hpp"

### Base classes

[json_input_handler](json_input_handler)

### Constructors

    json_filter(json_input_handler& parent)

    json_filter(json_output_handler& output_handler)

### Accessors

    json_input_handler& parent()
Returns a reference to the parent input handler. 
