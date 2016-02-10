For next release (0.99):

- Full support for AllocatorAwareContainer concept for C++ 11 compilers, continued support for VC++ 2010 with conditional directives. 
- Include examples and test cases

For subsequent releases:

- Support [JSON Content Rules](http://www.ietf.org/id/draft-newton-json-content-rules-05.txt) for schema validation in `jsoncons_ext`

- Add extensions for reading from and serializing to binary formats (bson,bjson,ubjson) 

- Revisit error codes, parsing_context, and opportunities for error recovery. Attempt to support [json 5](http://json5.org/) behaviour through an parse_error_handler and a json_filter.

