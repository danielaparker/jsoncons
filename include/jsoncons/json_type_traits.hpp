// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_TYPE_TRAITS_HPP
#define JSONCONS_JSON_TYPE_TRAITS_HPP

#include <array>
#include <string>
#include <vector>
#include <valarray>
#include <exception>
#include <cstring>
#include <utility>
#include <algorithm> // std::swap
#include <limits> // std::numeric_limits
#include <type_traits> // std::enable_if
#include <iterator> // std::iterator_traits, std::input_iterator_tag
#include <jsoncons/bignum.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/detail/type_traits_helper.hpp>
#include <string>
#include <tuple>
#include <map>
#include <functional>
#include <memory>

namespace jsoncons {


template <class T,class Enable=void>
struct is_basic_json_class : std::false_type
{};

#if defined(_MSC_VER) && _MSC_VER < 1916  
template <class T>
struct is_basic_json_class<T, typename std::enable_if<!std::is_void<typename T::char_type>::value && 
                                                      !std::is_void<typename T::implementation_policy>::value && 
                                                      !std::is_void<typename T::allocator_type>::value>::type> : std::true_type
{};
#else
template <class T>
struct is_basic_json_class<T, decltype(std::declval<jsoncons::basic_json<typename T::char_type,typename T::implementation_policy,typename T::allocator_type>>(),void())> : std::true_type
{};
#endif

template <class T>
struct is_json_type_traits_declared : public std::false_type
{};

#if !defined(JSONCONS_NO_DEPRECATED)
template <class T>
using is_json_type_traits_impl = is_json_type_traits_declared<T>;
#endif

// json_type_traits

template<typename T>
struct unimplemented : std::false_type
{};

template <class Json, class T, class Enable=void>
struct json_type_traits
{
    typedef typename Json::allocator_type allocator_type;

    static constexpr bool is_compatible = false;

    static constexpr bool is(const Json&)
    {
        return false;
    }

    static T as(const Json&)
    {
        static_assert(unimplemented<T>::value, "as not implemented");
    }

    static Json to_json(const T&, allocator_type = allocator_type())
    {
        static_assert(unimplemented<T>::value, "to_json not implemented");
    }
};

namespace detail {

// is_incompatible
template<class Json, class T, class Enable = void>
struct is_incompatible : std::false_type {};


// is_incompatible
template<class Json, class T>
struct is_incompatible<Json,T,
    typename std::enable_if<!std::integral_constant<bool, json_type_traits<Json, T>::is_compatible>::value>::type
> : std::true_type {};

// is_compatible_string_type
template<class Json, class T, class Enable=void>
struct is_compatible_string_type : std::false_type {};

template<class Json, class T>
struct is_compatible_string_type<Json,T, 
    typename std::enable_if<!std::is_same<T,typename Json::array>::value &&
    jsoncons::detail::is_string_like<T>::value && 
    !is_incompatible<Json,typename std::iterator_traits<typename T::iterator>::value_type>::value
>::type> : std::true_type {};

// is_compatible_string_view_type
template<class Json, class T, class Enable=void>
struct is_compatible_string_view_type : std::false_type {};

template<class Json, class T>
struct is_compatible_string_view_type<Json,T, 
    typename std::enable_if<!std::is_same<T,typename Json::array>::value &&
    jsoncons::detail::is_string_view_like<T>::value && 
    !is_incompatible<Json,typename std::iterator_traits<typename T::iterator>::value_type>::value
>::type> : std::true_type {};

// is_compatible_array_type
template<class Json, class T, class Enable=void>
struct is_compatible_array_type : std::false_type {};

template<class Json, class T>
struct is_compatible_array_type<Json,T, 
    typename std::enable_if<!std::is_same<T,typename Json::array>::value &&
    jsoncons::detail::is_vector_like<T>::value && 
    !is_incompatible<Json,typename std::iterator_traits<typename T::iterator>::value_type>::value
>::type> : std::true_type {};

// is_compatible_object_type
template<class Json, class T, class Enable=void>
struct is_compatible_object_type : std::false_type {};

template<class Json, class T>
struct is_compatible_object_type<Json,T, 
                       typename std::enable_if<
    !is_incompatible<Json,typename T::mapped_type>::value
>::type> : std::true_type {};

template <class Json, class T>
class json_array_input_iterator
{
public:
    typedef typename Json::const_array_iterator iterator_base;
    typedef typename std::iterator_traits<iterator_base>::value_type value_type;
    typedef typename std::iterator_traits<iterator_base>::difference_type difference_type;
    typedef typename std::iterator_traits<iterator_base>::pointer pointer;
    typedef T reference;
    typedef std::input_iterator_tag iterator_category;

