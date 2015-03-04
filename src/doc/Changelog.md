0.97.1 Release
--------------

- "Transforming JSON with filters" example fixed
- Added a class-specific in-place new to the json class that is implemented in terms of the global version (required to create json objects with placement new operator.)
- Reorganized header files, removing unnecessary includes. 
- Incorporates validation contributed by Alex Merry for ensuring that there is an object or array on parse head.
- Incorporates fix contributed by Milan Burda for “Switch case is in protected scope” clang build error

0.97 Release
------------

- Reversion of 0.96 change:

The virtual methods `do_float_value`, `do_integer_value`, and `do_unsigned_value` of `json_input_handler` and `json_outputhandler` have been restored to `do_double_value`, `do_longlong_value` and `do_ulonglong_value`, and their typedefed parameter types `float_type`, `integer_type`, and `unsigned_type` have been restored to `double`, `long long`, and `unsigned long long`.

The rationale for this reversion is that the change doesn't really help to make the software more flexible, and that it's better to leave out the typedefs. There will be future enhancements to support greater numeric precision, but these will not affect the current method signatures.

- Fix for "unused variable" warning message

0.96 Release
------------

This release includes breaking changes to interfaces. Going forward, the interfaces are expected to be stable.

Breaking changes:

- Renamed `error_handler` to `parse_error_handler`.

- Renamed namespace `json_parser_error` to `json_parser_errc`

- Renamed `value_adapter` to `json_type_traits`, if you have implemented your own type specializations,
  you will have to rename `value_adapter` also.

- Only json arrays now support `operator[](size_t)` to loop over values, this is no longer supported for `json` objects. Use a json object iterator instead.

- The virtual methods `do_double_value`, `do_longlong_value` and `do_ulonglong_value` of `json_input_handler` and `json_outputhandler` have been renamed to `do_float_value`, `do_integer_value`, and `do_unsigned_value`, 
  and their parameters have been changed from `double`, `long long`, and `unsigned long long` to typedefs `float_type`, `integer_type`, and `unsigned_type`.
  The rationale for this change is to allow different configurations for internal number types (reversed in 0.97.)

General changes

- `json` member function `begin_object` now returns a bidirectional iterator rather than a random access iterator.

- Static singleton `instance` methods have been added to `default_parse_error_handler`
  and `empty_json_input_handler`. 

- Added to the `json` class overloaded static methods parse, parse_string 
  and parse_file that take a `parse_error_handler` as a parameter. 

- Added methods `last_char()` and `eof()` to `parsing_context`.

- Enhancements to json parsing and json parse event error notification.

- Added to `json_input_handler` and `json_output_handler` a non virtual method `value` that takes a null terminated string.

- Added methods `is_integer`, `is_unsigned` and `is_float` to `json` to replace `is_longlong`, `is_ulonglong` and `is_double`, which have been deprecated.

- Added methods `as_integer`, `as_unsigned` and `as_float` to `json` to replace `is_longlong`, `is_ulonglong` and `is_double`, which have been deprecated.

Bug fixes:

- Fixed issue with column number reported by json_reader

- Where &s[0] and s.length() were passed to methods, &s[0] has been replaced with s.c_str(). 
  While this shouldn't be an issue on most implementations, VS throws an exception in debug mode when the string has length zero.

- Fixes two issues in 0.95 reported by Alex Merry that caused errors with GCC: a superfluous typename has been removed in csv_serializer.hpp, and a JSONCONS_NOEXCEPT specifier has been added to the json_parser_category_impl name method.

- Fixed a number of typename issues in the 0.96 candidate identifed by Ignatov Serguei.

- Fixes issues with testsuite cmake and scons reported by Alex Merry and Ignatov Serguei

0.95
----

Enhancements:

- Added template method `any_cast` to `json` class.

- The allocator type parameter in basic_json is now supported, it allows you to supply a 
  custom allocator for dynamically allocated, fixed size small objects in the json container.
  The allocator type is not used for structures including vectors and strings that use large 
  or variable amounts of memory, these always use the default allocators.

Non-breaking Change:
 
- `json_filter` method `parent` has been renamed to `input_handler` (old name still works) 

Breaking change (if you've implemented your own input and output handlers, or if you've
passed json events to input and output handlers directly):

