### jsoncons::csv::csv_serializing_options

```c++
typedef basic_csv_serializing_options<char> csv_serializing_options
```
Specifies options for serializing and deserializing csv data. The `csv_serializing_options` class is an instantiation of the `basic_csv_serializing_options` class template that uses `char` as the character type.

#### Header
```c++
#include <jsoncons_ext/csv/csv_serializing_options.hpp>
```

#### Constructors

    csv_serializing_options()
Constructs an `csv_serializing_options` with default values. 

#### Properties

Parameter          | Description        | Default       
-------------      | -------------  | ------------- 
floating_point_format|Overrides [floating point format}(../chars_format.md) when serializing csv from json.|For a floating point value that was previously decoded from json text, preserves the original format when serializing.For a floating point value that was directly inserted into a json value, serializes with [chars_format::general](chars_format.md).
precision|Overrides floating point precision when serializing csv from json. |The default, For a floating point value that was previously decoded from json text, preserves the original precision. The fefault, For a floating point value that was directly inserted into a json value, serializes with shortest representation.
assume_header      | Assume first row in file is header, use field names to construct objects | false         
infer_types      | Infer null, true, false, integers and floating point values in the CSV source | true         
header_lines      | Number of header lines in the CSV text | 1 if assume_header is true, otherwise 0         
column_names      | A comma separated list of names corresponding to the fields in the file | "bool-field,float-field,string-field"
column_types      | A comma separated list of data types corresponding to the columns in the file. The following data types are supported: string, integer, float and boolean | "bool,float,string"}
column_defaults      | A comma separated list of strings containing default json values corresponding to the columns in the file. | "false,0.0,"\"\""
comment_starter|Character to comment out a line, must be at column 1.|None
field_delimiter    | A delimiter character that indicates the end of a field        | ,             
ignore_empty_values      | Do not output name-value pairs with empty values| false         
ignore_empty_lines      | If set to true, all lines in the file that are empty (apart from record delimiter characters) are ignored. To ignore lines with only spaces or tabs, set trim to true.| true         
line_delimiter|String to write between records|\n  
mapping|mapping_type::n_rows, mapping_type::n_objects, mapping_type::m_columns|If assume_header is true or column_names is not empty, mapping_type::n_rows, mapping_type::n_columns otherwise
max_lines         | Maximum number of lines to read | Unlimited
quote_char         | Quote character              | "             
quote_escape_char  | Character to escape quote character (by default the quote character is doubled)| "             
quote_style|quote_style_type::all, quote_style_type::minimal, quote_style_type::none, or quote_style_type::nonnumeric|quote_style_type::minimal
subfield_delimiter    |A delimiter character that indicates the end of a single value in a multi-valued field|None
trim_leading      | Trim leading whitespace | false         
trim_trailing      | Trim trailing whitespace | false         
trim      | Trim both leading and trailing whitespace | false        
trim_leading_inside_quotes      | Trim leading whitespace inside quote characters| false         
trim_trailing_inside_quotes      | Trim trailing whitespace inside quote characters| false         
trim_inside_quotes      | Trim both leading and trailing whitespace inside quote characters| false        
unquoted_empty_value_is_null | Replace empty field with json null value | false         

