Parameter          | Description        | Default       
-------------      | -------------  | ------------- 
assume_header      | Assume first row in file is header, use field names to construct objects | false         
header_lines      | Number of header lines in the CSV text | 0         
header            | A string containing delimited names corresponding to the fields in the file | Empty
max_lines         | Maximum number of lines to read | Unlimited
line_delimiter|String to write between records|\n  
field_delimiter    | Field separator              | ,             
quote_char         | Quote character              | "             
quote_escape_char  | Character to escape quote character (by default the quote character is doubled)| "             
quote_style|all, minimal, none, or nonnumeric|minimal
comment_starter|Character to comment out a line, must be at column 1.|None
