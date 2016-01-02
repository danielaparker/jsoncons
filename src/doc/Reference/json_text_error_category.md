    jsoncons::json_parser_errc

The constant integer values scoped by `jsoncons::json_parser_errc` define the values for json text error codes.

### Header

    #include "jsoncons/json_error_category.hpp"

### Member constants

constant                            |Description
------------------------------------|------------------------------
`unexpected_eof`                    |Unexpected end of file
`invalid_json_text`                    |A JSON text must be an object or array
`extra_character`          |Unexpected non-whitespace character after JSON text
`max_depth_exceeded`         |Maximum stack depth exceeded
`mismatched_parentheses_or_brackets`        |Mismatched parentheses or brackets      
`single_quote`        |JSON strings cannot be quoted with single quotes
`illegal_character_in_string`        |Illegal character in string
`extra_comma`        |Extra comma      
`expected_comma_or_end`          |Expected comma or end       
`unexpected_end_of_object`          |Unexpected end of object '}'       
`unexpected_end_of_array`           |Unexpected end of array ']'        
`expected_name`                     |Expected object member name
`expected_value`                    |Expected value                     
`expected_colon`           |Expected colon        
`illegal_control_character`         |Illegal control character in string
`illegal_escaped_character`         |Illegal escaped character in string
`expected_codepoint_surrogate_pair`  |Invalid codepoint, expected another \\u token to begin the second half of a codepoint surrogate pair.
`invalid_hex_escape_sequence`       |Invalid codepoint, expected hexadecimal digit.
`invalid_unicode_escape_sequence`   |Invalid codepoint, expected four hexadecimal digits.
`invalid_number`                    |Invalid number
`leading_zero`                    |A number cannot have a leading zero



