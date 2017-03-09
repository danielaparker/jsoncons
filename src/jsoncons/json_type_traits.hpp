// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_TYPE_TRAITS_HPP
#define JSONCONS_JSON_TYPE_TRAITS_HPP

#include <array>
#include <string>
#include <vector>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <algorithm>
#include <fstream>
#include <limits>
#include <type_traits>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif

namespace jsoncons {

template <class Json, class T, class Enable=void>
struct json_string_type_traits
{
    typedef void char_traits_type;
};

template <class Json, class T>
struct json_string_type_traits<Json, T, typename std::enable_if<std::is_same<typename T::traits_type,typename Json::char_traits_type>::value>::type>
{
    typedef typename Json::char_traits_type char_traits_type;
};

template <class Json, class T, class Enable=void>
struct json_type_traits
{
    static const bool is_compatible = false;

    static bool is(const Json&)
    {
        return false;
    }
};

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
    !std::is_void<typename json_string_type_traits<Json,T>::char_traits_type>::value && 
    !is_incompatible<Json,typename std::iterator_traits<typename T::iterator>::value_type>::value
>::type> : std::true_type {};

// is_compatible_array_type
template<class Json, class T, class Enable=void>
struct is_compatible_array_type : std::false_type {};

template<class Json, class T>
struct is_compatible_array_type<Json,T, 
    typename std::enable_if<!std::is_same<T,typename Json::array>::value &&
    std::is_void<typename json_string_type_traits<Json,T>::char_traits_type>::value && 
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

// is_std_array
template<class T>
struct is_std_array : std::false_type {};

template<class E, size_t N>
struct is_std_array<std::array<E, N>> : std::true_type {};

template <class Json, class T>
class json_array_input_iterator
{
public:
    typedef typename Json::const_array_iterator base_iterator;
    typedef typename std::iterator_traits<base_iterator>::value_type value_type;
    typedef typename std::iterator_traits<base_iterator>::difference_type difference_type;
    typedef typename std::iterator_traits<base_iterator>::pointer pointer;
    typedef T reference;
    typedef std::input_iterator_tag iterator_category;

    json_array_input_iterator()
    {
    }

    json_array_input_iterator(base_iterator it)
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
    base_iterator it_;
};

template <class Json, class T>
class json_object_input_iterator
{
public:
    typedef typename Json::const_object_iterator base_iterator;
    typedef typename std::iterator_traits<base_iterator>::value_type value_type;
    typedef typename std::iterator_traits<base_iterator>::difference_type difference_type;
    typedef typename std::iterator_traits<base_iterator>::pointer pointer;
    typedef T reference;
    typedef std::input_iterator_tag iterator_category;
    typedef typename T::second_type mapped_type;

    json_object_input_iterator()
    {
    }

    json_object_input_iterator(base_iterator it)
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
        return T(it_->key(),json_type_traits<Json,mapped_type>::as(it_->value()));
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
    base_iterator it_;
};

template<class Json>
struct json_type_traits<Json, typename type_wrapper<typename Json::char_type>::const_pointer_type>
{
    typedef typename Json::char_type char_type;
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_string();
    }
    static const char_type* as(const Json& rhs)
    {
        return rhs.as_cstring();
    }
    static Json to_json(const char_type* val)
    {
        return Json(typename Json::variant(val));
    }
    static Json to_json(const char_type* val, 
                        const allocator_type& allocator)
    {
        return Json(typename Json::variant(val,allocator));
    }
};

template<class Json>
struct json_type_traits<Json, typename type_wrapper<typename Json::char_type>::pointer_type>
{
    typedef typename Json::char_type char_type;
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_string();
    }
    static Json to_json(const char_type *val)
    {
        return Json(typename Json::variant(val));
    }
    static Json to_json(const char_type *val, 
                        const allocator_type& allocator)
    {
        return Json(typename Json::variant(val,allocator));
    }
};

// integral

template<class Json, class T>
struct json_type_traits<Json, T,
                        typename std::enable_if<std::is_integral<T>::value &&
                        std::is_signed<T>::value &&
                        !std::is_same<T,bool>::value
>::type>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        if (rhs.is_integer())
        {
            return rhs.as_integer() >= (std::numeric_limits<T>::min)() && rhs.as_integer() <= (std::numeric_limits<T>::max)();
        }
        else if (rhs.is_uinteger())
        {
            return rhs.as_uinteger() <= static_cast<uint64_t>((std::numeric_limits<T>::max)());
        }
        else
        {
            return false;
        }
    }
    static T as(const Json& rhs)
    {
        return static_cast<T>(rhs.as_integer());
    }
    static Json to_json(T val)
    {
        return Json(typename Json::variant(static_cast<int64_t>(val)));
    }
    static Json to_json(T val, allocator_type)
    {
        return Json(typename Json::variant(static_cast<int64_t>(val)));
    }
};

