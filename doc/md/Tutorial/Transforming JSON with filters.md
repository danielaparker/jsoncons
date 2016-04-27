Suppose you have a JSON address book that looks like this

    {
        "address-book" : 
        [
            {
                "name":"Jane Roe",
                "email":"jane.roe@example.com"
            },
            {
                 "name":"John",
                 "email" : "john.doe@example.com"
             }
        ]
    }
`address-book.json`

Now suppose you want to break the name into a first name and last name, and report a warning when `name` does not contain a space or tab separator. Pretend that the file `address-book.json` is too large to hold in memory.

You can achieve the desired result by subclassing the [json_filter](json_filter) class, overriding the default methods for receiving name and string value events, and passing modified events on to the parent [json_input_handler](json_input_handler) (which in this example will forward them to a [json_serializer](json_serializer).) 

    #include "jsoncons/json_serializer.hpp"
    #include "jsoncons/json_filter.hpp"
    #include "jsoncons/json_reader.hpp"

    using jsoncons::parsing_context;
    using jsoncons::json_serializer;
    using jsoncons::json_reader;
    using jsoncons::json_input_handler;
    using jsoncons::json_output_handler;

    class my_json_filter : public json_filter
    {
    public:
        my_json_filter(json_output_handler& handler)
           : json_filter(handler)
        {
        }

    private:
        void do_name(const char* p, size_t length, 
                     const parsing_context& context) override
        {
            property_name_ = string(p,length);
            if (property_name_ != "name")
            {
                input_handler().name(p, length, context);
            }
        }

        void do_string_value(const char* p, size_t length, 
                             const parsing_context& context) override
        {
            if (property_name_ == "name")
            {
                string value(p,length);
                size_t end_first = value.find_first_of(" \t");
                size_t start_last = value.find_first_not_of(" \t", end_first);
                input_handler().name("first-name", context);
                std::string first = value.substr(0, end_first);
                input_handler().value(first, context); 
                if (start_last != std::string::npos)
                {
                    input_handler().name("last-name", context);
                    std::string last = value.substr(start_last);
                    input_handler().value(last, context); 
                }
                else
                {
                    std::cerr << "Incomplete name \"" << value
                       << "\" at line " << context.line_number()
                       << " and column " << context.column_number() << std::endl;
                }
            }
            else
            {
                input_handler().value(p, length, context);
            }
        }

        std::string property_name_;
    };

In your code you will pass `my_json_filter` to the constructor of [json_reader](json_reader), and call read

    std::string in_file = "address-book.json";
    std::string out_file = "address-book-new.json";
    std::ifstream is(in_file,std::ofstream::binary);
    std::ofstream os(out_file);

    json_serializer serializer(os, true);
    my_json_filter filter(serializer);
    json_reader reader(is,filter);
	reader.read();

The output is a warning message written to the console

    Incomplete name "John" at line 9 and column 27

and an address book file with the required change

    {
        "address-book":
        [
            {
                "first-name":"Jane",
                "last-name":"Roe",
                "email":"jane.roe@example.com"
            },
            {
                "first-name":"John",
                "email":"john.doe@example.com"
            }
        ]
    }

`address-book-new.json`
