// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <jsoncons/json.hpp>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <cstdint>

using boost::numeric::ublas::matrix;

namespace jsoncons
{
    template<>
    struct json_convert_traits<matrix<double>>
    {
        template <class CharT>
        static matrix<double> decode(std::basic_istringstream<CharT>& is,
                                     const basic_json_serializing_options<CharT>& options)
        {
            basic_json<CharT> j = basic_json<CharT>::parse(is, options);
            if (j.is_array() && j.size() > 0)
            {
                size_t m = j.size();
                size_t n = 0;
                for (const auto& a : j.array_range())
                {
                    if (a.size() > n)
                    {
                        n = a.size();
                    }
                }

                boost::numeric::ublas::matrix<double> A(m,n,double());
                for (size_t i = 0; i < m; ++i)
                {
                    const auto& a = j[i];
                    for (size_t j = 0; j < a.size(); ++j)
                    {
                        A(i,j) = a[j].template as<double>();
                    }
                }
                return A;
            }
            else
            {
                boost::numeric::ublas::matrix<double> A;
                return A;
            }
        }

        static void encode(const matrix<double>& val, json_content_handler& handler)
        {
            handler.begin_array();
            for (size_t i = 0; i < val.size1(); ++i)
            {
                handler.begin_array();
                for (size_t j = 0; j < val.size2(); ++j)
                {
                    handler.double_value(val(i, j));
                }
                handler.end_array();
            }
            handler.end_array();
        }
    };
};

using namespace jsoncons;

TEST_CASE("convert_pair_test")
{
    auto val = std::make_pair(false,std::string("foo"));
    std::string s;

    jsoncons::encode_json(val, s);

    auto result = jsoncons::decode_json<std::pair<bool,std::string>>(s);

    CHECK(val == result);
}

TEST_CASE("convert_vector_test")
{
    std::vector<double> v = {1,2,3,4,5,6};

    std::string s;
    jsoncons::encode_json(v,s);

    auto result = jsoncons::decode_json<std::vector<double>>(s);

    REQUIRE(v.size() == result.size());
    for (size_t i = 0; i < result.size(); ++i)
    {
        CHECK(v[i] == result[i]);
    }
}

TEST_CASE("convert_map_test")
{
    std::map<std::string,double> m = {{"a",1},{"b",2}};

    std::string s;
    jsoncons::encode_json(m,s);
    auto result = jsoncons::decode_json<std::map<std::string,double>>(s);

    REQUIRE(result.size() == m.size());
    CHECK(m["a"] == result["a"]);
    CHECK(m["b"] == result["b"]);
}

TEST_CASE("convert_array_test")
{
    std::array<double,4> v{1,2,3,4};

    std::string s;
    jsoncons::encode_json(v,s);
    std::array<double, 4> result = jsoncons::decode_json<std::array<double,4>>(s);
    REQUIRE(result.size() == v.size());
    for (size_t i = 0; i < result.size(); ++i)
    {
        CHECK(v[i] == result[i]);
    }
}

#if !defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ < 9
TEST_CASE("convert_tuple_test")
{
    typedef std::map<std::string,std::tuple<std::string,std::string,double>> employee_collection;

    employee_collection employees = 
    { 
        {"John Smith",{"Hourly","Software Engineer",10000}},
        {"Jane Doe",{"Commission","Sales",20000}}
    };

    std::string s;
    jsoncons::encode_json(employees, s, jsoncons::indenting::indent);
    std::cout << "(1)\n" << s << std::endl;
    auto employees2 = jsoncons::decode_json<employee_collection>(s);
    REQUIRE(employees2.size() == employees.size());

    std::cout << "\n(2)\n";
    for (const auto& pair : employees2)
    {
        std::cout << pair.first << ": " << std::get<1>(pair.second) << std::endl;
    }
}
#endif

TEST_CASE("test_encode_boost_matrix_to_json")
{
    matrix<double> A(2, 2);
    A(0, 0) = 1.1;
    A(0, 1) = 2.1;
    A(1, 0) = 3.1;
    A(1, 1) = 4.1;

    std::string s;
    jsoncons::encode_json(A,s,indenting::indent);
    std::cout << "(1) " << s << std::endl;
    auto A2 = jsoncons::decode_json<matrix<double>>(s);

}