    json_array_input_iterator()
    {
    }

    json_array_input_iterator(iterator_base it)
        : it_(it)
    {
    }

    json_array_input_iterator& operator=(json_array_input_iterator rhs)
    {
        swap(*this,rhs);
        return *this;
    }

    json_array_input_iterator& operator++()
    {
        ++it_;
        return *this;
    }

    json_array_input_iterator operator++(int) // postfix increment
    {
        json_array_input_iterator temp(*this);
        ++it_;
        return temp;
    }

    json_array_input_iterator& operator--()
    {
        --it_;
        return *this;
    }

    json_array_input_iterator operator--(int)
    {
        json_array_input_iterator temp(*this);
        --it_;
        return temp;
    }

    reference operator*() const
    {
        return json_type_traits<Json,T>::as(*it_);
    }

    friend bool operator==(const json_array_input_iterator& it1, const json_array_input_iterator& it2)
    {
        return it1.it_ == it2.it_;
    }
    friend bool operator!=(const json_array_input_iterator& it1, const json_array_input_iterator& it2)
    {
        return !(it1.it_ == it2.it_);
    }
    friend void swap(json_array_input_iterator& lhs, json_array_input_iterator& rhs)
    {
        using std::swap;
        swap(lhs.it_,rhs.it_);
        swap(lhs.empty_,rhs.empty_);
    }

private:
    iterator_base it_;
};

template <class Json, class T>
class json_object_input_iterator
{
public:
    typedef typename Json::const_object_iterator iterator_base;
    typedef typename std::iterator_traits<iterator_base>::value_type value_type;
    typedef typename std::iterator_traits<iterator_base>::difference_type difference_type;
    typedef typename std::iterator_traits<iterator_base>::pointer pointer;
    typedef T reference;
    typedef std::input_iterator_tag iterator_category;
    typedef typename T::first_type key_type;
    typedef typename T::second_type mapped_type;

    json_object_input_iterator()
    {
    }

    json_object_input_iterator(iterator_base it)
        : it_(it)
    {
    }

    json_object_input_iterator& operator=(json_object_input_iterator rhs)
    {
        swap(*this,rhs);
        return *this;
    }

    json_object_input_iterator& operator++()
    {
        ++it_;
        return *this;
    }

    json_object_input_iterator operator++(int) // postfix increment
    {
        json_object_input_iterator temp(*this);
        ++it_;
        return temp;
    }

    json_object_input_iterator& operator--()
    {
        --it_;
        return *this;
    }

    json_object_input_iterator operator--(int)
    {
        json_object_input_iterator temp(*this);
        --it_;
        return temp;
    }

    reference operator*() const
    {
        return T(key_type(it_->key()),json_type_traits<Json,mapped_type>::as(it_->value()));
    }

    friend bool operator==(const json_object_input_iterator& it1, const json_object_input_iterator& it2)
    {
        return it1.it_ == it2.it_;
    }
    friend bool operator!=(const json_object_input_iterator& it1, const json_object_input_iterator& it2)
    {
        return !(it1.it_ == it2.it_);
    }
    friend void swap(json_object_input_iterator& lhs, json_object_input_iterator& rhs)
    {
        using std::swap;
        swap(lhs.it_,rhs.it_);
        swap(lhs.empty_,rhs.empty_);
    }

private:
    iterator_base it_;
};

}

template<class Json>
struct json_type_traits<Json, typename type_wrapper<typename Json::char_type>::const_pointer_type>
{
    typedef typename Json::char_type char_type;
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        return j.is_string();
    }
    static const char_type* as(const Json& j)
    {
        return j.as_cstring();
    }
    template <class ... Args>
    static Json to_json(const char_type* s, Args&&... args)
    {
        return Json(s, semantic_tag::none, std::forward<Args>(args)...);
    }
};

