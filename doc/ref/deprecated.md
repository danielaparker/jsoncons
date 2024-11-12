## Deprecated Features

As the `jsoncons` library has evolved, names have sometimes changed. To ease transition, jsoncons deprecates the old names but continues to support many of them. The deprecated names can be suppressed by defining macro JSONCONS_NO_DEPRECATED, which is recommended for new code.

Category/class|Old name|Replacement
--------|-----------|--------------
__corelib__|&nbsp;|&nbsp;
`basic_json_parser`|&nbsp;|&nbsp;
&nbsp;|`basic_json_parser(std::function<bool(json_errc,const ser_context&)>, const TempAllocator&`|Set error handler in options
&nbsp;|`basic_json_parser(const basic_json_decode_options<char_type>&,std::function<bool(json_errc,const ser_context&)>,`const TempAllocator&`|Set error handler in options



