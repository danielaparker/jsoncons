## Deprecated Features

As the `jsoncons` library has evolved, names have sometimes changed. To ease transition, jsoncons deprecates the old names but continues to support many of them. The deprecated names can be suppressed by defining macro JSONCONS_NO_DEPRECATED, which is recommended for new code.

Category/class|Old name|Replacement
--------|-----------|--------------
`pretty json`|&nbsp;|&nbsp;
&nbsp;|`encode_json` overloads that take a `jsoncons::indenting` argument|Use `_pretty` overloads instead
&nbsp;|`basic_json::dump`overloads that take a `jsoncons::indenting` argument|Use `_pretty` overloads instead
`allocator_set`|&nbsp;|&nbsp;
&nbsp;|`combine_allocators`|Use `make_alloc_set` 
&nbsp;|`temp_allocator_only`|Use `make_alloc_set` with first argument **temp_allocator_arg** 
`parsing`|&nbsp;|&nbsp;
&nbsp;|`strict_json_parsing`|Set `json_option::`allow_comments`to **false**  
&nbsp;|`allow_trailing_commas`|Set `json_option::`allow_trailing_commas`to **true**  
`basic_json_options`|&nbsp;|&nbsp;
&nbsp;|`bigint_chars_format`|`bignum_chars_format`
&nbsp;|`bigint_chars_format::number`|`bignum_chars_format::raw`
&nbsp;|`err_handler`|Use the `allow_trailing_comma` and `allow_comments` options instead
`basic_json_parser`|&nbsp;|&nbsp;
&nbsp;|`basic_json_parser` constructor overloads that take an `err_handler` argument|Use the `allow_trailing_comma` and `allow_comments` options instead
&nbsp;|`basic_json_reader` constructor overloads that take an `err_handler` argument|Use the `allow_trailing_comma` and `allow_comments` options instead
&nbsp;|`basic_json_cursor` constructor overloads that take an `err_handler` argument|Use the `allow_trailing_comma` and `allow_comments` options instead
`csv::basic_csv_parser`|&nbsp;|&nbsp;
&nbsp;|`csv::basic_csv_parser` constructor overloads that take an `err_handler` argument|
&nbsp;|`csv::basic_csv_reader` constructor overloads that take an `err_handler` argument|
&nbsp;|`csv::basic_csv_cursor` constructor overloads that take an `err_handler` argument|
`csv::result_options`|&nbsp;|&nbsp;
&nbsp;|**csv::result_options::value**|Use `csv::result_options{}` instead