template<class Json>
struct json_type_traits<Json, typename type_wrapper<typename Json::char_type>::pointer_type>
{
    typedef typename Json::char_type char_type;
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        return j.is_string();
    }
    template <class ... Args>
    static Json to_json(const char_type* s, Args&&... args)
    {
        return Json(s, semantic_tag::none, std::forward<Args>(args)...);
    }
};

// integral

template<class Json, class T>
struct json_type_traits<Json, T,
                        typename std::enable_if<jsoncons::detail::is_integer_like<T>::value
>::type>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        if (j.is_int64())
        {
            return (j.template as_integer<int64_t>() >= (std::numeric_limits<T>::lowest)()) && (j.template as_integer<int64_t>() <= (std::numeric_limits<T>::max)());
        }
        else if (j.is_uint64())
        {
            return j.template as_integer<uint64_t>() <= static_cast<uint64_t>((std::numeric_limits<T>::max)());
        }
        else
        {
            return false;
        }
    }
    static T as(const Json& j)
    {
        return j.template as_integer<T>();
    }
    static Json to_json(T val, allocator_type = allocator_type())
    {
        return Json(val, semantic_tag::none);
    }
};

template<class Json, class T>
struct json_type_traits<Json, T,
                        typename std::enable_if<jsoncons::detail::is_uinteger_like<T>::value>::type>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        if (j.is_int64())
        {
            return j.template as_integer<int64_t>() >= 0 && static_cast<uint64_t>(j.template as_integer<int64_t>()) <= (std::numeric_limits<T>::max)();
        }
        else if (j.is_uint64())
        {
            return j.template as_integer<uint64_t>() <= (std::numeric_limits<T>::max)();
        }
        else
        {
            return false;
        }
    }

    static T as(const Json& j)
    {
        return j.template as_integer<T>();
    }

    static Json to_json(T val, allocator_type = allocator_type())
    {
        return Json(val, semantic_tag::none);
    }
};

template<class Json,class T>
struct json_type_traits<Json, T,
                        typename std::enable_if<std::is_floating_point<T>::value
>::type>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        return j.is_double();
    }
    static T as(const Json& j)
    {
        return static_cast<T>(j.as_double());
    }
    static Json to_json(T val, allocator_type = allocator_type())
    {
        return Json(val, semantic_tag::none);
    }
};

template<class Json>
struct json_type_traits<Json, typename Json::object>
{
    typedef typename Json::object json_object;
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        return j.is_object();
    }
    static Json to_json(const json_object& o)
    {
        return Json(o,semantic_tag::none);
    }
};

template<class Json>
struct json_type_traits<Json, typename Json::array>
{
    typedef typename Json::array json_array;
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        return j.is_array();
    }
    static Json to_json(const json_array& a)
    {
        return Json(a, semantic_tag::none);
    }
};

template<class Json>
struct json_type_traits<Json, Json>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json&) noexcept
    {
        return true;
    }
    static Json as(Json j)
    {
        return j;
    }
    static Json to_json(const Json& val, allocator_type = allocator_type())
    {
        return val;
    }
};

template<class Json>
struct json_type_traits<Json, jsoncons::null_type>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        return j.is_null();
    }
    static typename jsoncons::null_type as(const Json& j)
    {
        JSONCONS_ASSERT(j.is_null());
        return jsoncons::null_type();
    }
    static Json to_json(jsoncons::null_type, allocator_type = allocator_type())
    {
        return Json::null();
    }
};

template<class Json>
struct json_type_traits<Json, bool>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        return j.is_bool();
    }
    static bool as(const Json& j)
    {
        return j.as_bool();
    }
    static Json to_json(bool val, allocator_type = allocator_type())
    {
        return Json(val, semantic_tag::none);
    }
};

template<class Json,class T>
struct json_type_traits<Json, T, typename std::enable_if<std::is_same<T, 
    std::conditional<!std::is_same<bool,std::vector<bool>::const_reference>::value,
                     std::vector<bool>::const_reference,
                     void>::type>::value>::type>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        return j.is_bool();
    }
    static bool as(const Json& j)
    {
        return j.as_bool();
    }
    static Json to_json(bool val, allocator_type = allocator_type())
    {
        return Json(val, semantic_tag::none);
    }
};

