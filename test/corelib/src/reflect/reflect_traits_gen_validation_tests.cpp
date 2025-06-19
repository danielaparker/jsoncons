// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>
#include <catch/catch.hpp>

namespace ns {

    struct employee_AMN
    {
        std::string name;
        uint64_t id;
        int age;
    };

    struct employee_NMN
    {
        std::string name;
        uint64_t id;
        int age;
    };

    class employee_ACGN
    {
        std::string name_;
        uint64_t id_;
        int age_;
    public:
        employee_ACGN(std::string name, uint64_t id, int age)
            : name_(name), id_(id), age_(age)
        {
        }

        const std::string& name() const {return name_;}
        uint64_t id() const {return id_;}
        int age() const {return age_;}
    };

    class employee_NCGN
    {
        std::string name_;
        uint64_t id_;
        int age_;
    public:
        employee_NCGN(std::string name, uint64_t id, int age)
            : name_(name), id_(id), age_(age)
        {
        }

        const std::string& name() const {return name_;}
        uint64_t id() const {return id_;}
        int age() const {return age_;}
    };

    class employee_AGSN
    {
        std::string name_;
        uint64_t id_;
        int age_;
    public:
        const std::string& name() const {return name_;}
        uint64_t id() const {return id_;}
        int age() const {return age_;}

        void name(const std::string& value) {name_ = value;}
        void id(uint64_t value) {id_ = value;}
        void age(int value) {age_ = value;}
    };

    class employee_NGSN
    {
        std::string name_;
        uint64_t id_;
        int age_;
    public:
        const std::string& name() const {return name_;}
        uint64_t id() const {return id_;}
        int age() const {return age_;}

        void name(const std::string& value) {name_ = value;}
        void id(uint64_t value) {id_ = value;}
        void age(int value) {age_ = value;}
    };

} // namespace jsoncons

JSONCONS_ALL_MEMBER_NAME_TRAITS(ns::employee_AMN,
    (name,"Name"),
    (id,"Id"),
    (age,"Age",JSONCONS_RDWR,
     [](int age) noexcept
     {
         return age >= 16 && age <= 68;
     }
    )
)

JSONCONS_N_MEMBER_NAME_TRAITS(ns::employee_NMN,3,
    (name,"Name"),
    (id,"Id"),
    (age,"Age",JSONCONS_RDWR,
     [](int age) noexcept
     {
         return age >= 16 && age <= 68;
     }
    )
)

JSONCONS_ALL_CTOR_GETTER_NAME_TRAITS(ns::employee_ACGN,
    (name,"Name"),
    (id,"Id"),
    (age,"Age",JSONCONS_RDWR,
     [](int age) noexcept
     {
         return age >= 16 && age <= 68;
     }
    )
)

JSONCONS_N_CTOR_GETTER_NAME_TRAITS(ns::employee_NCGN, 3,
    (name,"Name"),
    (id,"Id"),
    (age,"Age",JSONCONS_RDWR,
     [](int age) noexcept
     {
         return age >= 16 && age <= 68;
     }
    )
)

JSONCONS_ALL_GETTER_SETTER_NAME_TRAITS(ns::employee_AGSN,
    (name,name,"Name"),
    (id,id,"Id"),
    (age,age,"Age",JSONCONS_RDWR,
     [](int age) noexcept
     {
         return age >= 16 && age <= 68;
     }
    )
)

JSONCONS_N_GETTER_SETTER_NAME_TRAITS(ns::employee_NGSN, 3,
    (name,name,"Name"),
    (id,id,"Id"),
    (age,age,"Age",JSONCONS_RDWR,
     [](int age) noexcept
     {
         return age >= 16 && age <= 68;
     }
    )
)

TEST_CASE("json validator tests")
{
    const std::string input = R"(
    [
      {
        "Name" : "John Smith",
        "Id" : 22,
        "Age" : 345
      },
      {
        "Name" : "",
        "Id" : 23,
        "Age" : 36
      },
      {
        "Name" : "Jane Doe",
        "Id" : 24,
        "Age" : 34
      }
    ]
    )";

    SECTION("employee_AMN test")
    {
        std::error_code ec;
        jsoncons::json_string_cursor cursor(input, ec);

        auto view = jsoncons::staj_array<ns::employee_AMN>(cursor);

        for (auto it = view.begin(ec); it != view.end(); it.increment(ec))
        {
            if (!ec)
            {
                auto val = *it;
            }
        }
    }

    SECTION("employee_NMN test")
    {
        std::error_code ec;
        jsoncons::json_string_cursor cursor(input, ec);

        auto view = jsoncons::staj_array<ns::employee_NMN>(cursor);

        for (auto it = view.begin(ec); it != view.end(); it.increment(ec))
        {
            if (!ec)
            {
                auto val = *it;
            }
        }
    }

    SECTION("employee_ACGN test")
    {
        std::error_code ec;
        jsoncons::json_string_cursor cursor(input, ec);

        auto view = jsoncons::staj_array<ns::employee_ACGN>(cursor);

        for (auto it = view.begin(ec); it != view.end(); it.increment(ec))
        {
            if (!ec)
            {
                auto val = *it;
            }
        }
    }

    SECTION("employee_NCGN test")
    {
        std::error_code ec;
        jsoncons::json_string_cursor cursor(input, ec);

        auto view = jsoncons::staj_array<ns::employee_NCGN>(cursor);

        for (auto it = view.begin(ec); it != view.end(); it.increment(ec))
        {
            if (!ec)
            {
                auto val = *it;
            }
        }
    }

    SECTION("employee_AGSN test")
    {
        std::error_code ec;
        jsoncons::json_string_cursor cursor(input, ec);

        auto view = jsoncons::staj_array<ns::employee_AGSN>(cursor);

        for (auto it = view.begin(ec); it != view.end(); it.increment(ec))
        {
            if (!ec)
            {
                auto val = *it;
            }
        }
    }

    SECTION("employee_NGSN test")
    {
        std::error_code ec;
        jsoncons::json_string_cursor cursor(input, ec);

        auto view = jsoncons::staj_array<ns::employee_NGSN>(cursor);

        for (auto it = view.begin(ec); it != view.end(); it.increment(ec))
        {
            if (!ec)
            {
                auto val = *it;
            }
        }
    }
}

