// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <iomanip>
#include <jsoncons/json.hpp>

#if defined(JSONCONS_HAS_STD_VARIANT)

namespace { namespace ns {

    enum class Color {yellow, red, green, blue};

    inline
    std::ostream& operator<<(std::ostream& os, Color val)
    {
        switch (val)
        {
            case Color::yellow: os << "yellow"; break;
            case Color::red: os << "red"; break;
            case Color::green: os << "green"; break;
            case Color::blue: os << "blue"; break;
        }
        return os;
    }

    class Fruit 
    {
    private:
        JSONCONS_TYPE_TRAITS_FRIEND
        std::string name_;
        Color color_;
    public:
        friend std::ostream& operator<<(std::ostream& os, const Fruit& val)
        {
            os << "name: " << val.name_ << ", color: " << val.color_ << "\n";
            return os;
        }
    };

    class Fabric 
    {
    private:
        JSONCONS_TYPE_TRAITS_FRIEND
        int size_;
        std::string material_;
    public:
        friend std::ostream& operator<<(std::ostream& os, const Fabric& val)
        {
            os << "size: " << val.size_ << ", material: " << val.material_ << "\n";
            return os;
        }
    };

    class Basket 
    {
    private:
        JSONCONS_TYPE_TRAITS_FRIEND
        std::string owner_;
        std::vector<std::variant<Fruit, Fabric>> items_;

    public:
        std::string owner() const
        {
            return owner_;
        }

        std::vector<std::variant<Fruit, Fabric>> items() const
        {
            return items_;
        }
    };

} // ns
} // namespace

JSONCONS_ENUM_NAME_TRAITS(ns::Color, (yellow, "YELLOW"), (red, "RED"), (green, "GREEN"), (blue, "BLUE"))

JSONCONS_ALL_MEMBER_NAME_TRAITS(ns::Fruit,
                                (name_, "name"),
                                (color_, "color"))
JSONCONS_ALL_MEMBER_NAME_TRAITS(ns::Fabric,
                                (size_, "size"),
                                (material_, "material"))
JSONCONS_ALL_MEMBER_NAME_TRAITS(ns::Basket,
                                (owner_, "owner"),
                                (items_, "items"))

void variant_example()
{
    std::string input = R"(
{
  "owner": "Rodrigo",
  "items": [
    {
      "name": "banana",
      "color": "YELLOW"
    },
    {
      "size": 40,
      "material": "wool"
    },
    {
      "name": "apple",
      "color": "RED"
    },
    {
      "size": 40,
      "material": "cotton"
    }
  ]
}
    )";

    ns::Basket basket = jsoncons::decode_json<ns::Basket>(input);
    std::cout << basket.owner() << "\n\n";

    std::cout << "(1)\n";
    for (const auto& var : basket.items()) 
    {
        std::visit([](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ns::Fruit>)
                std::cout << "Fruit " << arg << '\n';
            else if constexpr (std::is_same_v<T, ns::Fabric>)
                std::cout << "Fabric " << arg << '\n';
        }, var);
    }

    std::string output;
    jsoncons::encode_json(basket, output, jsoncons::indenting::indent);
    std::cout << "(2)\n" << output << "\n\n";
}

