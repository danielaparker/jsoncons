// Copyright 2020 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstdint>
#include <jsoncons/json.hpp>

#define NEW_GETTER_SETTER_IS(P1, P2, P3, Seq, Count) NEW_GETTER_SETTER_IS_LAST(P1, P2, P3, Seq, Count)
#define NEW_GETTER_SETTER_IS_LAST(P1, P2, P3, Seq, Count) if ((num_params-Count) < num_mandatory_params1 && JSONCONS_EXPAND(NEW_GETTER_SETTER_IS_ Seq)
#define NEW_GETTER_SETTER_IS_(Getter, Setter, Name) !ajson.contains(Name)) return false;

#define NEW_GETTER_SETTER_NAME_AS(P1, P2, P3, Seq, Count) NEW_GETTER_SETTER_NAME_AS_LAST(P1, P2, P3, Seq, Count)
#define NEW_GETTER_SETTER_NAME_AS_LAST(P1, P2, P3, Seq, Count) NEW_GETTER_SETTER_NAME_AS_T(P1, P2, P3, Seq, Count, JSONCONS_NARGS Seq)
#define NEW_GETTER_SETTER_NAME_AS_T(P1, P2, P3, Seq, Count, SeqCount) std::cout << SeqCount <<"\n\n"; if ((num_params-Count) < num_mandatory_params2 || JSONCONS_EXPAND(NEW_GETTER_SETTER_NAME_AS_ ## 3 Seq)
#define NEW_GETTER_SETTER_NAME_AS_(Getter, Setter, Name) ajson.contains(Name)) aval.Setter(ajson.at(Name).template as<typename std::decay<decltype(aval.Getter())>::type>());
#define NEW_GETTER_SETTER_NAME_AS_1(Getter, Setter, Name) ajson.contains(Name)) aval.Setter(ajson.at(Name).template as<typename std::decay<decltype(aval.Getter())>::type>());
#define NEW_GETTER_SETTER_NAME_AS_2(Getter, Setter, Name) ajson.contains(Name)) aval.Setter(ajson.at(Name).template as<typename std::decay<decltype(aval.Getter())>::type>());
#define NEW_GETTER_SETTER_NAME_AS_3(Getter, Setter, Name) ajson.contains(Name)) aval.Setter(ajson.at(Name).template as<typename std::decay<decltype(aval.Getter())>::type>());

#define NEW_ALL_GETTER_SETTER_NAME_AS(P1, P2, P3, Seq, Count) NEW_ALL_GETTER_SETTER_NAME_AS_LAST(P1, P2, P3, Seq, Count)
#define NEW_ALL_GETTER_SETTER_NAME_AS_LAST(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(NEW_ALL_GETTER_SETTER_NAME_AS_ Seq)
#define NEW_ALL_GETTER_SETTER_NAME_AS_(Getter, Setter, Name) aval.Setter(ajson.at(Name).template as<typename std::decay<decltype(aval.Getter())>::type>());

#define NEW_GETTER_SETTER_NAME_TO_JSON(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(NEW_GETTER_SETTER_NAME_TO_JSON_ Seq)
#define NEW_GETTER_SETTER_NAME_TO_JSON_LAST(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(NEW_GETTER_SETTER_NAME_TO_JSON_ Seq)
#define NEW_GETTER_SETTER_NAME_TO_JSON_(Getter, Setter, Name) ajson.try_emplace(Name, aval.Getter() );

#define NEW_ALL_GETTER_SETTER_NAME_TO_JSON(P1, P2, P3, Seq, Count) NEW_ALL_GETTER_SETTER_NAME_TO_JSON_LAST(P1, P2, P3, Seq, Count)
#define NEW_ALL_GETTER_SETTER_NAME_TO_JSON_LAST(P1, P2, P3, Seq, Count) if ((num_params-Count) < num_mandatory_params2) JSONCONS_EXPAND(NEW_ALL_GETTER_SETTER_NAME_TO_JSON_ Seq)
#define NEW_ALL_GETTER_SETTER_NAME_TO_JSON_(Getter, Setter, Name) \
  ajson.try_emplace(Name, aval.Getter()); \
else \
  {json_traits_helper<Json>::set_optional_json_member(Name, aval.Getter(), ajson);}

#define NEW_GETTER_SETTER_NAME_TRAITS_BASE(AsT,ToJ, NumTemplateParams, ValueType,NumMandatoryParams1,NumMandatoryParams2, ...)  \
namespace jsoncons \
{ \
    template<typename Json JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_MORE_TPL_PARAM, NumTemplateParams)> \
    struct json_type_traits<Json, ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> \
    { \
        using value_type = ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams); \
        using allocator_type = typename Json::allocator_type; \
        using char_type = typename Json::char_type; \
        using string_view_type = typename Json::string_view_type; \
        constexpr static size_t num_params = JSONCONS_NARGS(__VA_ARGS__); \
        constexpr static size_t num_mandatory_params1 = NumMandatoryParams1; \
        constexpr static size_t num_mandatory_params2 = NumMandatoryParams2; \
        static bool is(const Json& ajson) noexcept \
        { \
            if (!ajson.is_object()) return false; \
            JSONCONS_VARIADIC_REP_N(JSONCONS_GETTER_SETTER_IS,,,, __VA_ARGS__)\
            return true; \
        } \
        static value_type as(const Json& ajson) \
        { \
            value_type aval{}; \
            JSONCONS_VARIADIC_REP_N(AsT,,,, __VA_ARGS__) \
            return aval; \
        } \
        static Json to_json(const value_type& aval, allocator_type alloc=allocator_type()) \
        { \
            Json ajson(json_object_arg, semantic_tag::none, alloc); \
            JSONCONS_VARIADIC_REP_N(ToJ,,,, __VA_ARGS__) \
            return ajson; \
        } \
    }; \
} \
  /**/

#define NEW_N_GETTER_SETTER_NAME_TRAITS(ValueType,NumMandatoryParams, ...)  \
    NEW_GETTER_SETTER_NAME_TRAITS_BASE(NEW_GETTER_SETTER_NAME_AS,NEW_GETTER_SETTER_NAME_TO_JSON, 0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/

namespace {
namespace ns {

    class book3b
    {
        std::string author_;
        std::string title_;
        double price_;
        std::string isbn_;
    public:
        book3b()
            : author_(), title_(), price_(), isbn_()
        {
        }

        book3b(const std::string& author,
              const std::string& title,
              double price,
              const std::string& isbn)
            : author_(author), title_(title), price_(price), isbn_(isbn)
        {
        }

        const std::string& get_author() const
        {
            return author_;
        }

        void set_author(const std::string& value)
        {
            author_ = value;
        }

        const std::string& get_title() const
        {
            return title_;
        }

        void set_title(const std::string& value)
        {
            title_ = value;
        }

        double get_price() const
        {
            return price_;
        }

        void set_price(double value)
        {
            price_ = value;
        }

        const std::string& get_isbn() const
        {
            return isbn_;
        }

        void set_isbn(const std::string& value)
        {
            isbn_ = value;
        }
    };

} // namespace
} // ns

NEW_N_GETTER_SETTER_NAME_TRAITS(ns::book3b, 2, (get_author, set_author, "Author"), (get_title, set_title, "Title"), (get_price, set_price, "Price"), (get_isbn, set_isbn, "Isbn"))

using namespace jsoncons;

TEST_CASE("json traits lambda tests")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;
    std::string an_isbn = "1400079276";

    SECTION("test 1")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;
        j["Price"] = a_price;

        ns::book3b book = j.as<ns::book3b>();

        std::cout << book.get_author() << "\n";
    }
} 

