// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json_encoder.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons/json_reader.hpp>
#include <jsoncons/json.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>
#include <iostream>
#include <fstream>
#include <catch/catch.hpp>

using namespace jsoncons;

struct warning
{
    std::string name;
    std::size_t line_number;
    std::size_t column_number;
};

class name_fixup_filter : public json_filter
{
    std::string member_name_;
public:
    std::vector<warning> warnings;

    name_fixup_filter(json_visitor& visitor)
        : json_filter(visitor)
    {
    }

private:
    JSONCONS_VISITOR_RETURN_TYPE visit_key(const string_view_type& name,
        const ser_context& context,
        std::error_code& ec) override
    {
        member_name_ = std::string(name);
        if (member_name_ != "name")
        {
            this->destination().key(name, context, ec);
            JSONCONS_VISITOR_RETURN;
        }
        else
        {
            JSONCONS_VISITOR_RETURN;
        }
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_string(const string_view_type& s,
        semantic_tag tag,
        const ser_context& context,
        std::error_code&) override
    {
        if (member_name_ == "name")
        {
            std::size_t end_first = s.find_first_of(" \t");
            std::size_t start_last = s.find_first_not_of(" \t", end_first);
            this->destination().key("first-name", context);
            string_view_type first = s.substr(0, end_first);
            this->destination().string_value(first, tag, context);
            if (start_last != string_view_type::npos)
            {
                this->destination().key("last-name", context);
                string_view_type last = s.substr(start_last);
                this->destination().string_value(last, tag, context);
            }
            else
            {
                warnings.push_back(warning{std::string(s),
                                   context.line(),
                                   context.column()});
            }
        }
        else
        {
            this->destination().string_value(s, tag, context);
        }
        JSONCONS_VISITOR_RETURN;
    }
};

TEST_CASE("test_filter")
{
    std::string in_file = "./corelib/input/address-book.json";
    std::string out_file = "./corelib/output/address-book-new.json";
    std::ifstream is(in_file);
    std::ofstream os(out_file);

    json_stream_encoder encoder(os);
    name_fixup_filter filter(encoder);
    json_stream_reader reader(is, filter);
    reader.read_next();

    CHECK(1 == filter.warnings.size());
    CHECK("John" ==filter.warnings[0].name);
    CHECK(9 == filter.warnings[0].line_number);
    CHECK(27 == filter.warnings[0].column_number);
}

TEST_CASE("test_filter2")
{
    std::string in_file = "./corelib/input/address-book.json";
    std::string out_file = "./corelib/output/address-book-new.json";
    std::ifstream is(in_file);
    std::ofstream os(out_file);

    json_stream_encoder encoder(os);

    name_fixup_filter filter2(encoder);

    rename_object_key_filter filter1("email","email2",filter2);

    json_stream_reader reader(is, filter1);
    reader.read_next();

    CHECK(1 == filter2.warnings.size());
    CHECK("John" ==filter2.warnings[0].name);
    CHECK(9 == filter2.warnings[0].line_number);
    CHECK(27 == filter2.warnings[0].column_number);
}

TEST_CASE("test_rename_name")
{
    json j;
    JSONCONS_TRY
    {
        j = json::parse(R"(
{"store":
{"book": [
{"category": "reference",
"author": "Margaret Weis",
"title": "Dragonlance Series",
"price": 31.96}, {"category": "reference",
"author": "Brent Weeks",
"title": "Night Angel Trilogy",
"price": 14.70
}]}}
)");
    }
    JSONCONS_CATCH (const ser_error& e)
    {
        std::cout << e.what() << '\n';
    }
    CHECK(j["store"]["book"][0]["price"].as<double>() == Approx(31.96).epsilon(0.001));

    std::stringstream ss;
    json_stream_encoder encoder(ss);
    rename_object_key_filter filter("price","price2",encoder);
    j.dump(filter);

    json j2 = json::parse(ss);
    CHECK(j2["store"]["book"][0]["price2"].as<double>() == Approx(31.96).epsilon(0.001));
}

TEST_CASE("test_chained_filters")
{
    ojson j = ojson::parse(R"({"first":1,"second":2,"fourth":3,"fifth":4})");

    json_decoder<ojson> decoder;

    rename_object_key_filter filter2("fifth", "fourth", decoder);
    rename_object_key_filter filter1("fourth", "third", filter2);

    j.dump(filter1);
    ojson j2 = decoder.get_result();
    CHECK(4 == j2.size());
    CHECK(1 == j2["first"]);
    CHECK(2 == j2["second"]);
    CHECK(3 == j2["third"]);
    CHECK(4 == j2["fourth"]);
}