template<class Json, class T>
struct json_type_traits<Json, T,
                        typename std::enable_if<std::is_integral<T>::value &&
                        std::is_unsigned<T>::value &&
                        !std::is_same<T,bool>::value
>::type >
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        if (rhs.is_integer())
        {
            return rhs.as_integer() >= 0 && static_cast<uint64_t>(rhs.as_integer()) <= (std::numeric_limits<T>::max)();
        }
        else if (rhs.is_uinteger())
        {
            return rhs.as_uinteger() <= (std::numeric_limits<T>::max)();
        }
        else
        {
            return false;
        }
    }
    static T as(const Json& rhs)
    {
        return static_cast<T>(rhs.as_uinteger());
    }

    static Json to_json(T val)
    {
        return Json(typename Json::variant(static_cast<uint64_t>(val)));
    }

    static Json to_json(T val, allocator_type)
    {
        return Json(typename Json::variant(static_cast<uint64_t>(val)));
    }
};

template<class Json,class T>
struct json_type_traits<Json, T,
                        typename std::enable_if<std::is_floating_point<T>::value
>::type>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_double();
    }
    static T as(const Json& rhs)
    {
        return static_cast<T>(rhs.as_double());
    }
    static Json to_json(T val)
    {
        return Json(typename Json::variant(static_cast<double>(val)));
    }
    static Json to_json(T val, const allocator_type&)
    {
        return Json(typename Json::variant(static_cast<double>(val)));
    }
};

template<class Json>
struct json_type_traits<Json, typename Json::object>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_object();
    }
    static Json to_json(const typename Json::object& val)
    {
        return Json(typename Json::variant(val));
    }
    static Json to_json(const typename Json::object& val, 
                        const allocator_type& allocator)
    {
        return Json(typename Json::variant(val,allocator));
    }
};

template<class Json>
struct json_type_traits<Json, typename Json::array>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_array();
    }
    static Json to_json(const typename Json::array& val)
    {
        return Json(typename Json::variant(val));
    }
    static Json to_json(const typename Json::array& val, 
                        const allocator_type& allocator)
    {
        return Json(typename Json::variant(val,allocator));
    }
};

template<class Json>
struct json_type_traits<Json, Json>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json&) JSONCONS_NOEXCEPT
    {
        return true;
    }
    static Json as(Json rhs)
    {
        return rhs;
    }
    static Json to_json(const Json& val)
    {
        return val;
    }
    static Json to_json(const Json& val, allocator_type)
    {
        return val;
    }
};

template<class Json>
struct json_type_traits<Json, jsoncons::null_type>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_null();
    }
    static typename jsoncons::null_type as(const Json& rhs)
    {
        JSONCONS_ASSERT(rhs.is_null());
        return jsoncons::null_type();
    }
    static Json to_json(jsoncons::null_type)
    {
        return Json::null();
    }
    static Json to_json(jsoncons::null_type, allocator_type)
    {
        return Json::null();
    }
};

template<class Json>
struct json_type_traits<Json, bool>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_bool();
    }
    static bool as(const Json& rhs)
    {
        return rhs.as_bool();
    }
    static Json to_json(bool val)
    {
        return Json(typename Json::variant(val));
    }
    static Json to_json(bool val, allocator_type)
    {
        return Json(typename Json::variant(val));
    }
};

template<class Json,class T>
struct json_type_traits<Json, T, typename std::enable_if<std::is_same<T, 
    std::conditional<!std::is_same<bool,std::vector<bool>::const_reference>::value,
                     std::vector<bool>::const_reference,
                     void>::type>::value>::type>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_bool();
    }
    static bool as(const Json& rhs)
    {
        return rhs.as_bool();
    }
    static Json to_json(bool val)
    {
        return Json(typename Json::variant(val));
    }
    static Json to_json(bool val, allocator_type)
    {
        return Json(typename Json::variant(val));
    }
};

template<class Json>
struct json_type_traits<Json, std::vector<bool>::reference>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_bool();
    }
    static bool as(const Json& rhs)
    {
        return rhs.as_bool();
    }
    static Json to_json(std::vector<bool>::reference val)
    {
        return Json(typename Json::variant(val));
    }
    static Json to_json(std::vector<bool>::reference val, 
                        allocator_type)
    {
        return Json(typename Json::variant(val));
    }
};

template<class Json, typename T>
struct json_type_traits<Json, T, 
                        typename std::enable_if<is_compatible_array_type<Json,T>::value && !is_std_array<T>::value>::type>
{
    typedef typename std::iterator_traits<typename T::iterator>::value_type element_type;
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        bool result = rhs.is_array();
        if (result)
        {
            for (auto e : rhs.array_range())
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

    static T as(const Json& rhs)
    {
        if (rhs.is_array())
        {
            T v(json_array_input_iterator<Json, element_type>(rhs.array_range().begin()),
                json_array_input_iterator<Json, element_type>(rhs.array_range().end()));
            return v;
        }
        else
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempt to cast json non-array to array");
        }
    }

    static Json to_json(const T& val)
    {
        return Json(std::begin(val), std::end(val));
    }

    static Json to_json(const T& val, const allocator_type& allocator)
    {
        return Json(std::begin(val), std::end(val), allocator);
    }
};

