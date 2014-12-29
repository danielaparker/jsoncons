    jsoncons::json_parser_errc

The enumeration values scoped by `jsoncons::json_parser_errc` define the values for json parser error codes.

### Header

    #include "jsoncons/parse_error_handler.hpp"

### Member constants

constant                            |Description
------------------------------------|------------------------------
`unexpected_value_separator`        |Unexpected value separator ','     
`expected_value_separator`          |Expected value separator ','       
`unexpected_end_of_object`          |Unexpected end of object '}'       
`unexpected_end_of_array`           |Unexpected end of array ']'        
`expected_name`                     |Expected name                      
`expected_value`                    |Expected value                     
`unexpected_name_separator`         |Unexpected name separator '`'      
`expected_name_separator`           |Expected name separator '`'        
`illegal_control_character`         |Illegal control character in string
`illegal_escaped_character`         |Illegal escaped character in string
`invalid_codepoint_surrogate_pair`  |Invalid codepoint, expected another \\u token to begin the second half of a codepoint surrogate pair.
`invalid_hex_escape_sequence`       |Invalid codepoint, expected hexadecimal digit.
`invalid_unicode_escape_sequence`   |Invalid codepoint, expected four hexadecimal digits.
`invalid_number`                    |Invalid number
`unexpected_eof`                    |Unexpected end of file
`eof_reading_string_value`          |Reached end of file while reading string value
`eof_reading_numeric_value`         |Reached end of file while reading numeric value



