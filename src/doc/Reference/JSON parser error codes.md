    namespace json_parser_error {
    enum json_parser_error_t 
    {
        expected_value_separator,
        unexpected_value_separator,
        unexpected_end_of_object,
        unexpected_end_of_array,
        expected_name,
        expected_value,
        expected_name_separator,
        unexpected_name_separator,
        illegal_control_character,
        illegal_escaped_character,
        invalid_codepoint_surrogate_pair,
        invalid_hex_escape_sequence,
        invalid_unicode_escape_sequence,
        invalid_number,
        unexpected_eof,
        eof_reading_string_value,
        eof_reading_numeric_value
    };
    }


