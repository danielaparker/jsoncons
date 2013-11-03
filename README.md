jsoncons
========

The jsoncons library implements C++ classes for the construction of [JavaScript Object Notation](http://www.json.org) (JSON). 

The library is header-only: it consists solely of header files containing templates and inline functions, and requires no separately-compiled library binaries when linking. It has no dependence on other libraries.

To install the jsoncons library, download the zip file, unpack the release, under `src` find the directory `jsoncons`, and copy it to your `include` directory. If you wish to use extensions, copy the `jsoncons_ext` directory as well. 

It has been tested with MS Visual C++ 10, SP1 and Intel C++ Studio XE 2013. Note that SP1 is required for VC++ 10, it fixes compiler bugs with move semantics.

Releases are on sourceforge https://sourceforge.net/projects/jsoncons/?source=navbar

Release 0.50
------------

This release is fully backwards compatible with 0.4*, and mostly backwards compatible to 0.32 apart from the two name changes in 0.41

Bug fixes

- When reading the escaped characters "\b", "\f", "\r" and "\t" appearing in json strings, json_reader was replacing them with the linefeed character, this has been fixed.

Deprecated 

- Deprecated modifiers `precision` and `fixed_decimal_places` from output_format. Use `set_floating_point_precision` instead.

- Deprecated constructor that takes `indenting` parameter from `output_format`. For pretty printing with indenting, use the `pretty_print` function or pass the `indenting` parameter in `json_serializer`.

Changes

- When serializing floating point values to a stream, previous versions defaulted to default floating point precision with a precision of 16. This has been changed to truncate trailing zeros  but keep one if immediately after a decimal point.

New features

- For line reporting in parser error messages, json_reader now recognizes \r\n, \n alone or \r alone.

- Added `set_floating_point_format` methods to `output_format` to give more control over floating point notation.

Non functional enhancements

- json_reader now estimates the minimum capacity for arrays and objects, and reports that information for the begin_array and begin_object events. This greatly reduces reallocations.

Release 0.42
------------

- Fixed another bug with multi line /**/ comments 
- Minor fixes to reporting line and column number of errors
- Added fixed_decimal_places setter to output_format
- Added version of `as_string` to `json` that takes `output_format` as a parameter
- Reorganization of test cases and examples in source tree

Release 0.41
------------

- Added non-member overload swap(json& a, json& b)
- Fixed bug with multi line /**/ comments 
- Added begin_json and end_json methods to json_output_handler
- json_deserializer should now satisfy basic exception safety (no leak guarantee)
- Moved csv_reader.hpp to jsoncons_ext/csv directory
- Changed csv_reader namespace to jsoncons_ext::csv
- json::parse_file no longer reads the entire file into memory before parsing
  (it now uses json_reader default buffering)

Release 0.40
------------

- json_listener renamed to json_input_handler
- json_writer renamed to json_output_handler

- Added json_filter class

- json get method that takes default argument now returns a value rather than a reference
- Issue in csv_reader related to get method issue fixed
- Issue with const json operator[] fixed
- Added as_char method to json
- Improved exception safety, some opportunites for memory leaks in the presence of exceptions removed

Release 0.33
------------

Added reserve method to json

Added static make_3d_array method to json

json_reader now configured for buffered reading

Added csv_reader class for reading CSV files and producing JSON events

Fixed bug with explicitly passing output_format in pretty_print.

Release 0.32
------------

Added remove_range method, operator== and  operator!= to proxy and json objects

Added static methods make_array and make_2d_array to json

Release 0.31
------------

error_handler method content_error renamed to error

Added error_code to warning, error and fatal_error methods of error_handler

Release 0.30
------------

json_in_stream renamed to json_listener

json_out_stream renamed to json_writer

Added buffer accessor method to parsing_context

Release 0.20
------------

Added parsing_context class for providing information about the
element being parsed.

error_handler methods take message and parsing_context parameters

json_in_stream handlers take parsing_context parameter

Release 0.19
------------

Added error_handler class for json_reader

Made json_exception a base class for all json exceptions

Added root() method to json_deserializer to get a reference to the json value

Removed swap_root() method from json_deserializer

Release 0.18
------------

Renamed serialize() class method to to_stream() in json  

Custom data serialization supported through template function specialization of serialize
(reverses change in 0.17)


Release 0.17
------------

Added is_custom() method to json and proxy

get_custom() method renamed to custom_data() in json and proxy

Added clear() method to json and proxy

set_member() method renamed to set()

set_custom() method renamed to set_custom_data()

push_back() method renamed to add() in json and proxy

Added add_custom_data method() in json and proxy

Custom data serialization supported through template class specialization of custom_serialization
(replaces template function specialization of serialize)

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