template<class Json>
struct json_type_traits<Json, std::vector<bool>::reference>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        return j.is_bool();
    }
    static bool as(const Json& j)
    {
        return j.as_bool();
    }
    static Json to_json(bool val, allocator_type = allocator_type())
    {
        return Json(val, semantic_tag::none);
    }
};

template<class Json, typename T>
struct json_type_traits<Json, T, 
                        typename std::enable_if<!is_json_type_traits_declared<T>::value && jsoncons::detail::is_compatible_array_type<Json,T>::value>::type>
{
    typedef typename std::iterator_traits<typename T::iterator>::value_type element_type;
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        bool result = j.is_array();
        if (result)
        {
            for (auto e : j.array_range())
            {
                if (!e.template is<element_type>())
                {
                    result = false;
                    break;
                }
            }
        }
        return result;
    }

    template <class Ty = element_type>
    static typename std::enable_if<!(std::is_integral<Ty>::value && !std::is_same<Ty,bool>::value),T>::type
    as(const Json& j)
    {
        if (j.is_array())
        {
            T v(jsoncons::detail::json_array_input_iterator<Json, element_type>(j.array_range().begin()),
                jsoncons::detail::json_array_input_iterator<Json, element_type>(j.array_range().end()));
            return v;
        }
        else
        {
            JSONCONS_THROW(json_runtime_error<std::runtime_error>("Attempt to cast json non-array to array"));
        }
    }

    template <class Ty = element_type>
    static typename std::enable_if<std::is_integral<Ty>::value && !std::is_same<Ty,bool>::value,T>::type
    as(const Json& j)
    {
        if (j.is_array())
        {
            T v(jsoncons::detail::json_array_input_iterator<Json, element_type>(j.array_range().begin()),
                jsoncons::detail::json_array_input_iterator<Json, element_type>(j.array_range().end()));
            return v;
        }
        else if (j.is_byte_string_view())
        {
            T v(j.as_byte_string_view().begin(),j.as_byte_string_view().end());
            return v;
        }
        else if (j.is_byte_string())
        {
            auto s = j.as_byte_string();
            T v(s.begin(),s.end());
            return v;
        }
        else
        {
            JSONCONS_THROW(json_runtime_error<std::runtime_error>("Attempt to cast json non-array to array"));
        }
    }

    static Json to_json(const T& val)
    {
        Json j = typename Json::array();
        auto first = std::begin(val);
        auto last = std::end(val);
        size_t size = std::distance(first,last);
        j.reserve(size);
        for (auto it = first; it != last; ++it)
        {
            j.push_back(*it);
        }
        return j;
    }

    static Json to_json(const T& val, const allocator_type& allocator)
    {
        Json j = typename Json::array(allocator);
        auto first = std::begin(val);
        auto last = std::end(val);
        size_t size = std::distance(first, last);
        j.reserve(size);
        for (auto it = first; it != last; ++it)
        {
            j.push_back(*it);
        }
        return j;
    }
};

template<class Json, typename T>
struct json_type_traits<Json, T, 
                        typename std::enable_if<!is_json_type_traits_declared<T>::value && jsoncons::detail::is_compatible_string_type<Json,T>::value>::type>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        return j.is_string();
    }

    static T as(const Json& j)
    {
   	    return T(j.as_string());
    }

    static Json to_json(const T& val)
    {
        return Json(val, semantic_tag::none);
    }

    static Json to_json(const T& val, const allocator_type& allocator)
    {
        return Json(val, semantic_tag::none, allocator);
    }
};

template<class Json, typename T>
struct json_type_traits<Json, T, 
                        typename std::enable_if<!is_json_type_traits_declared<T>::value && jsoncons::detail::is_compatible_string_view_type<Json,T>::value>::type>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        return j.is_string_view();
    }

    static T as(const Json& j)
    {
   	    return T(j.as_string_view().data(),j.as_string_view().size());
    }

    static Json to_json(const T& val)
    {
        return Json(val, semantic_tag::none);
    }

    static Json to_json(const T& val, const allocator_type& allocator)
    {
        return Json(val, semantic_tag::none, allocator);
    }
};

template<class Json, typename T>
struct json_type_traits<Json, T, 
                        typename std::enable_if<!is_json_type_traits_declared<T>::value && jsoncons::detail::is_compatible_object_type<Json,T>::value>::type