- The input handler virtual method 
  `name(const std::string& name, const parsing_context& context)` 
  has been changed to
  `do_name(const char* p, size_t length, const parsing_context& context)` 

- The output handler virtual method 
  `name(const std::string& name)` 
  has been changed to
  `do_name(const char* p, size_t length)` 

- The input handler virtual method 
  `string_value(const std::string& value, const parsing_context& context)` 
  has been changed to
  `do_string_value(const char* p, size_t length, const parsing_context& context)` 

- The output handler virtual method 
  `string_value(const std::string& value)` 
  has been changed to
  `do_string_value(const char* p, size_t length)` 

The rationale for the method parameter changes is to allow different internal
representations of strings but preserve efficiency. 

- The input and output handler virtual implementation methods begin_json, end_json,
  begin_object, end_object, begin_array, end_array, name, string_value, 
  longlong_value, ulonglong_value, double_value, bool_value and null_value 
  have been renamed to do_begin_json, do_end_json, do_begin_object, do_end_object, 
  do_begin_array, do_end_array, do_name, do_string_value, do_longlong_value, 
  do_ulonglong_value, do_double_value, do_bool_value and do_null_value and have been 
  made private. 
  
- Public non-virtual interface methods begin_json, end_json,
  begin_object, end_object, begin_array, end_array, name
  have been added to json_input_handler and json_output_handler. 

The rationale for these changes is to follow best C++ practices by making the
json_input_handler and json_output_handler interfaces public non-virtual and 
the implementations private virtual. Refer to the documentation and tutorials for details.   

- The error_handler virtual implementation methods have been renamed to `do_warning` and 
  `do_error`, and made private. Non virtual public interface methods `warning` and `error` 
  have been added. Error handling now leverages `std::error_code` to communicate parser 
  error events in an extendable way.

Bug fixes:

- Fixed bug in csv_reader

0.94.1
------

Bug fixes:

- Incorporates fix from Alex Merry for comparison of json objects

0.94
----

Bug fixes 

- Incorporates contributions from Cory Fields for silencing some compiler warnings
- Fixes bug reported by Vitaliy Gusev in json object operator[size_t]
- Fixes bug in json is_empty method for empty objects

Changes

- json constructors that take string, double etc. are now declared explicit (assignments and defaults to get and make_array methods have their own implementation and do not depend on implicit constructors.)
- make_multi_array renamed to make_array (old name is still supported)
- Previous versions supported any type values through special methods set_custom_data, add_custom_data, and custom_data. This version introduces a new type json::any that wraps any values and works with the usual accessors set, add and as, so the specialized methods are no longer required.

Enhancements 

- json get method with default value now accepts extended types as defaults
- json make_array method with default value now accepts extended types as defaults

New extensions

- Added jsoncons_ext/boost/type_extensions.hpp to collect 
  extensions traits for boost types, in particular, for
  boost::gregorian dates.

0.93 Release
------------

New features

- Supports wide character strings and streams with wjson, wjson_reader etc. Assumes UTF16 encoding if sizeof(wchar_t)=2 and UTF32 encoding if sizeof(wchar_t)=4.
- The empty class null_type  is added to the jsoncons namespace, it replaces the member type json::null_type (json::null_type is typedefed to jsoncons::null_type for backward compatibility.)

Defect fixes:

- The ascii character 0x7f (del) was not being considered a control character to be escaped, this is fixed.
- Fixed two issues with serialization when the output format property escape_all_non_ascii is enabled. One, the individual bytes were being checked if they were non ascii, rather than first converting to a codepoint. Two, continuations weren't being handled when decoding.

0.92a Release
-------------

Includes contributed updates for valid compilation and execution in gcc and clang environments

0.92 Release
------------

Breaking change (but only if you have subclassed json_input_handler or json_output_handler)

- For consistency with other names, the input and output handler methods new to 0.91 - value_string, value_double, value_longlong, value_ulonglong and value_bool - have been renamed to string_value, double_value, longlong_value, ulonglong_value and bool_value.

Non breaking changes (previous features are deprecated but still work)

- name_value_pair has been renamed to member_type (typedefed to previous name.)

- as_string(output_format format) has been deprecated, use the existing to_string(output_format format) instead

Enhancements:

