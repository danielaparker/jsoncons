// Copyright 2013-2024 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <iostream>

#if JSONCONS_VERSION_MAJOR == 0 && JSONCONS_VERSION_MINOR < 179

void incremental_parsing_example()
{
    jsoncons::json_decoder<jsoncons::json> decoder;
    jsoncons::json_parser parser;
    try
    {
        parser.update("[fal");
        parser.parse_some(decoder);
        std::cout << "(1) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        parser.update("se,");
        parser.parse_some(decoder);
        std::cout << "(2) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        parser.update("9");
        parser.parse_some(decoder);
        std::cout << "(3) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        parser.update("0]");
        parser.parse_some(decoder);
        std::cout << "(4) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        parser.finish_parse(decoder);
        std::cout << "(5) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        parser.check_done();
        std::cout << "(6) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        jsoncons::json j = decoder.get_result();
        std::cout << "(7) " << j << "\n\n";
    }
    catch (const jsoncons::ser_error& e)
    {
        std::cout << e.what() << '\n';
    }
}

#else

void incremental_parsing_example()
{
    std::vector<std::string> chunks = {"[fal", "se,", "9", "0]"};
    std::size_t index = 0;

    auto read_chunk = [&](jsoncons::parser_input& input, std::error_code& /*ec*/) -> bool
    {
        if (index < chunks.size())
        {
            input.set_buffer(chunks[index].data(), chunks[index].size());
            ++index;
            return true;
        }
        else
        {
            return false;
        }
    };

    jsoncons::json_decoder<jsoncons::json> decoder;
    jsoncons::json_parser parser{read_chunk};

    parser.reset();
    try
    {
        parser.parse_some(decoder);
        std::cout << "(1) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";
        parser.finish_parse(decoder);
        std::cout << "(2) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";
        parser.check_done();
        std::cout << "(3) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        jsoncons::json j = decoder.get_result();
        std::cout << "(4) " << j << "\n\n";
    }
    catch (const jsoncons::ser_error& e)
    {
        std::cout << e.what() << '\n';
    }
}

#endif

void parse_nan_replacement_example()
{
    std::string s = R"(
        {
           "A" : "NaN",
           "B" : "Infinity",
           "C" : "-Infinity"
        }
    )";

    auto options = jsoncons::json_options{}
        .nan_to_str("NaN")
        .inf_to_str("Infinity");

    jsoncons::json_decoder<jsoncons::json> decoder;
    jsoncons::json_parser parser(options);
    try
    {
#if JSONCONS_VERSION_MAJOR == 0 && JSONCONS_VERSION_MINOR < 179
        parser.update(s);     // until 1.0.0
#else        
        parser.set_buffer(s.data(), s.size());   // since 1.0.0
#endif        
        parser.parse_some(decoder);
        parser.finish_parse(decoder);
        parser.check_done();
    }
    catch (const jsoncons::ser_error& e)
    {
        std::cout << e.what() << '\n';
    }

    jsoncons::json j = decoder.get_result(); // performs move
    if (j["A"].is<double>())
    {
        std::cout << "A: " << j["A"].as<double>() << '\n';
    }
    if (j["B"].is<double>())
    {
        std::cout << "B: " << j["B"].as<double>() << '\n';
    }
    if (j["C"].is<double>())
    {
        std::cout << "C: " << j["C"].as<double>() << '\n';
    }
}

int main()
{
    std::cout << "\njson_parser examples\n\n";
    
    incremental_parsing_example();
    parse_nan_replacement_example();

    std::cout << '\n';
}