>
{
    typedef typename T::mapped_type mapped_type;
    typedef typename T::value_type value_type;
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        bool result = j.is_object();
        for (auto member : j.object_range())
        {
            if (!member.value().template is<mapped_type>())
            {
                result = false;
            }
        }
        return result;
    }

    static T as(const Json& j)
    {
        T v(jsoncons::detail::json_object_input_iterator<Json,value_type>(j.object_range().begin()),
            jsoncons::detail::json_object_input_iterator<Json,value_type>(j.object_range().end()));
        return v;
    }

    static Json to_json(const T& val)
    {
        Json j = typename Json::object(val.begin(), val.end());
        return j;
    }

    static Json to_json(const T& val, const allocator_type& allocator)
    {
        Json j = typename Json::object(val.begin(), val.end(), allocator);
        return j;
    }
};

template<class Json, class E, size_t N>
struct json_type_traits<Json, std::array<E, N>>
{
    typedef typename Json::allocator_type allocator_type;

    typedef E element_type;

    static bool is(const Json& j) noexcept
    {
        bool result = j.is_array() && j.size() == N;
        if (result)
        {
            for (auto e : j.array_range())
            {
                if (!e.template is<element_type>())
                {
                    result = false;
                    break;
                }
            }
        }
        return result;
    }

    static std::array<E, N> as(const Json& j)
    {
        std::array<E, N> buff;
        JSONCONS_ASSERT(j.size() == N);
        for (size_t i = 0; i < N; i++)
        {
            buff[i] = j[i].template as<E>();
        }
        return buff;
    }

    static Json to_json(const std::array<E, N>& val)
    {
        Json j = typename Json::array();
        j.reserve(N);
        for (auto it = val.begin(); it != val.end(); ++it)
        {
            j.push_back(*it);
        }
        return j;
    }

    static Json to_json(const std::array<E, N>& val, 
                        const allocator_type& allocator)
    {
        Json j = typename Json::array(allocator);
        j.reserve(N);
        for (auto it = val.begin(); it != val.end(); ++it)
        {
            j.push_back(*it);
        }
        return j;
    }
};

namespace detail
{

template<size_t Pos, class Json, class Tuple>
struct json_tuple_helper
{
    using element_type = typename std::tuple_element<Pos - 1, Tuple>::type;
    using next = json_tuple_helper<Pos - 1, Json, Tuple>;
    
    static bool is(const Json& j) noexcept
    {
        if(j[Pos - 1].template is<element_type>())
        {
            return next::is(j);
        }
        else
        {
            return false;
        }
    }

    static void as(Tuple& tuple, const Json& j)
    {
        std::get<Pos - 1>(tuple) = j[Pos - 1].template as<element_type>();
        next::as(tuple, j);
    }

    static void to_json(const Tuple& tuple, std::array<Json, std::tuple_size<Tuple>::value>& jsons)
    {
        jsons[Pos - 1] = json_type_traits<Json, element_type>::to_json(std::get<Pos-1>(tuple));
        next::to_json(tuple, jsons);
    }
};

template<class Json, class Tuple>
struct json_tuple_helper<0, Json, Tuple>
{
    static bool is(const Json&) noexcept
    {
        return true;
    }

    static void as(Tuple&, const Json&)
    {
    }

    static void to_json(const Tuple&, std::array<Json, std::tuple_size<Tuple>::value>&)
    {
    }
};

}

template<class Json, typename... E>
struct json_type_traits<Json, std::tuple<E...>>
{
private:
    using helper = jsoncons::detail::json_tuple_helper<sizeof...(E), Json, std::tuple<E...>>;

public:
    static bool is(const Json& j) noexcept
    {
        return helper::is(j);
    }
    
    static std::tuple<E...> as(const Json& j)
    {
        std::tuple<E...> buff;
        helper::as(buff, j);
        return buff;
    }
    
    static Json to_json(const std::tuple<E...>& val)
    {
        std::array<Json, sizeof...(E)> buf;
        helper::to_json(val, buf);
        return Json(typename Json::array(buf.begin(), buf.end()));
    }
};

template<class Json, class T1, class T2>
struct json_type_traits<Json, std::pair<T1,T2>>
{
public:
    static bool is(const Json& j) noexcept
    {
        return j.is_array() && j.size() == 2;
    }
    
