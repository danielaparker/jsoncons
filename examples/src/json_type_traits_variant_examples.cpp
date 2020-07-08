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

    class Basket {
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
    using variant_type  = std::variant<int, double, bool, std::string>;

    variant_type var(false);

    std::string buffer;
    jsoncons::encode_json(var,buffer);

    std::cout << "(1) " << buffer << "\n\n";

    auto var2 = jsoncons::decode_json<variant_type>(buffer);
    bool val = std::get<bool>(var2);

    std::cout << "(2) " << val << "\n\n";


}

#endif // defined(JSONCONS_HAS_STD_VARIANT)

void json_type_traits_variant_examples()
{
    std::cout << "\njson_type_traits variant examples\n\n";

#if defined(JSONCONS_HAS_STD_VARIANT)
    variant_example();
    variant_example2();
#endif

    std::cout << std::endl;
}