template<class Json, typename T>
struct json_type_traits<Json, T, 
                        typename std::enable_if<is_compatible_string_type<Json,T>::value>::type>
{
    typedef typename std::iterator_traits<typename T::iterator>::value_type element_type;
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_string();
    }

    static T as(const Json& rhs)
    {
        if (rhs.is_string())
        {
            return rhs.as_string_view();
        }
        else
        {
            T s;
            rhs.dump(s);
            return s;
        }
    }

    static Json to_json(const T& val)
    {
        return Json(typename Json::variant(val.data(), val.size()));
    }

    static Json to_json(const T& val, const allocator_type& allocator)
    {
        return Json(typename Json::variant(val.data(),val.size(),allocator));
    }
};

template<class Json, typename T>
struct json_type_traits<Json, T, 
                        typename std::enable_if<is_compatible_object_type<Json,T>::value>::type
>
{
    typedef typename T::mapped_type mapped_type;
    typedef typename T::value_type value_type;
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        bool result = rhs.is_object();
        for (auto member : rhs.object_range())
        {
            if (!member.value().template is<mapped_type>())
            {
                result = false;
            }
        }
        return result;
    }

    static T as(const Json& rhs)
    {
        T v(json_object_input_iterator<Json,value_type>(rhs.object_range().begin()),
            json_object_input_iterator<Json,value_type>(rhs.object_range().end()));
        return v;
    }

    static Json to_json(const T& val)
    {
        Json j;
        j.reserve(val.size());
        for (auto p: val)
        {
            j.set(p.first, p.second);
        }
        return j;
    }

    static Json to_json(const T& val, const allocator_type& allocator)
    {
        Json j(allocator);
        j.reserve(val.size());
        for (auto p: val)
        {
            j.set(p.first, p.second);
        }
        return j;
    }
};

template<class Json, class E, size_t N>
struct json_type_traits<Json, std::array<E, N>>
{
    typedef typename Json::allocator_type allocator_type;

    typedef E element_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        bool result = rhs.is_array() && rhs.size() == N;
        if (result)
        {
            for (auto e : rhs.array_range())
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

    static std::array<E, N> as(const Json& json)
    {
        std::array<E, N> buff;
        JSONCONS_ASSERT(json.size() == N);
        for (size_t i = 0; i < N; i++)
        {
            buff[i] = json[i].template as<E>();
        }
        return buff;
    }

    static Json to_json(const std::array<E, N>& val)
    {
        return Json(val.begin(), val.end());
    }

    static Json to_json(const std::array<E, N>& val, 
                        const allocator_type& allocator)
    {
        return Json(val.begin(), val.end(), allocator);
    }
};

namespace detail
{

template<size_t __pos, class Json, class Tuple>
struct json_tuple_helper
{
	using element_type = typename std::tuple_element<__pos - 1, Tuple>::type;
	using next = json_tuple_helper<__pos - 1, Json, Tuple>;
	
	static bool is(const Json& json) JSONCONS_NOEXCEPT
	{
		if(json[__pos - 1].template is<element_type>())
		{
			return next::is(json);
		}
		else
		{
			return false;
		}
	}

	static void as(Tuple& tuple, const Json& json)
	{
		std::get<__pos - 1>(tuple) = json[__pos - 1].template as<element_type>();
		next::as(tuple, json);
	}

	static void to_json(const Tuple& tuple, std::array<Json, std::tuple_size<Tuple>::value>& jsons)
	{
		jsons[__pos - 1] = json_type_traits<Json, element_type>::to_json(std::get<__pos>(tuple));
		next::to_json(tuple, jsons);
	}
};

template<class Json, class Tuple>
struct json_tuple_helper<0, Json, Tuple>
{
	static bool is(const Json& json) JSONCONS_NOEXCEPT
	{
		return true;
	}

	static void as(Tuple& tuple, const Json& json)
	{
	}

	static void to_json(const Tuple& tuple, Json& json)
	{
	}
};

}

template<class Json, typename... E>
struct json_type_traits<Json, std::tuple<E...>>
{
private:
	using helper = detail::json_tuple_helper<sizeof...(E), Json, std::tuple<E...>>;

public:
	static bool is(const Json& rhs) JSONCONS_NOEXCEPT
	{
		return helper::is(rhs);
	}
	
	static std::tuple<E...> as(const Json& json)
	{
		std::tuple<E...> buff;
		helper::as(buff, json);
		return buff;
	}
	
	static Json to_json(const std::tuple<E...>& value)
	{
		std::array<Json, sizeof...(E)> buf;
		helper::to_hson(value, buf);
		return J(buf.begin(), buf.end());
	}
};

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
