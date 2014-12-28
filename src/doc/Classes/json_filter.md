    jsoncons::json_filter

    typedef basic_json_filter<char> json_filter

The `json_filter` class is an instantiation of the `basic_json_filter` class template that uses `char` as the character type.

### Header

    #include "jsoncons/json_filter.hpp"

### Base classes

[json_input_handler](json_input_handler)

### Constructors

    json_filter(json_input_handler& handler)
    json_filter(json_input_handler& handler,
                 parse_error_handler& err_handler)
All JSON events that pass through the `json_filter` go to the specified `json_input_handler` (i.e. another filter.)
You must ensure that the `handler` exists as long as does `json_filter`, as `json_filter` holds a pointer to but does not own this object.

    json_filter(json_output_handler& handler)
    json_filter(json_output_handler& handler,
                 parse_error_handler& err_handler)
All JSON events that pass through the `json_filter` go to the specified `json_output_handler`.
You must ensure that the `handler` exists as long as does `json_filter`, as `json_filter` holds a pointer to but does not own this object.

### Accessors

    json_input_handler& input_handler()
Returns a reference to the JSON event handler. 

### Deprecated methods

    json_input_handler& parent()
Replaced by `input_handler`. 

