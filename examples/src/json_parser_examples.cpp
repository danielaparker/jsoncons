// Copyright 2018 Daniel Parker
// Distributed under Boost license

#include <string>
#include <sstream>
#include <jsoncons/json.hpp>
#include <jsoncons/json_parser.hpp>

using namespace jsoncons;

void incremental_parsing_example()
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser(decoder);
    try
    {
        parser.update("[fal",4);
        parser.parse_some();

        std::cout << "(1) done: " << std::boolalpha << parser.done() << "\n";
        std::cout << "(2) source_exhausted: " << std::boolalpha << parser.source_exhausted() << "\n\n";

        parser.update("se]",3);

        parser.parse_some();

        std::cout << "(3) done: " << std::boolalpha << parser.done() << "\n";
        std::cout << "(4) source_exhausted: " << std::boolalpha << parser.source_exhausted() << "\n\n";

        parser.end_parse();

        json j = decoder.get_result();
        std::cout << "(5) " << j << "\n";
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void json_parser_examples()
{
    std::cout << "\njson_parser examples\n\n";
    incremental_parsing_example();

    std::cout << std::endl;
}