void variant_example2()
{
    using variant_type  = std::variant<int, double, bool, std::string, ns::Color>;

    variant_type var1(100);
    variant_type var2(10.1);
    variant_type var3(false);
    variant_type var4(std::string("Hello World"));
    variant_type var5(ns::Color::yellow);

    std::string buffer1;
    jsoncons::encode_json(var1,buffer1);
    std::string buffer2;
    jsoncons::encode_json(var2,buffer2);
    std::string buffer3;
    jsoncons::encode_json(var3,buffer3);
    std::string buffer4;
    jsoncons::encode_json(var4,buffer4);
    std::string buffer5;
    jsoncons::encode_json(var5,buffer5);

    std::cout << "(1) " << buffer1 << "\n";
    std::cout << "(2) " << buffer2 << "\n";
    std::cout << "(3) " << buffer3 << "\n";
    std::cout << "(4) " << buffer4 << "\n";
    std::cout << "(5) " << buffer5 << "\n";

    auto v1 = jsoncons::decode_json<variant_type>(buffer1);
    auto v2 = jsoncons::decode_json<variant_type>(buffer2);
    auto v3 = jsoncons::decode_json<variant_type>(buffer3);
    auto v4 = jsoncons::decode_json<variant_type>(buffer4);
    auto v5 = jsoncons::decode_json<variant_type>(buffer5);

    auto visitor = [](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int>)
                std::cout << "int " << arg << '\n';
            else if constexpr (std::is_same_v<T, double>)
                std::cout << "double " << arg << '\n';
            else if constexpr (std::is_same_v<T, bool>)
                std::cout << "bool " << arg << '\n';
            else if constexpr (std::is_same_v<T, std::string>)
                std::cout << "std::string " << arg << '\n';
            else if constexpr (std::is_same_v<T, ns::Color>)
                std::cout << "ns::Color " << arg << '\n';
        };

    std::cout << "\n";
    std::cout << "(6) ";
    std::visit(visitor, v1);
    std::cout << "(7) ";
    std::visit(visitor, v2);
    std::cout << "(8) ";
    std::visit(visitor, v3);
    std::cout << "(9) ";
    std::visit(visitor, v4);
    std::cout << "(10) ";
    std::visit(visitor, v5);
    std::cout << "\n\n";
}

void variant_example3()
{
    using variant_type  = std::variant<int, double, bool, ns::Color, std::string>;

    variant_type var1(100);
    variant_type var2(10.1);
    variant_type var3(false);
    variant_type var4(std::string("Hello World"));
    variant_type var5(ns::Color::yellow);

    std::string buffer1;
    jsoncons::encode_json(var1, buffer1);
    std::string buffer2;
    jsoncons::encode_json(var2, buffer2);
    std::string buffer3;
    jsoncons::encode_json(var3, buffer3);
    std::string buffer4;
    jsoncons::encode_json(var4, buffer4);
    std::string buffer5;
    jsoncons::encode_json(var5, buffer5);

    std::cout << "(1) " << buffer1 << "\n";
    std::cout << "(2) " << buffer2 << "\n";
    std::cout << "(3) " << buffer3 << "\n";
    std::cout << "(4) " << buffer4 << "\n";
    std::cout << "(5) " << buffer5 << "\n";

    auto v1 = jsoncons::decode_json<variant_type>(buffer1);
    auto v2 = jsoncons::decode_json<variant_type>(buffer2);
    auto v3 = jsoncons::decode_json<variant_type>(buffer3);
    auto v4 = jsoncons::decode_json<variant_type>(buffer4);
    auto v5 = jsoncons::decode_json<variant_type>(buffer5);

    auto visitor = [](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int>)
                std::cout << "int " << arg << '\n';
            else if constexpr (std::is_same_v<T, double>)
                std::cout << "double " << arg << '\n';
            else if constexpr (std::is_same_v<T, bool>)
                std::cout << "bool " << arg << '\n';
            else if constexpr (std::is_same_v<T, std::string>)
                std::cout << "std::string " << arg << '\n';
            else if constexpr (std::is_same_v<T, ns::Color>)
                std::cout << "ns::Color " << arg << '\n';
        };

    std::cout << "\n";
    std::cout << "(6) ";
    std::visit(visitor, v1);
    std::cout << "(7) ";
    std::visit(visitor, v2);
    std::cout << "(8) ";
    std::visit(visitor, v3);
    std::cout << "(9) ";
    std::visit(visitor, v4);
    std::cout << "(10) ";
    std::visit(visitor, v5);
    std::cout << "\n\n";
}

#endif // defined(JSONCONS_HAS_STD_VARIANT)

void json_type_traits_variant_examples()
{
    std::cout << "\njson_type_traits variant examples\n\n";

#if defined(JSONCONS_HAS_STD_VARIANT)
    variant_example();
    variant_example2();
    variant_example3();
#endif

    std::cout << std::endl;
}

