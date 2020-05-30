// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <jsoncons/json.hpp>
#include "sample_allocators.hpp"

using namespace jsoncons;

namespace {

    class MyIterator
    {
        const char* p_;
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = char;
        using difference_type = std::ptrdiff_t;
        using pointer = const char*; 
        using reference = const char&;

        MyIterator(const char* p)
            : p_(p)
        {
        }

        reference operator*() const
        {
            return *p_;
        }

        pointer operator->() const 
        {
            return p_;
        }

        MyIterator& operator++()
        {
            ++p_;
            return *this;
        }

        MyIterator operator++(int) 
        {
            MyIterator temp(*this);
            ++*this;
            return temp;
        }

        bool operator!=(const MyIterator& rhs) const
        {
            return p_ != rhs.p_;
        }
    };

    void custom_iterator_source()
    {
        char source[] = {'[','\"', 'f','o','o','\"',',','\"', 'b','a','r','\"',']'};

        MyIterator first(source);
        MyIterator last(source + sizeof(source));

        json j = json::parse(first, last);

        std::cout << j << "\n\n";
    }

    void read_mulitple_json_objects()
    {
        std::ifstream is("./input/multiple-json-objects.json");
        if (!is.is_open())
        {
            throw std::runtime_error("Cannot open file");
        }

        json_decoder<json> decoder;
        json_reader reader(is, decoder);

        while (!reader.eof())
        {
            reader.read_next();
            if (!reader.eof())
            {
                json j = decoder.get_result();
                std::cout << j << std::endl;
            }
        }
    }

    void read_with_stateful_allocator()
    {
        using my_json = basic_json<char,sorted_policy,FreelistAllocator<char>>;
        std::string input = R"(
    [ 
      { 
          "author" : "Haruki Murakami",
          "title" : "Hard-Boiled Wonderland and the End of the World",
          "isbn" : "0679743464",
          "publisher" : "Vintage",
          "date" : "1993-03-02",
          "price": 18.90
      },
      { 
          "author" : "Graham Greene",
          "title" : "The Comedians",
          "isbn" : "0099478374",
          "publisher" : "Vintage Classics",
          "date" : "2005-09-21",
          "price": 15.74
      }
    ]
    )";

        json_decoder<my_json,FreelistAllocator<char>> decoder(result_allocator_arg, FreelistAllocator<char>(1),
                                                              FreelistAllocator<char>(2));
        basic_json_reader<char,stream_source<char>,FreelistAllocator<char>> reader(input, decoder, FreelistAllocator<char>(3));
        reader.read();

        my_json j = decoder.get_result();
        std::cout << pretty_print(j) << "\n";
    }

} // namespace

void json_reader_examples()
{
    std::cout << "\njson_reader examples\n\n";

    read_mulitple_json_objects();
    read_with_stateful_allocator();
    custom_iterator_source();

    std::cout << "\n\n";
}
