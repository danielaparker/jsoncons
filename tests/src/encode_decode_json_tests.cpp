// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <cstdint>

namespace 
{
    template <class T>
    struct MyAlloc
    {
        using value_type = T;
        using size_type = std::size_t;
        using propagate_on_container_move_assignment = std::true_type;

        #if _GLIBCXX_FULLY_DYNAMIC_STRING == 0
        MyAlloc() = default;
        #endif
        MyAlloc(int) {}

        template< class U >
        MyAlloc(const MyAlloc<U>&) noexcept {}

        T* allocate(size_type n)
        {
            return static_cast<T*>(::operator new(n * sizeof(T)));
        }

        void deallocate(T* ptr, size_type) noexcept
        {
            ::operator delete(ptr);
        }

        bool operator==(const MyAlloc&) const { return true; }
        bool operator!=(const MyAlloc&) const { return false; }

        template<typename U>
        struct rebind
        {
            typedef MyAlloc<U> other;
        };
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef T& reference;
        typedef const T& const_reference;
        typedef std::ptrdiff_t difference_type;
    };
    //template <class U>
    //using MyAlloc = std::allocator<U>;
} // namespace

using namespace jsoncons;

TEST_CASE("encode and decode json")
{
    json j(std::make_pair(false,std::string("foo")));

    SECTION("string test")
    {
        std::string s;
        encode_json(j, s);
        json result = decode_json<json>(s);
        CHECK(result == j);
    }

    SECTION("stream test")
    {
        std::stringstream ss;
        encode_json(j, ss);
        json result = decode_json<json>(ss);
        CHECK(result == j);
    }
}

TEST_CASE("encode and decode wjson")
{
    wjson j(std::make_pair(false,std::wstring(L"foo")));

    SECTION("string test")
    {
        std::wstring s;
        encode_json(j, s);
        wjson result = decode_json<wjson>(s);
        CHECK(result == j);
    }

    SECTION("stream test")
    {
        std::wstringstream ss;
        encode_json(j, ss);
        wjson result = decode_json<wjson>(ss);
        CHECK(result == j);
    }
}

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
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        CHECK(v[i] == result[i]);
    }
}

TEST_CASE("convert_vector_test, temp_allocator")
{
    std::vector<double> v = {1,2,3,4,5,6};

    std::string s;
    jsoncons::encode_json(v,s);

    auto result = jsoncons::decode_json<std::vector<double>>(
        temp_allocator_arg, MyAlloc<char>(1), s);

    REQUIRE(v.size() == result.size());
    for (std::size_t i = 0; i < result.size(); ++i)
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

TEST_CASE("convert_map_test, temp_allocator")
{
    std::map<std::string,double> m = {{"a",1},{"b",2}};

    std::string s;
    jsoncons::encode_json(m,s);
    auto result = jsoncons::decode_json<std::map<std::string,double>>(
        temp_allocator_arg, MyAlloc<char>(1), s);
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
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        CHECK(v[i] == result[i]);
    }
}

TEST_CASE("convert vector of vector test")
{
    std::vector<double> u{1,2,3,4};
    std::vector<std::vector<double>> v{u,u};

    std::string s;
    jsoncons::encode_json(v,s);
    auto result = jsoncons::decode_json<std::vector<std::vector<double>>>(s);
    REQUIRE(result.size() == v.size());
    for (const auto& item : result)
    {
        REQUIRE(item.size() == u.size());
        CHECK(item[0] == 1);
        CHECK(item[1] == 2);
        CHECK(item[2] == 3);
        CHECK(item[3] == 4);
    }
}

TEST_CASE("convert vector of vector test, temp_allocator")
{
    std::vector<double> u{1,2,3,4};
    std::vector<std::vector<double>> v{u,u};

    std::string s;
    jsoncons::encode_json(v,s);
    auto result = jsoncons::decode_json<std::vector<std::vector<double>>>(
        temp_allocator_arg, MyAlloc<char>(1), s);
    REQUIRE(result.size() == v.size());
    for (const auto& item : result)
    {
        REQUIRE(item.size() == u.size());
        CHECK(item[0] == 1);
        CHECK(item[1] == 2);
        CHECK(item[2] == 3);
        CHECK(item[3] == 4);
    }
}

#if !(defined(__GNUC__) && __GNUC__ <= 5)
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

TEST_CASE("convert_tuple_test, temp_allocator")
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
    auto employees2 = jsoncons::decode_json<employee_collection>(
        temp_allocator_arg, MyAlloc<char>(1), s);
    REQUIRE(employees2.size() == employees.size());

    std::cout << "\n(2)\n";
    for (const auto& pair : employees2)
    {
        std::cout << pair.first << ": " << std::get<1>(pair.second) << std::endl;
    }
}
#endif

TEST_CASE("encode/decode map with integer key")
{
    std::map<int,double> m = {{1,1},{2,2}};

    std::string s;
    jsoncons::encode_json(m,s);
    auto result = jsoncons::decode_json<std::map<int,double>>(s);

    REQUIRE(result.size() == m.size());
    CHECK(m[1] == result[1]);
    CHECK(m[2] == result[2]);
}

