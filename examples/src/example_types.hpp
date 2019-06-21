#ifndef EXAMPLE_TYPES
#define EXAMPLE_TYPES

#include <string>
#include <vector>
#include <jsoncons/json.hpp>

// book example

namespace ns {
    struct book
    {
        std::string author;
        std::string title;
        double price;
    };

    class Person
    {
    public:
        Person(const std::string& name, const std::string& surname,
               const std::string& ssn, unsigned int age)
           : name(name), surname(surname), ssn(ssn), age(age) { }

        bool operator==(const Person& rhs) const
        {
            return name == rhs.name && surname == rhs.surname && ssn == rhs.ssn &&
                   age == rhs.age;
        }
        bool operator!=(const Person& rhs) const { return !(rhs == *this); }

    private:
        // Make json_type_traits specializations friends to give accesses to private members
        JSONCONS_TYPE_TRAITS_FRIEND;

        Person() : age(0) {}

        std::string name;
        std::string surname;
        std::string ssn;
        unsigned int age;
    };

    class reputon
    {
    public:
        reputon(const std::string& rater,
                const std::string& assertion,
                const std::string& rated,
                double rating)
            : rater_(rater), assertion_(assertion), rated_(rated), rating_(rating)
        {
        }

        const std::string& rater() const {return rater_;}
        const std::string& assertion() const {return assertion_;}
        const std::string& rated() const {return rated_;}
        double rating() const {return rating_;}

        friend bool operator==(const reputon& lhs, const reputon& rhs)
        {
            return lhs.rater_ == rhs.rater_ && lhs.assertion_ == rhs.assertion_ && 
                   lhs.rated_ == rhs.rated_ && lhs.rating_ == rhs.rating_;
        }

        friend bool operator!=(const reputon& lhs, const reputon& rhs)
        {
            return !(lhs == rhs);
        };

    private:
        std::string rater_;
        std::string assertion_;
        std::string rated_;
        double rating_;
    };

    class reputation_object
    {
    public:
        reputation_object(const std::string& application, 
                          const std::vector<reputon>& reputons)
            : application_(application), 
              reputons_(reputons)
        {}

        const std::string& application() const { return application_;}
        const std::vector<reputon>& reputons() const { return reputons_;}

        friend bool operator==(const reputation_object& lhs, const reputation_object& rhs)
        {
            return (lhs.application_ == rhs.application_) && (lhs.reputons_ == rhs.reputons_);
        }

        friend bool operator!=(const reputation_object& lhs, const reputation_object& rhs)
        {
            return !(lhs == rhs);
        };
    private:
        std::string application_;
        std::vector<reputon> reputons_;
    };
} // namespace ns

namespace jsoncons {

    template<class Json>
    struct json_type_traits<Json, ns::book>
    {
        typedef typename Json::allocator_type allocator_type;

        static bool is(const Json& j) noexcept
        {
            return j.is_object() && j.contains("author") && 
                   j.contains("title") && j.contains("price");
        }
        static ns::book as(const Json& j)
        {
            ns::book val;
            val.author = j.at("author").template as<std::string>();
            val.title = j.at("title").template as<std::string>();
            val.price = j.at("price").template as<double>();
            return val;
        }
        static Json to_json(const ns::book& val, 
                            allocator_type allocator=allocator_type())
        {
            Json j(allocator);
            j.try_emplace("author", val.author);
            j.try_emplace("title", val.title);
            j.try_emplace("price", val.price);
            return j;
        }
    };
} // namespace jsoncons

// Declare the traits. Specify which data members need to be serialized.
JSONCONS_GETTER_CTOR_TRAITS_DECL(ns::reputon, rater, assertion, rated, rating)
JSONCONS_GETTER_CTOR_TRAITS_DECL(ns::reputation_object, application, reputons)

// Declare the traits. Specify which data members need to be serialized.
JSONCONS_MEMBER_TRAITS_DECL(ns::Person, name, surname, ssn, age)

#endif