- json now has extensibilty, you can access and modify json values with new types, see the tutorial Extensibility 

Preparation for allocator support:

- The basic_json and related classes now have an Storage template parameter, which is currently just a placeholder, but will later provide a hook to allow users to control how json storage is allocated. This addition is transparent to users of the json and related classes.

0.91 Release
------------

This release should be largely backwards compatible with 0.90 and 0.83 with two exceptions: 

1. If you have used object iterators, you will need to replace uses of std::pair with name_value_pair, in particular, first becomes name() and second becomes value(). 

2. If you have subclassed json_input_handler, json_output_handler, or json_filter, and have implemented value(const std::string& ...,  value(double ..., etc., you will need to modify the names to  value_string(const std::string& ...,  value_double(double ... (no changes if you are feeding existing implementations.)

The changes are

- Replaced std::pair<std::string,json> with name_value_pair that has accessors name() and value()

- In json_input_handler and json_output_handler, allowed for overrides of the value methods by making them non-virtual and adding virtual methods value_string, value_double, value_longlong, value_ulonglong, and value_bool

Other new features:

- Changed implementation of is<T> and as<T>, the current implementation should be user extensible

- make_multi_array<N> makes a multidimensional array with the number of dimensions specified as a template parameter. Replaces make_2d_array and make_3d_array, which are now deprecated.

- Added support for is<std::vector<T>> and as<std::vector<T>>

- Removed JSONCONS_NO_CXX11_RVALUE_REFERENCES, compiler must support move semantics

Incorporates a number of contributions from Pedro Larroy and the developers of the clearskies_core project:

- build system for posix systems
- GCC to list of supported compilers
- Android fix
- fixed virtual destructors missing in json_input_handler, json_output_handler and parsing_context
- fixed const_iterator should be iterator in json_object implementation 

To clean up the interface and avoid too much duplicated functionality, we've deprecated some json methods (but they still work)

    make_array
Use json val(json::an_array) or json::make_multi_array<1>(...) instead (but make_array will continue to work)

    make_2d_array
    make_3d_array
Use make_multi_array<2> and make_multi_array<3> instead

    as_vector
Use as<std::vector<int>> etc. instead

    as_int
    as_uint
    as_char
Use as<int>, as<unsigned int>, and as<char> instead

Release 0.90a
-------------

Fixed issue affecting clang compile

Release 0.90
------------

This release should be fully backwards compatible with 0.83. 

Includes performance enhancements to json_reader and json_deserializer

Fixes issues with column numbers reported with exceptions

Incorporates a number of patches contributed by Marc Chevrier:

- Fixed issue with set member on json object when a member with that name already exists
- clang port
- cmake build files for examples and test suite
- json template method is<T> for examining the types of json values
- json template method as<T> for accessing json values

Release 0.83
------------

Optimizations (very unlikely to break earlier code)

- get(const std::name& name) const now returns const json& if keyed value exists, otherwise a const reference to json::null

- get(const std::string& name, const json& default_val) const now returns const json (actually a const proxy that evaluates to json if read)

Bug fixes

- Line number not incremented within multiline comment - fixed

Deprecated features removed

- Removed deprecated output_format properties (too much bagage to carry around)

Release 0.82a
-------------

- The const version of the json operator[](const std::string& name) didn't need to return a proxy, the return value has been changed to const json& (this change is transparent to the user.) 

- get(const std::name& name) has been changed to return a copy (rather than a reference), and json::null if there is no member with that name (rather than throw.) This way both get methods return default values if no keyed value exists.

- non-const and const methods json& at(const std::name& name) have been added to replace the old single argument get method. These have the same behavior as the corresponding operator[] functions, but the non-const at is more efficient.

Release 0.81
------------

- Added accessor and modifier methods floatfield to output_format to provide a supported way to set the floatfield format flag to fixed or scientific with a specified number of decimal places (this can be done in older versions, but only with deprecated methods.)

- The default constructor now constructs an empty object (rather than a null object.) While this is a change, it's unlikely to break exisitng code (all test cases passed without modification.)

This means that instead of

    json obj(json::an_object);
    obj["field"] = "field";

you can simply write

    json obj;
    obj["field"] = "field";

The former notation is still supported, though.

- Added a version of 'resize_array' to json that resizes the array to n elements and initializes them to a specified value.

- Added a version of the static method json::make_array that takes no arguments and makes an empty json array

Note that

    json arr(json::an_array);

is equivalent to

    json arr = json::make_array();

and

    json arr(json::an_array);
    arr.resize_array(10,0.0);

is equivalent to

    json arr = json::make_array(10,0.0);

For consistency the json::make_array notation is now favored in the documentation. 

Release 0.71
-------------

- Added resize_array method to json for resizing json arrays 

- Fixed issue with remove_range method (templated code failed to compile if calling this method.)

- Added remove_member method to remove a member from a json object

- Fixed issue with multiline line comments, added test case

- Fixed issue with adding custom data to a json array using add_custom_data, added examples.

Release 0.70
-------------

- Since 0.50, jsoncons has used snprintf for default serialization of double values to string values. This can result in invalid json output when running on a locale like German or Spanish. The period character (â€˜.â€™) is now always used as the decimal point, non English locales are ignored.

- The output_format methods that support alternative floating point formatting, e.g. fixed, have been deprecated.

- Added a template method as_vector<T> to the json class. If a json value is an array and conversion is possible to the template type, returns a std::vector of that type, otherwise throws an std::exception. Specializations are provided for std::string, bool, char, int, unsigned int, long, unsigned long, long long, unsigned long long, and double. For example

    std::string s("[0,1,2,3]");

    json val = json::parse_string(s);

    std::vector<int> v = val.as_vector<int>(); 

- Undeprecated the json member function precision

Release 0.60b
-------------

This release (0.60b) is fully backwards compatible with 0.50.

A change introduced with 0.60 has been reversed. 0.60 introduced an alternative method of constructing a json arrray or object with an initial default constructor, a bug with this was fixed in 0.60a, but this feature and related documentation has been removed because it added complexity but no real value.

### Enhancements

- Added swap member function to json

- Added add and add_custom_data overrides to json that take an index value, for adding a new element at the specified index and shifting all elements currently at or above that index to the right.

- Added capacity member functions to json

### 0.60  extensions

- csv_serializer has been added to the csv extension

Release 0.50
------------

This release is fully backwards compatible with 0.4*, and mostly backwards compatible to 0.32 apart from the two name changes in 0.41

Bug fixes

- When reading the escaped characters "\\b", "\\f", "\\r" and "\\t" appearing in json strings, json_reader was replacing them with the linefeed character, this has been fixed.

Deprecated 

- Deprecated modifiers precision and fixed_decimal_places from output_format. Use set_floating_point_format instead.
- Deprecated constructor that takes indenting parameter from output_format. For pretty printing with indenting, use the pretty_print function or pass the indenting parameter in json_serializer.

Changes

- When serializing floating point values to a stream, previous versions defaulted to default floating point precision with a precision of 16. This has been changed to truncate trailing zeros  but keep one if immediately after a decimal point.

New features

- For line reporting in parser error messages, json_reader now recognizes \\r\\n, \\n alone or \\r alone (\\r alone is new.)
- Added set_floating_point_format methods to output_format to give more control over floating point notation.

Non functional enhancements

- json_reader now estimates the minimum capacity for arrays and objects, and reports that information for the begin_array and begin_object events. This greatly reduces reallocations.

Release 0.42
------------

- Fixed another bug with multi line /**/ comments 
- Minor fixes to reporting line and column number of errors
- Added fixed_decimal_places setter to output_format
- Added version of as_string to json that takes output_format as a parameter
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

Free function serialize replaces free function to_stream for
serializing custom data.

pretty print tidied up for nested arrays

Release 0.15
------------

Made eof() method on json_reader public, to support reading
multiple JSON objects from a stream.

Release 0.14
------------

Added pretty_print class

Renamed json_stream_writer to json_serializer, 
implements pure virtual class json_out_stream
 
Renamed json_stream_listener to json_deserializer
implements pure virtual class json_in_stream

Renamed json_parser to json_reader, parse to read.

Changed indenting so object and array members start on new line.

Added support for storing user data in json object, and
serializing to JSON.

Release 0.13
------------

Replaced simple_string union member with json_string that 
wraps std::basic_string<Char>

name() and value() event handler methods on 
basic_json_stream_writer take const std::basic_string<Char>&
rather than const Char* and length.

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


