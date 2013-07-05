jsoncons
========

The jsoncons library implements C++ classes for the construction of JavaScript Object Notation (JSON). 

The library is header-only: it consists solely of header files containing templates and inline functions, and requires no separately-compiled library binaries when linking. It has no dependence on other libraries.

It has been tested with MS Visual C++ 10, SP1

Release 0.17
------------

Added is_custom() method to json and proxy

Added clear() method to json and proxy

set_member() method renamed to insert_member()

set_custom() method renamed to insert_custom_member()

push_back() method renamed to add_element() in json and proxy

Added add_custom_element method() in json and proxy

Release 0.16
------------

Change to json_out_stream and json_serializer:

    void value(const custom_data& value)

removed.

Free function `serialize` replaces free function `to_stream` for
serializing custom data.

pretty print tidied up for nested arrays

Release 0.15
------------

Made eof() method on json_reader public, to support reading
multiple JSON objects from a stream.

Release 0.14
------------

Added `pretty_print` class

Renamed `json_stream_writer` to `json_serializer`, 
implements pure virtual class `json_out_stream`
 
Renamed `json_stream_listener` to `json_deserializer`
implements pure virtual class `json_in_stream`

Renamed `json_parser` to `json_reader`, `parse` to `read`.

Changed indenting so object and array members start on new line.

Added support for storing user data in `json` object, and
serializing to JSON.

Release 0.13
------------

Replaced `simple_string` union member with json_string that 
wraps `std::basic_string<Char>`

name() and value() event handler methods on 
basic_json_stream_writer take `const std::basic_string<Char>&`
rather than `const Char*` and `length`.

Release 0.12
------------

Implemented operator<< for json::proxy

Added to_stream methods to json::proxy

Release 0.11
------------

Added members to json_parser to access and modify the buffer capacity

Added checks when parsing integer values to determine overflow for 
long long and unsigned long long, and if overflow, parse them as
doubles.


