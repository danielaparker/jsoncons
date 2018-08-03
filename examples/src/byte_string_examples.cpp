// Copyright 2018 Daniel Parker
// Distributed under Boost license

#include <stdexcept>
#include <string>
#include <jsoncons/json.hpp>

using namespace jsoncons;

void byte_string_chars_format_examples()
{
    std::vector<uint8_t> bytes = {'H','e','l','l','o'};

    json j(byte_string(bytes.data(),bytes.size()));

    // default
    std::cout << "(1) "<< j << "\n\n";

    // base16
    json_serializing_options options2;
    options2.byte_string_format(byte_string_chars_format::base16);
    std::cout << "(2) "<< print(j, options2) << "\n\n";

    // base64
    json_serializing_options options3;
    options3.byte_string_format(byte_string_chars_format::base64);
    std::cout << "(3) "<< print(j, options3) << "\n\n";

    // base64url
    json_serializing_options options4;
    options4.byte_string_format(byte_string_chars_format::base64url);
    std::cout << "(4) "<< print(j, options4) << "\n\n";
}

void byte_string_examples()
{
    std::cout << "byte_string examples" << "\n\n";
    byte_string_chars_format_examples();
}


