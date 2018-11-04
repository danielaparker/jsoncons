// Copyright 2018 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_structures.hpp>
#include <jsoncons/json_stream_reader.hpp>
#include <jsoncons/json_decoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

template<class CharT>
class basic_object_iterator
{
    typedef basic_json<CharT> json_type;
    typedef key_value<std::basic_string<CharT>,json_type> key_value_type;
    typedef key_value_type& reference;
    basic_stream_reader<CharT>& reader_;
    key_value_type kv_;
public:
    basic_object_iterator(basic_stream_reader<CharT>& reader)
      : reader_(reader)
    {
        operator++();
    }

    basic_object_iterator(const basic_object_iterator&) = delete;
    basic_object_iterator(basic_object_iterator&&) = delete;

    reference operator*() 
    {
        return kv_;
    }

    basic_object_iterator& operator++()
    {
        if (reader_.current().event_type() != stream_event_type::begin_object)
        {
            throw std::invalid_argument("Not an object");
        }
        reader_.next();
        if (reader_.done())
        {
            return *this;  // end
        }
        JSONCONS_ASSERT(reader_.current().event_type() == stream_event_type::name);
        kv_ = key_value_type(reader_.current().as<jsoncons::basic_string_view<CharT>>());

        json_decoder<json_type> decoder;
        //auto& old = reader_.handler(decoder);
        //std::cout << "next 1: " << (int)reader_.current().event_type() << "\n";
        //reader_.next();
        //std::cout << "next 2: " << (int)reader_.current().event_type() << "\n";
        //reader_.handler(old);
        kv_.value(decoder.get_result());

        return *this;
    }
};

typedef basic_object_iterator<char> object_iterator;

TEST_CASE("object_iterator test")
{
    std::string s = R"(
        {
            "enrollmentNo" : 100,
            "firstName" : "Tom",
            "lastName" : "Cochrane",
            "mark" : 55              
        }
    )";

    std::istringstream is(s);

    json_stream_reader reader(is);

    object_iterator it(reader);

    std::cout << (*it).key() << "\n";
    std::cout << (*it).value() << "\n";

}