    static std::pair<T1,T2> as(const Json& j)
    {
        return std::make_pair<T1,T2>(j[0].template as<T1>(),j[1].template as<T2>());
    }
    
    static Json to_json(const std::pair<T1,T2>& val)
    {
        return typename Json::array{val.first,val.second};
    }
};

template<class Json, class Allocator>
struct json_type_traits<Json, basic_byte_string<Allocator>>
{
public:
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        return j.is_byte_string();
    }
    
    static basic_byte_string<Allocator> as(const Json& j)
    {
        return j.template as_byte_string<Allocator>();
    }
    
    static Json to_json(const basic_byte_string<Allocator>& val, 
                        const allocator_type& allocator = allocator_type())
    {
        return Json(val, semantic_tag::none, allocator);
    }
};

template<class Json>
struct json_type_traits<Json, byte_string_view>
{
public:
    static bool is(const Json& j) noexcept
    {
        return j.is_byte_string_view();
    }
    
    static byte_string_view as(const Json& j)
    {
        return j.as_byte_string_view();
    }
    
    static Json to_json(const byte_string_view& val)
    {
        return Json(val);
    }
};

// basic_bignum

template<class Json, class Allocator>
struct json_type_traits<Json, basic_bignum<Allocator>>
{
public:
    static bool is(const Json& j) noexcept
    {
        return j.is_bignum();
    }
    
    static basic_bignum<Allocator> as(const Json& j)
    {
        return j.as_bignum();
    }
    
    static Json to_json(const basic_bignum<Allocator>& val)
    {
        std::basic_string<typename Json::char_type> s;
        val.dump(s);
        return Json(s,semantic_tag::big_integer);
    }
};

// std::valarray

template<class Json, class T>
struct json_type_traits<Json, std::valarray<T>>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) noexcept
    {
        bool result = j.is_array();
        if (result)
        {
            for (auto e : j.array_range())
            {
                if (!e.template is<T>())
                {
                    result = false;
                    break;
                }
            }
        }
        return result;
    }
    
    static std::valarray<T> as(const Json& j)
    {
        if (j.is_array())
        {
            std::valarray<T> v(j.size());
            for (size_t i = 0; i < j.size(); ++i)
            {
                v[i] = j[i].template as<T>();
            }
            return v;
        }
        else
        {
            JSONCONS_THROW(json_runtime_error<std::runtime_error>("Attempt to cast json non-array to array"));
        }
    }
    
    static Json to_json(const std::valarray<T>& val)
    {
        Json j = typename Json::array();
        auto first = std::begin(val);
        auto last = std::end(val);
        size_t size = std::distance(first,last);
        j.reserve(size);
        for (auto it = first; it != last; ++it)
        {
            j.push_back(*it);
        }
        return j;
    } 

    static Json to_json(const std::valarray<T>& val, const allocator_type& allocator)
    {
        Json j = typename Json::array(allocator);
        auto first = std::begin(val);
        auto last = std::end(val);
        size_t size = std::distance(first,last);
        j.reserve(size);
        for (auto it = first; it != last; ++it)
        {
            j.push_back(*it);
        }
        return j;
    }
};

}

// This follows https://github.com/Loki-Astari/ThorsSerializer/blob/master/src/Serialize/Traits.h

#define JSONCONS_EXPAND(X) X    

#define JSONCONS_NARGS(...) JSONCONS_EXPAND( JSONCONS_NARG_(__VA_ARGS__, JSONCONS_RSEQ_N()) )
#define JSONCONS_NARG_(...) JSONCONS_EXPAND( JSONCONS_ARG_N(__VA_ARGS__) )
#define JSONCONS_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, N, ...) N
#define JSONCONS_RSEQ_N() 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define JSONCONS_QUOTE(A) JSONCONS_EXPAND(#A)

#define JSONCONS_GENERATE(Act, TC, JV, P1, V, P2)         JSONCONS_EXPAND(JSONCONS_EXPAND(Act(TC, JV, P1, V, P2)))

