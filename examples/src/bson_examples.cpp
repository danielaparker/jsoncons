// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/bson/bson.hpp>
#include <string>
#include <iomanip>

using namespace jsoncons;

void encode_to_bson()
{
    std::vector<uint8_t> buffer;
    bson::bson_bytes_encoder encoder(buffer);
    encoder.begin_array(); // The total number of bytes comprising 
                          // the bson document will be calculated
    encoder.string_value("cat");
    std::vector<uint8_t> purr = {'p','u','r','r'};
    encoder.byte_string_value(purr,7);
    encoder.int64_value(1431027667, semantic_tag::timestamp);
    encoder.end_array();
    encoder.flush();

    std::cout << byte_string_view(buffer.data(),buffer.size()) << "\n\n";

/* 
    23000000 -- Total number of bytes comprising the document (35 bytes) 
      02 -- UTF-8 string
        3000 -- "0"
        04000000 -- number bytes in the string (including trailing byte)
          636174  -- "cat"
            00 -- trailing byte
      05 -- binary
        3100 -- "1"
        04000000 -- number of bytes
        07 -- subtype
        70757272 -- 'P','u','r','r'
      09 -- datetime
      3200 -- "2"
        d3bf4b55 -- 1431027667
      00 
*/ 
}

void bson_examples()
{
    std::cout << "\nbson examples\n\n";
    encode_to_bson();
    std::cout << std::endl;
}

