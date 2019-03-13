// Copyright 2018 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>

namespace jc=jsoncons; // For brevity

void construct_json_byte_string()
{
    jc::byte_string bs = {'H','e','l','l','o'};

    // default suggested encoding (base64url)
    jc::json j1(bs);
    std::cout << "(1) "<< j1 << "\n\n";

    // base64 suggested encoding
    jc::json j2(bs, jc::semantic_tag_type::base64);
    std::cout << "(2) "<< j2 << "\n\n";

    // base16 suggested encoding
    jc::json j3(bs, jc::semantic_tag_type::base16);
    std::cout << "(3) "<< j3 << "\n\n";
}

void serialize_json_byte_string()
{
    jc::byte_string bs = {'H','e','l','l','o'};

    jc::json j(bs);

    // default
    std::cout << "(1) "<< j << "\n\n";

    // base16
    jc::json_options options2;
    options2.byte_string_format(jc::byte_string_chars_format::base16);
    std::cout << "(2) "<< print(j, options2) << "\n\n";

    // base64
    jc::json_options options3;
    options3.byte_string_format(jc::byte_string_chars_format::base64);
    std::cout << "(3) "<< print(j, options3) << "\n\n";

    // base64url
    jc::json_options options4;
    options4.byte_string_format(jc::byte_string_chars_format::base64url);
    std::cout << "(4) "<< print(j, options4) << "\n\n";
}

void byte_string_examples()
{
    serialize_json_byte_string();

    std::cout << "jc::byte_string examples" << "\n\n";
    construct_json_byte_string();
}


