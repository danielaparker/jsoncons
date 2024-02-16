## Deprecated Features

As the `jsoncons` library has evolved, names have sometimes changed. To ease transition, jsoncons deprecates the old names but continues to support many of them. The deprecated names can be suppressed by defining macro JSONCONS_NO_DEPRECATED, which is recommended for new code.

Related class|Old name|Replacement
--------|-----------|--------------
`json_decoder`|&nbsp;|&nbsp;
&nbsp;`json_decoder(result_allocator_arg_t, const allocator_type& alloc, const temp_allocator_type& temp_alloc)`|`json_decoder(const allocator_type& alloc, const temp_allocator_type& temp_alloc)`
&nbsp;|`root()`|`get_result()`
