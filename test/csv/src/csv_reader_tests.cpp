// Copyright 2013-2024 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>
#include <catch/catch.hpp>

namespace csv = jsoncons::csv; 

TEST_CASE("test csv_reader buffered read")
{
    SECTION("test 1")
    {
        auto j_expected = jsoncons::ojson::parse(R"(
{"Date":["2017-01-09","2017-01-08","2017-01-07"],"ProductType":["UST","UST","UST"],"1Y":[0.0062,0.0063,0.0061],"2Y":[0.0075,0.0076,0.0074],"3Y":[0.0083,0.0084,0.0084],"5Y":[0.011,0.0112,0.011]}
)");
        
        std::string text(jsoncons::stream_source<char>::default_max_buffer_size-1, ' ');
        text.append("Date,ProductType,1Y,2Y,3Y,5Y");
        text.push_back('\n');
        text.append(jsoncons::stream_source<char>::default_max_buffer_size-1, ' ');
        text.append("2017-01-09,\"UST\",0.0062,0.0075,0.0083,0.011");
        text.push_back('\n');
        text.append("2017-01-08,\"UST\",0.0063,0.0076,0.0084,0.0112");
        text.append(jsoncons::stream_source<char>::default_max_buffer_size - 1, ' ');
        text.push_back('\n');
        text.append(jsoncons::stream_source<char>::default_max_buffer_size-1, ' ');
        text.append("2017-01-07,\"UST\",");
        text.append(jsoncons::stream_source<char>::default_max_buffer_size - 1, ' ');
        text.append("0.0061,0.0074,0.0084,0.0110");
        text.push_back('\n');

        jsoncons::json_decoder<jsoncons::ojson> decoder;
        auto options = csv::csv_options{}
            .assume_header(true)
            .trim(true)
            .mapping_kind(csv::csv_mapping_kind::m_columns);

        std::istringstream is(text);
        csv::csv_stream_reader reader(is, decoder, options);
        reader.read();
        auto j = decoder.get_result();
        //std::cout << j << "\n";
        
        CHECK(j_expected == j);
    }
}

 