#define JSONCONS_REP_N(Act, TC, JV, P1, V, ...)            JSONCONS_EXPAND(JSONCONS_REP_OF_N(Act, TC, JV, P1, V, JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__))
#define JSONCONS_REP_OF_N(Act, TC, JV, P1, V, Count, ...)  JSONCONS_EXPAND(JSONCONS_REP_OF_N_(Act, TC, JV, P1, V, Count, __VA_ARGS__))
#define JSONCONS_REP_OF_N_(Act, TC, JV, P1, V, Count, ...) JSONCONS_EXPAND(JSONCONS_REP_OF_ ## Count(Act, TC, JV, P1, V, __VA_ARGS__))

#define JSONCONS_REP_OF_50(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_49(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_49(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_48(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_48(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_47(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_47(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_46(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_46(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_45(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_45(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_44(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_44(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_43(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_43(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_42(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_42(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_41(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_41(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_40(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_40(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_39(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_39(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_38(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_38(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_37(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_37(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_36(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_36(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_35(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_35(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_34(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_34(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_33(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_33(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_32(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_32(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_31(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_31(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_30(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_30(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_29(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_29(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_28(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_28(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_27(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_27(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_26(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_26(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_25(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_25(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_24(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_24(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_23(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_23(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_22(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_22(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_21(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_21(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_20(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_20(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_19(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_19(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_18(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_18(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_17(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_17(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_16(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_16(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_15(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_15(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_14(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_14(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_13(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_13(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_12(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_12(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_11(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_11(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_10(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_10(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_9(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_9(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_8(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_8(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_7(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_7(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_6(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_6(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_5(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_5(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_4(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_4(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_3(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_3(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2); JSONCONS_EXPAND(JSONCONS_REP_OF_2(Act, TC, JV, P1, V, __VA_ARGS__)) 
#define JSONCONS_REP_OF_2(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V, P2); JSONCONS_EXPAND(JSONCONS_REP_OF_1(Act, TC, JV, P1, V, __VA_ARGS__)) 
#define JSONCONS_REP_OF_1(Act, TC, JV, P1, V, P2)          JSONCONS_GENERATE(Act, TC, JV, P1, V, P2);
#define JSONCONS_REP_OF_0(Act, TC, JV, P1, V)              Act(TC, JV, P1, V) ## _LAST 

#define JSONCONS_IS(TC, JV, Type, V, Member) JSONCONS_EXPAND(V = V && (JV).contains(JSONCONS_QUOTE(Member)));
#define JSONCONS_IS_LAST(TC, JV, Type, V)

#define JSONCONS_TO_JSON(TC, JV, Type, V, Member) (JV).try_emplace(JSONCONS_QUOTE(Member), V.Member);
#define JSONCONS_TO_JSON_LAST(TC, JV, Type, V)

#define JSONCONS_AS(TC, JV, Type, V, Member) if ((JV).contains(JSONCONS_QUOTE(Member))) {val.Member = (JV).at(JSONCONS_QUOTE(Member)).template as<decltype(V.Member)>();}
                                         
#define JSONCONS_AS_LAST(TC, JV, Type, V)

#define JSONCONS_TYPE_TRAITS_DECL_BASE(Count, ValueType, ...)  \
namespace jsoncons \
{ \
    template<class Json> \
    struct json_type_traits<Json, ValueType> \
    { \
        typedef typename Json::allocator_type allocator_type; \
        static bool is(const Json& j) noexcept \
        { \
            bool val = j.is_object(); \
            JSONCONS_REP_N(JSONCONS_IS, Count, j, ValueType, val, __VA_ARGS__)\
            return j.is_object(); \
        } \
        static ValueType as(const Json& j) \
        { \
            ValueType val{}; \
            JSONCONS_REP_N(JSONCONS_AS, Count, j, ValueType, val, __VA_ARGS__) \
            return val; \
        } \
        static Json to_json(const ValueType& val, allocator_type allocator=allocator_type()) \
        { \
            Json j(allocator); \
            JSONCONS_REP_N(JSONCONS_TO_JSON, Count, j, ValueType, val, __VA_ARGS__) \
            return j; \
        } \
    }; \
} // jsoncons \


#define JSONCONS_TYPE_TRAITS_DECL(ValueType,...) \
    JSONCONS_TYPE_TRAITS_DECL_BASE(0, ValueType, __VA_ARGS__)

#define JSONCONS_TYPE_TRAITS_FRIEND \
    template <class JSON,class T,class Enable> \
    friend struct jsoncons::json_type_traits;

#endif

