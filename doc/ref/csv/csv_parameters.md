### jsoncons::csv::csv_parameters

Parameter          | Description        | Default       
-------------      | -------------  | ------------- 
assume_header      | Assume first row in file is header, use field names to construct objects | false         
infer_types      | Infer null, true, false, integers and floating point values in the CSV source | true         
header_lines      | Number of header lines in the CSV text | 1 if assume_header is true, otherwise 0         
column_names      | A comma separated list of names corresponding to the fields in the file | "bool-field,float-field,string-field"
column_types      | A comma separated list of data types corresponding to the columns in the file. The following data types are supported: string, integer, float and boolean | "bool,float,string"}
column_defaults      | A comma separated list of strings containing default json values corresponding to the columns in the file. | "false,0.0,"\"\""
comment_starter|Character to comment out a line, must be at column 1.|None
field_delimiter    | Field separator              | ,             
ignore_empty_values      | Do not output name-value pairs with empty values| false         
ignore_empty_lines      | If set to true, all lines in the file that are empty (apart from record delimiter characters) are ignored. To ignore lines with only spaces or tabs, set trim to true.| true         
line_delimiter|String to write between records|\n  
mapping|mapping_type::n_rows, mapping_type::n_objects, mapping_type::m_columns|If assume_header is true or column_names is not empty, mapping_type::n_rows, mapping_type::n_columns otherwise
max_lines         | Maximum number of lines to read | Unlimited
quote_char         | Quote character              | "             
quote_escape_char  | Character to escape quote character (by default the quote character is doubled)| "             
quote_style|quote_style_type::all, quote_style_type::minimal, quote_style_type::none, or quote_style_type::nonnumeric|quote_style_type::minimal
trim_leading      | Trim leading whitespace | false         
trim_trailing      | Trim trailing whitespace | false         
trim      | Trim both leading and trailing whitespace | false        
trim_leading_inside_quotes      | Trim leading whitespace inside quote characters| false         
trim_trailing_inside_quotes      | Trim trailing whitespace inside quote characters| false         
trim_inside_quotes      | Trim both leading and trailing whitespace inside quote characters| false        
unquoted_empty_value_is_null | Replace empty field with json null value | false         

