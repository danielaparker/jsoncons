// Copyright 2016 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/cbor/cbor_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>

using namespace jsoncons;
using namespace jsoncons::cbor;

BOOST_AUTO_TEST_SUITE(cbor_serializer_tests)

template<class Json>
void encode_cbor(const Json& j, std::basic_ostream<typename Json::char_type>& os)
{
    typedef typename Json::char_type char_type;
    basic_cbor_serializer<char_type> serializer(os);
    j.dump(serializer);
}

BOOST_AUTO_TEST_CASE(test_serialize_to_stream)
{
    json j = json::parse(R"(
{ "store": {
    "book": [ 
          { "author": "Nigel Rees"
          },
          { "author": "Evelyn Waugh"
          },
          { "author": "Herman Melville"
          }
        ]
    }  
}
    )");

    std::ofstream outfile;
    outfile.open("./output/store.cbor", std::ios::binary | std::ios::out);
    encode_cbor(j,outfile);

    std::vector<uint8_t> v;
    std::ifstream infile;
    infile.open("./output/store.cbor", std::ios::binary | std::ios::in);
    infile.seekg(0, std::ios::end);   
    v.resize(infile.tellg());
    infile.seekg(0, std::ios::beg);    
    infile.read((char*)&v[0],v.size());

    json j2 = decode_cbor<json>(v);

    BOOST_CHECK(j == j2);
}

BOOST_AUTO_TEST_CASE(test_array)
{
    std::vector<uint8_t> v;
    cbor_bytes_serializer serializer(v);
    serializer.begin_json();
    //serializer.begin_object(1);
    serializer.begin_array(3);
    serializer.bool_value(true);
    serializer.bool_value(false);
    serializer.null_value();
    serializer.end_array();
    //serializer.end_object();
    serializer.end_json();

    for (auto c: v)
    {
        std::cout << std::hex << (int)c << std::endl;
    }

    try
    {
        json result = decode_cbor<json>(v);
        std::cout << result << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

BOOST_AUTO_TEST_CASE(test_indefinite_length_array)
{
    std::vector<uint8_t> v;
    cbor_bytes_serializer serializer(v);
    serializer.begin_json();
    serializer.begin_array();
    serializer.begin_array(4);
    serializer.bool_value(true);
    serializer.bool_value(false);
    serializer.null_value();
    serializer.string_value("Hello");
    serializer.end_array();
    serializer.end_array();
    serializer.end_json();

    for (auto c: v)
    {
        std::cout << std::hex << (int)c << std::endl;
    }

    try
    {
        json result = decode_cbor<json>(v);
        std::cout << result << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

BOOST_AUTO_TEST_SUITE_END()

