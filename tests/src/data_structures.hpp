#ifndef JSONCONS_TESTS_DATA_STRUTURES_HPP
#define JSONCONS_TESTS_DATA_STRUTURES_HPP

#include <string>
#include <vector>
#include <jsoncons/json.hpp>

namespace ns {

    template <typename T1, typename T2>
    struct TemplatedStruct
    {
          T1 aT1;
          T2 aT2;
    };

    template <typename T1>
    struct MyStruct
    {
          T1 typeContent;
          std::string someString;
    };

    template <typename T1>
    struct MyStruct2
    {
          T1 typeContent;
          std::string someString;
    };

    template <typename T1>
    struct MyStruct3
    {
        T1 typeContent_;
        std::string someString_;
    public:
        MyStruct3(T1 typeContent, const std::string& someString)
            : typeContent_(typeContent), someString_(someString)
        {
        }

        const T1& typeContent() const {return typeContent_;}
        const std::string& someString() const {return someString_;}
    };

    struct book
    {
        std::string author;
        std::string title;
        double price;

        friend std::ostream& operator<<(std::ostream& os, const book& b)
        {
            std::cout << "author: " << b.author << ", title: " << b.title << ", price: " << b.price << "\n";
            return os;
        }
    };
    struct book2
    {
        std::string author;
        std::string title;
        double price;
        std::string isbn;
    };
    class book3
    {
        std::string author_;
        std::string title_;
        double price_;
    public:
        book3(const std::string& author,
              const std::string& title,
              double price)
            : author_(author), title_(title), price_(price)
        {
        }

        book3(const book3&) = default;
        book3(book3&&) = default;
        book3& operator=(const book3&) = default;
        book3& operator=(book3&&) = default;

        const std::string& author() const
        {
            return author_;
        }

        const std::string& title() const
        {
            return title_;
        }

        double price() const
        {
            return price_;
        }
    };

    enum class float_format {scientific = 1,fixed = 2,hex = 4,general = fixed | scientific};

    enum class hiking_experience {beginner,intermediate,advanced};

    struct hiking_reputon
    {
        std::string rater;
        hiking_experience assertion;
        std::string rated;
        double rating;

        friend bool operator==(const hiking_reputon& lhs, const hiking_reputon& rhs)
        {
            return lhs.rater == rhs.rater && lhs.assertion == rhs.assertion && 
                   lhs.rated == rhs.rated && lhs.rating == rhs.rating;
        }

        friend bool operator!=(const hiking_reputon& lhs, const hiking_reputon& rhs)
        {
            return !(lhs == rhs);
        };
    };

    class hiking_reputation
    {
        std::string application;
        std::vector<hiking_reputon> reputons;

        // Make json_type_traits specializations friends to give accesses to private members
        JSONCONS_TYPE_TRAITS_FRIEND;

        hiking_reputation()
        {
        }
    public:
        hiking_reputation(const std::string& application, const std::vector<hiking_reputon>& reputons)
            : application(application), reputons(reputons)
        {}

        friend bool operator==(const hiking_reputation& lhs, const hiking_reputation& rhs)
        {
            return (lhs.application == rhs.application) && (lhs.reputons == rhs.reputons);
        }

        friend bool operator!=(const hiking_reputation& lhs, const hiking_reputation& rhs)
        {
            return !(lhs == rhs);
        };
    };
}

// Declare the traits. Specify which data members need to be serialized.

JSONCONS_ENUM_TRAITS_DECL(ns::float_format, scientific, fixed, hex, general)
JSONCONS_GETTER_CTOR_TRAITS_DECL(ns::book3, author, title, price)
JSONCONS_MEMBER_TRAITS_DECL(ns::book,author,title,price)
JSONCONS_MEMBER_TRAITS_DECL(ns::book2,author,title,price,isbn)
JSONCONS_TEMPLATE_MEMBER_TRAITS_DECL(1,ns::MyStruct,typeContent,someString)
JSONCONS_STRICT_TEMPLATE_MEMBER_TRAITS_DECL(1,ns::MyStruct2,typeContent,someString)
JSONCONS_TEMPLATE_GETTER_CTOR_TRAITS_DECL(1,ns::MyStruct3,typeContent,someString)
JSONCONS_TEMPLATE_MEMBER_TRAITS_DECL(2,ns::TemplatedStruct,aT1,aT2)

JSONCONS_ENUM_TRAITS_DECL(ns::hiking_experience, beginner, intermediate, advanced)
JSONCONS_MEMBER_TRAITS_DECL(ns::hiking_reputon, rater, assertion, rated, rating)
JSONCONS_MEMBER_TRAITS_DECL(ns::hiking_reputation, application, reputons)

#endif
