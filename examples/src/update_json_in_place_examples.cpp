// Copyright 2020 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <iostream>

using namespace jsoncons;

namespace {

    class string_locator : public jsoncons::default_json_visitor
    {
        char* data_;
        std::size_t length_;
        std::vector<std::string> path_;
        std::string from_;
        std::vector<std::string> current_;
        std::vector<std::size_t> positions_;
    public:
        using jsoncons::default_json_visitor::string_view_type;

        string_locator(char* data, std::size_t length,
                       const std::vector<std::string>& path,
                       const std::string& from)
            : data_(data), length_(length),
              path_(path), from_(from)
        {
        }

        const std::vector<std::size_t>& positions() const
        {
            return positions_;
        }
    private:
        bool visit_begin_object(semantic_tag, const ser_context&, std::error_code&) override
        {
            current_.emplace_back();
            return true;
        }

        bool visit_end_object(const ser_context&, std::error_code&) override
        {
            current_.pop_back();
            return true;
        }

        bool visit_key(const string_view_type& key, const ser_context&, std::error_code&) override
        {
            current_.back() = key;
            return true;
        }

        bool visit_string(const string_view_type& value,
                          jsoncons::semantic_tag,
                          const jsoncons::ser_context& context,
                          std::error_code&) override
        {
            if (path_.size() <= current_.size() && std::equal(path_.rbegin(), path_.rend(), current_.rbegin()))
            {
                if (value == from_)
                {
                    positions_.push_back(context.position()+1); // one past quote character

                }
            }
            return true;
        }
    };

    void update_json_in_place(std::string& input,
                         const std::vector<std::string>& path,
                         const std::string& from,
                         const std::string& to)
    {
        string_locator locator(input.data(), input.size(), path, from);
        jsoncons::json_reader reader(jsoncons::string_view(input), locator);
        reader.read();

        for (auto it = locator.positions().rbegin(); it != locator.positions().rend(); ++it)
        {
            input.replace(*it, from.size(), to);
        }
    }

} // namespace

void update_json_in_place_examples()
{
    std::cout << "\njson update in place examples\n\n";

    std::string input = R"(
{
    "Cola" : {"Type":"Drink", "Price": 10.99},"Water" : {"Type":"Drink"}, "Extra" : {"Cola" : {"Type":"Drink", "Price": 8.99}}
}
)";

    try
    {
        std::cout << "(original)\n" << input << "\n";
        update_json_in_place(input, {"Cola", "Type"}, "Drink", "SoftDrink");

        std::cout << "(updated)\n" << input << "\n";
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << "\n";
    }

    std::cout << std::endl;
}



