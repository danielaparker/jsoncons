    jsoncons::json_filter

    typedef basic_json_filter<char> json_filter

The `json_filter` class is an instantiation of the `basic_json_filter` class template that uses `char` as the character type.

### Header

    #include "jsoncons/json_filter.hpp"

### Base classes

[json_input_handler](json_input_handler)

### Constructors

    json_filter(json_input_handler& parent)
You must ensure that the parent input handler exists as long as `json_filter` does, as `json_filter` holds a pointer to but does not own this object.

    json_filter(json_output_handler& output_handler)
You must ensure that the parent output handler exists as long as `json_filter` does, as `json_filter` holds a pointer to but does not own this object.

### Accessors

    json_input_handler& parent()
Returns a reference to the parent input handler. 
