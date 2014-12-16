    namespace json_parser_error {
        enum json_parser_error_t 
        {
            unexpected_eof,
            unexpected_value_separator,
            expected_value_separator,
            unexpected_end_of_object,
            unexpected_end_of_array,
            unrecognized_value,
            expected_name_separator,
            illegal_control_character,
            illegal_escaped_character,
            invalid_codepoint_surrogate_pair,
            invalid_hex_escape_sequence,
            invalid_unicode_escape_sequence,
            invalid_number,
            value_not_found,
            eof_reading_string_value,
            eof_reading_numeric_value
        };
    }


