// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_HPP
#define JSONCONS_JSON_HPP

#include <limits>
#include <string>
#include <vector>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <memory>
#include <typeinfo>
#include <cstring>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/detail/jsoncons_utilities.hpp>
#include <jsoncons/json_traits.hpp>
#include <jsoncons/json_structures.hpp>
#include <jsoncons/json_output_handler.hpp>
#include <jsoncons/serialization_options.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons/json_reader.hpp>
#include <jsoncons/json_type_traits.hpp>
#include <jsoncons/json_error_category.hpp>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif

namespace jsoncons {

template <typename IteratorT>
class range 
{
    IteratorT first_;
    IteratorT last_;
public:
    range(const IteratorT& first, const IteratorT& last)
        : first_(first), last_(last)
    {
    }

public:
    IteratorT begin()
    {
        return first_;
    }
    IteratorT end()
    {
        return last_;
    }
};

enum class json_type_tag : uint8_t 
{
    null_t = 0,
    bool_t,
    integer_t,
    uinteger_t,
    double_t,
    small_string_t,
    string_t,
    bstring_t,
    array_t,
    empty_object_t,
    object_t
};

enum class tt : uint8_t 
{
    null_null,null_bool,null_integer,null_uinteger,null_double,null_smallstr,null_string,null_bstring,null_array,null_emptyobj,null_object,
    bool_null,bool_bool,bool_integer,bool_uinteger,bool_double,bool_smallstr,bool_string,bool_bstring,bool_array,bool_emptyobj,bool_object,
    integer_null,integer_bool,integer_integer,integer_uinteger,integer_double,integer_smallstr,integer_string,integer_bstring,integer_array,integer_emptyobj,integer_object,
    uinteger_null,uinteger_bool,uinteger_integer,uinteger_uinteger,uinteger_double,uinteger_smallstr,uinteger_string,uinteger_bstring,uinteger_array,uinteger_emptyobj,uinteger_object,
    double_null,double_bool,double_integer,double_uinteger,double_double,double_smallstr,double_string,double_bstring,double_array,double_emptyobj,double_object,
    smallstr_null,smallstr_bool,smallstr_integer,smallstr_uinteger,smallstr_double,smallstr_smallstr,smallstr_string,smallstr_bstring,smallstr_array,smallstr_emptyobj,smallstr_object,
    string_null,string_bool,string_integer,string_uinteger,string_double,string_smallstr,string_string,string_bstring,string_array,string_emptyobj,string_object,
    bstring_null,bstring_bool,bstring_integer,bstring_uinteger,bstring_double,bstring_smallstr,bstring_string,bstring_bstring,bstring_array,bstring_emptyobj,bstring_object,
    array_null,array_bool,array_integer,array_uinteger,array_double,array_smallstr,array_string,array_bstring,array_array,array_emptyobj,array_object,
    emptyobj_null,emptyobj_bool,emptyobj_integer,emptyobj_uinteger,emptyobj_double,emptyobj_smallstr,emptyobj_string,emptyobj_bstring,emptyobj_array,emptyobj_emptyobj,emptyobj_object,
    object_null,object_bool,object_integer,object_uinteger,object_double,object_smallstr,object_string,object_bstring,object_array,object_emptyobj,object_object
};

template<typename unused=void>
struct json_globals {
    static const tt t_by_t[11][11];
};
typedef json_globals<> globals;

template<typename unused>
const tt json_globals<unused>::t_by_t[11][11] = {
    {tt::null_null,tt::null_bool,tt::null_integer,tt::null_uinteger,tt::null_double,tt::null_smallstr,tt::null_string,tt::null_bstring,tt::null_array,tt::null_emptyobj,tt::null_object},
    {tt::bool_null,tt::bool_bool,tt::bool_integer,tt::bool_uinteger,tt::bool_double,tt::bool_smallstr,tt::bool_string,tt::bool_bstring,tt::bool_array,tt::bool_emptyobj,tt::bool_object},
    {tt::integer_null,tt::integer_bool,tt::integer_integer,tt::integer_uinteger,tt::integer_double,tt::integer_smallstr,tt::integer_string,tt::integer_bstring,tt::integer_array,tt::integer_emptyobj,tt::integer_object},
    {tt::uinteger_null,tt::uinteger_bool,tt::uinteger_integer,tt::uinteger_uinteger,tt::uinteger_double,tt::uinteger_smallstr,tt::uinteger_string,tt::uinteger_bstring,tt::uinteger_array,tt::uinteger_emptyobj,tt::uinteger_object},
    {tt::double_null,tt::double_bool,tt::double_integer,tt::double_uinteger,tt::double_double,tt::double_smallstr,tt::double_string,tt::double_bstring,tt::double_array,tt::double_emptyobj,tt::double_object},
    {tt::smallstr_null,tt::smallstr_bool,tt::smallstr_integer,tt::smallstr_uinteger,tt::smallstr_double,tt::smallstr_smallstr,tt::smallstr_string,tt::smallstr_bstring,tt::smallstr_array,tt::smallstr_emptyobj,tt::smallstr_object},
    {tt::string_null,tt::string_bool,tt::string_integer,tt::string_uinteger,tt::string_double,tt::string_smallstr,tt::string_string,tt::string_bstring,tt::string_array,tt::string_emptyobj,tt::string_object},
    {tt::bstring_null,tt::bstring_bool,tt::bstring_integer,tt::bstring_uinteger,tt::bstring_double,tt::bstring_smallstr,tt::bstring_string,tt::bstring_bstring,tt::bstring_array,tt::bstring_emptyobj,tt::bstring_object},
    {tt::array_null,tt::array_bool,tt::array_integer,tt::array_uinteger,tt::array_double,tt::array_smallstr,tt::array_string,tt::array_bstring,tt::array_array,tt::array_emptyobj,tt::array_object},
    {tt::emptyobj_null,tt::emptyobj_bool,tt::emptyobj_integer,tt::emptyobj_uinteger,tt::emptyobj_double,tt::emptyobj_smallstr,tt::emptyobj_string,tt::emptyobj_bstring,tt::emptyobj_array,tt::emptyobj_emptyobj,tt::emptyobj_object},
    {tt::object_null,tt::object_bool,tt::object_integer,tt::object_uinteger,tt::object_double,tt::object_smallstr,tt::object_string,tt::object_bstring,tt::object_array,tt::object_emptyobj,tt::object_object}
};
                        
template <class CharT, 
          class JsonTraits = json_traits<CharT>, 
          class Allocator = std::allocator<CharT>>
class basic_json
{
public:

    typedef Allocator allocator_type;

    typedef JsonTraits json_traits_type;

    typedef typename JsonTraits::parse_error_handler_type parse_error_handler_type;

    typedef CharT char_type;
    typedef typename json_traits_type::char_traits_type char_traits_type;

#if !defined(JSONCONS_HAS_STRING_VIEW)
    typedef Basic_string_view_<char_type,char_traits_type> string_view_type;
#else
    typedef std::basic_string_view<char_type,char_traits_type> string_view_type;
#endif
    // string_type is for interface only, not storage 
    typedef std::basic_string<CharT,char_traits_type> string_type;

    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<char_type> char_allocator_type;

    using key_storage_type = typename json_traits_type::template key_storage<char_allocator_type>;

    using string_storage_type = typename json_traits_type::template string_storage<char_allocator_type>;

    typedef basic_json<CharT,JsonTraits,Allocator> value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef key_value_pair<key_storage_type,value_type> key_value_pair_type;

#if !defined(JSONCONS_NO_DEPRECATED)
    typedef value_type json_type;
    typedef key_value_pair_type kvp_type;
    typedef key_value_pair_type member_type;
    typedef jsoncons::null_type null_type;
#endif

    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<basic_json> val_allocator_type;
    using array_storage_type = typename json_traits_type::template array_storage<basic_json, val_allocator_type>;

    typedef json_array<basic_json> array;

    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<key_value_pair_type> kvp_allocator_type;

    using object_storage_type = typename json_traits_type::template object_storage<key_value_pair_type , kvp_allocator_type>;
    typedef json_object<key_storage_type,basic_json,json_traits_type::preserve_order> object;

    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<array> array_allocator;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<object> object_allocator;

    typedef typename object::iterator object_iterator;
    typedef typename object::const_iterator const_object_iterator;
    typedef typename array::iterator array_iterator;
    typedef typename array::const_iterator const_array_iterator;

    struct variant
    {
        struct base_data
        {
            json_type_tag type_id_;

            base_data(json_type_tag id)
                : type_id_(id)
            {}
        };

        struct null_data : public base_data
        {
            null_data()
                : base_data(json_type_tag::null_t)
            {
            }
        };

        struct empty_object_data : public base_data
        {
            empty_object_data()
                : base_data(json_type_tag::empty_object_t)
            {
            }
        };

        struct bool_data : public base_data
        {
            bool val_;

            bool_data(bool val)
                : base_data(json_type_tag::bool_t),val_(val)
            {
            }

            bool_data(const bool_data& val)
                : base_data(json_type_tag::bool_t),val_(val.val_)
            {
            }

            bool value() const
            {
                return val_;
            }

        };

        struct integer_data : public base_data
        {
            int64_t val_;

            integer_data(int64_t val)
                : base_data(json_type_tag::integer_t),val_(val)
            {
            }

            integer_data(const integer_data& val)
                : base_data(json_type_tag::integer_t),val_(val.val_)
            {
            }

            int64_t value() const
            {
                return val_;
            }
        };

        struct uinteger_data : public base_data
        {
            uint64_t val_;

            uinteger_data(uint64_t val)
                : base_data(json_type_tag::uinteger_t),val_(val)
            {
            }

            uinteger_data(const uinteger_data& val)
                : base_data(json_type_tag::uinteger_t),val_(val.val_)
            {
            }

            uint64_t value() const
            {
                return val_;
            }
        };

        struct double_data : public base_data
        {
            uint8_t precision_;
            double val_;

            double_data(double val, uint8_t precision)
                : base_data(json_type_tag::double_t), 
                  precision_(precision), 
                  val_(val)
            {
            }

            double_data(const double_data& val)
                : base_data(json_type_tag::double_t),
                  precision_(val.precision_), 
                  val_(val.val_)
            {
            }

            double value() const
            {
                return val_;
            }

            uint8_t precision() const
            {
                return precision_;
            }
        };

        struct small_string_data : public base_data
        {
            static const size_t capacity = 14/sizeof(char_type);
            static const size_t max_length = (14 / sizeof(char_type)) - 1;

            uint8_t length_;
            char_type data_[capacity];

            small_string_data(const char_type* p, uint8_t length)
                : base_data(json_type_tag::small_string_t), length_(length)
            {
                JSONCONS_ASSERT(length <= max_length);
                std::memcpy(data_,p,length*sizeof(char_type));
                data_[length] = 0;
            }

            small_string_data(const small_string_data& val)
                : base_data(json_type_tag::small_string_t), length_(val.length_)
            {
                std::memcpy(data_,val.data_,val.length_*sizeof(char_type));
                data_[length_] = 0;
            }

            uint8_t length() const
            {
                return length_;
            }

            const char_type* data() const
            {
                return data_;
            }

            const char_type* c_str() const
            {
                return data_;
            }
        };
        struct string_data : public base_data
        {
            typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<Json_string_<basic_json>> string_holder_allocator_type;
            typedef typename std::allocator_traits<string_holder_allocator_type>::pointer pointer;

            pointer ptr_;

            template <typename... Args>
            void create(string_holder_allocator_type allocator, Args&& ... args)
            {
                typename std::allocator_traits<Allocator>:: template rebind_alloc<Json_string_<basic_json>> alloc(allocator);
                ptr_ = alloc.allocate(1);
                try
                {
                    std::allocator_traits<string_holder_allocator_type>:: template rebind_traits<Json_string_<basic_json>>::construct(alloc, to_plain_pointer(ptr_), std::forward<Args>(args)...);
                }
                catch (...)
                {
                    alloc.deallocate(ptr_,1);
                    throw;
                }
            }

            string_data(const Json_string_<basic_json>& val)
                : base_data(json_type_tag::string_t)
            {
                create(val.get_allocator(), val);
            }

            string_data(pointer ptr)
                : base_data(json_type_tag::string_t)
            {
                ptr_ = ptr;
            }

            string_data(const Json_string_<basic_json>& val, const Allocator& a)
                : base_data(json_type_tag::string_t)
            {
                create(string_holder_allocator_type(a), val, a);
            }

            string_data(const string_data & val)
                : base_data(json_type_tag::string_t)
            {
                create(val.ptr_->get_allocator(), *(val.ptr_));
            }

            string_data(const string_data & val, const Allocator& a)
                : base_data(json_type_tag::string_t)
            {
                create(string_holder_allocator_type(a), *(val.ptr_), a);
            }

            template<class InputIterator>
            string_data(InputIterator first, InputIterator last, const Allocator& a)
                : base_data(json_type_tag::string_t)
            {
                create(string_holder_allocator_type(a), first, last, a);
            }

            string_data(const char_type* data, size_t length, const Allocator& a)
                : base_data(json_type_tag::string_t)
            {
                create(string_holder_allocator_type(a), data, length, a);
            }

            ~string_data()
            {
                typename std::allocator_traits<string_holder_allocator_type>:: template rebind_alloc<Json_string_<basic_json>> alloc(ptr_->get_allocator());
                std::allocator_traits<string_holder_allocator_type>:: template rebind_traits<Json_string_<basic_json>>::destroy(alloc, to_plain_pointer(ptr_));
                alloc.deallocate(ptr_,1);
            }

            const char_type* data() const
            {
                return ptr_->data();
            }

            const char_type* c_str() const
            {
                return ptr_->c_str();
            }

            size_t length() const
            {
                return ptr_->length();
            }

            allocator_type get_allocator() const
            {
                return ptr_->get_allocator();
            }
        };

        struct object_data : public base_data
        {
            typedef typename std::allocator_traits<object_allocator>::pointer pointer;
            pointer ptr_;

            template <typename... Args>
            void create(Allocator allocator, Args&& ... args)
            {
                typename std::allocator_traits<object_allocator>:: template rebind_alloc<object> alloc(allocator);
                ptr_ = alloc.allocate(1);
                try
                {
                    std::allocator_traits<object_allocator>:: template rebind_traits<object>::construct(alloc, to_plain_pointer(ptr_), std::forward<Args>(args)...);
                }
                catch (...)
                {
                    alloc.deallocate(ptr_,1);
                    throw;
                }
            }

            explicit object_data(const Allocator& a)
                : base_data(json_type_tag::object_t)
            {
                create(a,a);
            }

            explicit object_data(pointer ptr)
                : base_data(json_type_tag::object_t)
            {
                ptr_ = ptr;
            }

            explicit object_data(const object & val)
                : base_data(json_type_tag::object_t)
            {
                create(val.get_allocator(), val);
            }

            explicit object_data(const object & val, const Allocator& a)
                : base_data(json_type_tag::object_t)
            {
                create(object_allocator(a), val, a);
            }

            explicit object_data(const object_data & val)
                : base_data(json_type_tag::object_t)
            {
                create(val.ptr_->get_allocator(), *(val.ptr_));
            }

            explicit object_data(const object_data & val, const Allocator& a)
                : base_data(json_type_tag::object_t)
            {
                create(object_allocator(a), *(val.ptr_), a);
            }

            ~object_data()
            {
                typename std::allocator_traits<Allocator>:: template rebind_alloc<object> alloc(ptr_->get_allocator());
                std::allocator_traits<Allocator>:: template rebind_traits<object>::destroy(alloc, to_plain_pointer(ptr_));
                alloc.deallocate(ptr_,1);
            }

            object& value()
            {
                return *ptr_;
            }

            const object& value() const
            {
                return *ptr_;
            }

            allocator_type get_allocator() const
            {
                return ptr_->get_allocator();
            }
        };
    public:
        struct array_data : public base_data
        {
            typedef typename std::allocator_traits<array_allocator>::pointer pointer;
            pointer ptr_;

            template <typename... Args>
            void create(array_allocator allocator, Args&& ... args)
            {
                typename std::allocator_traits<Allocator>:: template rebind_alloc<array> alloc(allocator);
                ptr_ = alloc.allocate(1);
                try
                {
                    std::allocator_traits<array_allocator>:: template rebind_traits<array>::construct(alloc, to_plain_pointer(ptr_), std::forward<Args>(args)...);
                }
                catch (...)
                {
                    alloc.deallocate(ptr_,1);
                    throw;
                }
            }

            allocator_type get_allocator() const
            {
                return ptr_->get_allocator();
            }

            array_data(const array& val)
                : base_data(json_type_tag::array_t)
            {
                create(val.get_allocator(), val);
            }

            array_data(pointer ptr)
                : base_data(json_type_tag::array_t)
            {
                ptr_ = ptr;
            }

            array_data(const array& val, const Allocator& a)
                : base_data(json_type_tag::array_t)
            {
                create(array_allocator(a), val, a);
            }

            array_data(const array_data & val)
                : base_data(json_type_tag::array_t)
            {
                create(val.ptr_->get_allocator(), *(val.ptr_));
            }

            array_data(const array_data & val, const Allocator& a)
                : base_data(json_type_tag::array_t)
            {
                create(array_allocator(a), *(val.ptr_), a);
            }

            template<class InputIterator>
            array_data(InputIterator first, InputIterator last, const Allocator& a)
                : base_data(json_type_tag::array_t)
            {
                create(array_allocator(a), first, last, a);
            }

            ~array_data()
            {
                typename std::allocator_traits<array_allocator>:: template rebind_alloc<array> alloc(ptr_->get_allocator());
                std::allocator_traits<array_allocator>:: template rebind_traits<array>::destroy(alloc, to_plain_pointer(ptr_));
                alloc.deallocate(ptr_,1);
            }

            array& value()
            {
                return *ptr_;
            }

            const array& value() const
            {
                return *ptr_;
            }
        };

    private:
        static const size_t data_size = static_max<sizeof(uinteger_data),sizeof(double_data),sizeof(small_string_data), sizeof(string_data), sizeof(array_data), sizeof(object_data)>::value;
        static const size_t data_align = static_max<JSONCONS_ALIGNOF(uinteger_data),JSONCONS_ALIGNOF(double_data),JSONCONS_ALIGNOF(small_string_data),JSONCONS_ALIGNOF(string_data),JSONCONS_ALIGNOF(array_data),JSONCONS_ALIGNOF(object_data)>::value;

        typedef typename std::aligned_storage<data_size,data_align>::type data_t;

        data_t data_;
    public:
        variant()
        {
            new(reinterpret_cast<void*>(&data_))empty_object_data();
        }

        variant(const Allocator& a)
        {
            new(reinterpret_cast<void*>(&data_))object_data(a);
        }

        variant(const variant& val)
        {
            Init_(val);
        }

        variant(const variant& val, const Allocator& allocator)
        {
            Init_(val,allocator);
        }

        variant(variant&& val) JSONCONS_NOEXCEPT
        {
            Init_rv_(std::forward<variant>(val));
        }

        variant(variant&& val, const Allocator& allocator) JSONCONS_NOEXCEPT
        {
            Init_rv_(std::forward<variant>(val), allocator,
                     typename std::allocator_traits<Allocator>::propagate_on_container_move_assignment());
        }

        explicit variant(null_type)
        {
            new(reinterpret_cast<void*>(&data_))null_data();
        }
        explicit variant(bool val)
        {
            new(reinterpret_cast<void*>(&data_))bool_data(val);
        }
        explicit variant(int64_t val)
        {
            new(reinterpret_cast<void*>(&data_))integer_data(val);
        }
        explicit variant(uint64_t val, const Allocator&)
        {
            new(reinterpret_cast<void*>(&data_))uinteger_data(val);
        }
        explicit variant(uint64_t val)
        {
            new(reinterpret_cast<void*>(&data_))uinteger_data(val);
        }
        variant(double val)
        {
            new(reinterpret_cast<void*>(&data_))double_data(val,std::numeric_limits<double>::digits10);
        }
        variant(double val, uint8_t precision)
        {
            new(reinterpret_cast<void*>(&data_))double_data(val,precision);
        }
        variant(const char_type* s, size_t length)
        {
            if (length <= small_string_data::max_length)
            {
                new(reinterpret_cast<void*>(&data_))small_string_data(s, static_cast<uint8_t>(length));
            }
            else
            {
                new(reinterpret_cast<void*>(&data_))string_data(s, length, char_allocator_type());
            }
        }
        variant(const char_type* s)
        {
            size_t length = char_traits_type::length(s);
            if (length <= small_string_data::max_length)
            {
                new(reinterpret_cast<void*>(&data_))small_string_data(s, static_cast<uint8_t>(length));
            }
            else
            {
                new(reinterpret_cast<void*>(&data_))string_data(s, length, char_allocator_type());
            }
        }

        variant(const char_type* s, const Allocator& alloc)
        {
            size_t length = char_traits_type::length(s);
            if (length <= small_string_data::max_length)
            {
                new(reinterpret_cast<void*>(&data_))small_string_data(s, static_cast<uint8_t>(length));
            }
            else
            {
                new(reinterpret_cast<void*>(&data_))string_data(s, length, alloc);
            }
        }

        variant(const char_type* s, size_t length, const Allocator& alloc)
        {
            if (length <= small_string_data::max_length)
            {
                new(reinterpret_cast<void*>(&data_))small_string_data(s, static_cast<uint8_t>(length));
            }
            else
            {
                new(reinterpret_cast<void*>(&data_))string_data(s, length, alloc);
            }
        }
        variant(const object& val)
        {
            new(reinterpret_cast<void*>(&data_))object_data(val);
        }
        variant(const object& val, const Allocator& alloc)
        {
            new(reinterpret_cast<void*>(&data_))object_data(val, alloc);
        }
        variant(const array& val)
        {
            new(reinterpret_cast<void*>(&data_))array_data(val);
        }
        variant(const array& val, const Allocator& alloc)
        {
            new(reinterpret_cast<void*>(&data_))array_data(val,alloc);
        }
        template<class InputIterator>
        variant(InputIterator first, InputIterator last, const Allocator& a)
        {
            new(reinterpret_cast<void*>(&data_))array_data(first, last, a);
        }

        ~variant()
        {
            Destroy_();
        }

        void Destroy_()
        {
            switch (type_id())
            {
            case json_type_tag::string_t:
                reinterpret_cast<string_data*>(&data_)->~string_data();
                break;
            case json_type_tag::object_t:
                reinterpret_cast<object_data*>(&data_)->~object_data();
                break;
            case json_type_tag::array_t:
                reinterpret_cast<array_data*>(&data_)->~array_data();
                break;
            default:
                break;
            }
        }

        variant& operator=(const variant& val)
        {
            if (this != &val)
            {
                Destroy_();
                switch (val.type_id())
                {
                case json_type_tag::null_t:
                    new(reinterpret_cast<void*>(&data_))null_data();
                    break;
                case json_type_tag::empty_object_t:
                    new(reinterpret_cast<void*>(&data_))empty_object_data();
                    break;
                case json_type_tag::double_t:
                    new(reinterpret_cast<void*>(&data_))double_data(*(val.double_data_cast()));
                    break;
                case json_type_tag::integer_t:
                    new(reinterpret_cast<void*>(&data_))integer_data(*(val.integer_data_cast()));
                    break;
                case json_type_tag::uinteger_t:
                    new(reinterpret_cast<void*>(&data_))uinteger_data(*(val.uinteger_data_cast()));
                    break;
                case json_type_tag::bool_t:
                    new(reinterpret_cast<void*>(&data_))bool_data(*(val.bool_data_cast()));
                    break;
                case json_type_tag::small_string_t:
                    new(reinterpret_cast<void*>(&data_))small_string_data(*(val.small_string_data_cast()));
                    break;
                case json_type_tag::string_t:
                    new(reinterpret_cast<void*>(&data_))string_data(*(val.string_data_cast()));
                    break;
                case json_type_tag::object_t:
                    new(reinterpret_cast<void*>(&data_))object_data(*(val.object_data_cast()));
                    break;
                case json_type_tag::array_t:
                    new(reinterpret_cast<void*>(&data_))array_data(*(val.array_data_cast()));
                    break;
                default:
                    break;
                }
            }
            return *this;
        }

        variant& operator=(variant&& val) JSONCONS_NOEXCEPT
        {
            if (this != &val)
            {
                Destroy_();
                new(reinterpret_cast<void*>(&data_))null_data();
                swap(val);
            }
            return *this;
        }

        json_type_tag type_id() const
        {
            return reinterpret_cast<const base_data*>(&data_)->type_id_;
        }

        const null_data* null_data_cast() const
        {
            return reinterpret_cast<const null_data*>(&data_);
        }

        const empty_object_data* empty_object_data_cast() const
        {
            return reinterpret_cast<const empty_object_data*>(&data_);
        }

        const bool_data* bool_data_cast() const
        {
            return reinterpret_cast<const bool_data*>(&data_);
        }

        const integer_data* integer_data_cast() const
        {
            return reinterpret_cast<const integer_data*>(&data_);
        }

        const uinteger_data* uinteger_data_cast() const
        {
            return reinterpret_cast<const uinteger_data*>(&data_);
        }

        const double_data* double_data_cast() const
        {
            return reinterpret_cast<const double_data*>(&data_);
        }

        const small_string_data* small_string_data_cast() const
        {
            return reinterpret_cast<const small_string_data*>(&data_);
        }

        string_data* string_data_cast()
        {
            return reinterpret_cast<string_data*>(&data_);
        }

        const string_data* string_data_cast() const
        {
            return reinterpret_cast<const string_data*>(&data_);
        }

        object_data* object_data_cast()
        {
            return reinterpret_cast<object_data*>(&data_);
        }

        const object_data* object_data_cast() const
        {
            return reinterpret_cast<const object_data*>(&data_);
        }

        array_data* array_data_cast()
        {
            return reinterpret_cast<array_data*>(&data_);
        }

        const array_data* array_data_cast() const
        {
            return reinterpret_cast<const array_data*>(&data_);
        }

        string_view_type as_string_view() const
        {
            switch (type_id())
            {
            case json_type_tag::small_string_t:
                return string_view_type(small_string_data_cast()->data(),small_string_data_cast()->length());
            case json_type_tag::string_t:
                return string_view_type(string_data_cast()->data(),string_data_cast()->length());
            default:
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a string");
            }
        }

        bool operator==(const variant& rhs) const
        {
            if (this == &rhs)
            {
                return true;
            }

            switch (globals::t_by_t[(size_t)type_id()][(size_t)rhs.type_id()])
            {
            case tt::null_null:
                return true;
            case tt::bool_bool:
                return bool_data_cast()->val_ == rhs.bool_data_cast()->val_;
            case tt::integer_integer:
                return integer_data_cast()->val_ == rhs.integer_data_cast()->val_;
            case tt::integer_uinteger:
                return integer_data_cast()->val_ >= 0 ? static_cast<uint64_t>(integer_data_cast()->val_) == rhs.uinteger_data_cast()->val_ : false;
            case tt::integer_double:
                return static_cast<double>(integer_data_cast()->val_) == rhs.double_data_cast()->val_;
            case tt::uinteger_uinteger:
                return uinteger_data_cast()->val_ == rhs.uinteger_data_cast()->val_;
            case tt::uinteger_integer:
                return rhs.integer_data_cast()->val_ >= 0 ? uinteger_data_cast()->val_ == static_cast<uint64_t>(rhs.integer_data_cast()->val_) : false;
            case tt::uinteger_double:
                return static_cast<double>(uinteger_data_cast()->val_) == rhs.double_data_cast()->val_;
            case tt::double_double:
                return double_data_cast()->val_ == rhs.double_data_cast()->val_;
            case tt::double_integer:
                return double_data_cast()->val_ == static_cast<double>(rhs.integer_data_cast()->val_);
            case tt::double_uinteger:
                return double_data_cast()->val_ == static_cast<double>(rhs.uinteger_data_cast()->val_);
            case tt::smallstr_smallstr:
                return as_string_view() == rhs.as_string_view();
            case tt::smallstr_string:
                return as_string_view() == rhs.as_string_view();
            case tt::string_string:
                return as_string_view() == rhs.as_string_view();
            case tt::string_smallstr:
                return as_string_view() == rhs.as_string_view();
            case tt::array_array:
                return array_data_cast()->value() == rhs.array_data_cast()->value();
            case tt::emptyobj_emptyobj:
                return true;
            case tt::emptyobj_object:
                return rhs.object_data_cast()->ptr_->size() == 0;
            case tt::object_object:
                return object_data_cast()->value() == rhs.object_data_cast()->value();
            case tt::object_emptyobj:
                return object_data_cast()->ptr_->size() == 0;
            default:
                return false;
            }
        }

        bool operator!=(const variant& rhs) const
        {
            return !(*this == rhs);
        }

        template <class Alloc = allocator_type>
        typename std::enable_if<std::is_pod<typename std::allocator_traits<Alloc>::pointer>::value,void>::type
        swap(variant& rhs) JSONCONS_NOEXCEPT
        {
            if (this == &rhs)
            {
                return;
            }

            std::swap(data_,rhs.data_);
        }

        template <class Alloc = allocator_type>
        typename std::enable_if<!std::is_pod<typename std::allocator_traits<Alloc>::pointer>::value, void>::type
        swap(variant& rhs) JSONCONS_NOEXCEPT
        {
            if (this == &rhs)
            {
                return;
            }

            switch (globals::t_by_t[(size_t)type_id()][(size_t)rhs.type_id()])
            {
            // null_
            case tt::null_string:
                {
                    auto ptr = rhs.string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))null_data();
                    new(reinterpret_cast<void*>(&data_))string_data(ptr);
                    break;
                }
            case tt::null_array:
                {
                    auto ptr = rhs.array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))null_data();
                    new(reinterpret_cast<void*>(&data_))array_data(ptr);
                    break;
                }
            case tt::null_object:
                {
                    auto ptr = rhs.object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))null_data();
                    new(reinterpret_cast<void*>(&data_))object_data(ptr);
                    break;
                }
            // bool_
            case tt::bool_string:
                {
                    auto ptr = rhs.string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))bool_data(*bool_data_cast());
                    new(reinterpret_cast<void*>(&data_))string_data(ptr);
                    break;
                }
            case tt::bool_array:
                {
                    auto ptr = rhs.array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))bool_data(*bool_data_cast());
                    new(reinterpret_cast<void*>(&data_))array_data(ptr);
                    break;
                }
            case tt::bool_object:
                {
                    auto ptr = rhs.object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))bool_data(*bool_data_cast());
                    new(reinterpret_cast<void*>(&data_))object_data(ptr);
                    break;
                }
            // integer_
            case tt::integer_string:
                {
                    auto ptr = rhs.string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))integer_data(*integer_data_cast());
                    new(reinterpret_cast<void*>(&data_))string_data(ptr);
                    break;
                }
            case tt::integer_array:
                {
                    auto ptr = rhs.array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))integer_data(*integer_data_cast());
                    new(reinterpret_cast<void*>(&data_))array_data(ptr);
                    break;
                }
            case tt::integer_object:
                {
                    auto ptr = rhs.object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))integer_data(*integer_data_cast());
                    new(reinterpret_cast<void*>(&data_))object_data(ptr);
                    break;
                }
            // uinteger_
            case tt::uinteger_string:
                {
                    auto ptr = rhs.string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))uinteger_data(*uinteger_data_cast());
                    new(reinterpret_cast<void*>(&data_))string_data(ptr);
                    break;
                }
            case tt::uinteger_array:
                {
                    auto ptr = rhs.array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))uinteger_data(*uinteger_data_cast());
                    new(reinterpret_cast<void*>(&data_))array_data(ptr);
                    break;
                }
            case tt::uinteger_object:
                {
                    auto ptr = rhs.object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))uinteger_data(*uinteger_data_cast());
                    new(reinterpret_cast<void*>(&data_))object_data(ptr);
                    break;
                }
            // double_
            case tt::double_string:
                {
                    auto ptr = rhs.string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))double_data(*double_data_cast());
                    new(reinterpret_cast<void*>(&data_))string_data(ptr);
                    break;
                }
            case tt::double_array:
                {
                    auto ptr = rhs.array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))double_data(*double_data_cast());
                    new(reinterpret_cast<void*>(&data_))array_data(ptr);
                    break;
                }
            case tt::double_object:
                {
                    auto ptr = rhs.object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))double_data(*double_data_cast());
                    new(reinterpret_cast<void*>(&data_))object_data(ptr);
                    break;
                }
            // smallstr_
            case tt::smallstr_string:
                {
                    auto ptr = rhs.string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))small_string_data(*small_string_data_cast());
                    new(reinterpret_cast<void*>(&data_))string_data(ptr);
                    break;
                }
            case tt::smallstr_array:
                {
                    auto ptr = rhs.array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))small_string_data(*small_string_data_cast());
                    new(reinterpret_cast<void*>(&data_))array_data(ptr);
                    break;
                }
            case tt::smallstr_object:
                {
                    auto ptr = rhs.object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))small_string_data(*small_string_data_cast());
                    new(reinterpret_cast<void*>(&data_))object_data(ptr);
                    break;
                }
            // string_
            case tt::string_null:
                {
                    auto ptr = string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))null_data();
                    new(reinterpret_cast<void*>(&(rhs.data_)))string_data(ptr);
                    break;
                }
            case tt::string_bool:
                {
                    auto ptr = string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))bool_data(*(rhs.bool_data_cast()));
                    new(reinterpret_cast<void*>(&(rhs.data_)))string_data(ptr);
                    break;
                }
            case tt::string_integer:
                {
                    auto ptr = string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))integer_data(*(rhs.integer_data_cast()));
                    new(reinterpret_cast<void*>(&(rhs.data_)))string_data(ptr);
                    break;
                }
            case tt::string_uinteger:
                {
                    auto ptr = string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))uinteger_data(*(rhs.uinteger_data_cast()));
                    new(reinterpret_cast<void*>(&(rhs.data_)))string_data(ptr);
                    break;
                }
            case tt::string_double:
                {
                    auto ptr = string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))double_data(*(rhs.double_data_cast()));
                    new(reinterpret_cast<void*>(&(rhs.data_)))string_data(ptr);
                    break;
                }
            case tt::string_smallstr:
                {
                    auto ptr = string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))small_string_data(*(rhs.small_string_data_cast()));
                    new(reinterpret_cast<void*>(&(rhs.data_)))string_data(ptr);
                    break;
                }
            case tt::string_string:
                {
                    auto ptr = string_data_cast()->ptr_;
                    string_data_cast()->ptr_ = rhs.string_data_cast()->ptr_;
                    rhs.string_data_cast()->ptr_ = ptr;
                    break;
                }
            case tt::string_array:
                {
                    auto ptr = string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))array_data(rhs.array_data_cast()->ptr_);
                    new(reinterpret_cast<void*>(&(rhs.data_)))string_data(ptr);
                    break;
                }
            case tt::string_emptyobj:
                {
                    auto ptr = string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))empty_object_data();
                    new(reinterpret_cast<void*>(&(rhs.data_)))string_data(ptr);
                    break;
                }
            case tt::string_object:
                {
                    auto ptr = string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))object_data(rhs.object_data_cast()->ptr_);
                    new(reinterpret_cast<void*>(&(rhs.data_)))string_data(ptr);
                    break;
                }
            // array_
            case tt::array_null:
                {
                    auto ptr = array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))null_data();
                    new(reinterpret_cast<void*>(&(rhs.data_)))array_data(ptr);
                    break;
                }
            case tt::array_bool:
                {
                    auto ptr = array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))bool_data(*(rhs.bool_data_cast()));
                    new(reinterpret_cast<void*>(&(rhs.data_)))array_data(ptr);
                    break;
                }
            case tt::array_integer:
                {
                    auto ptr = array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))integer_data(*(rhs.integer_data_cast()));
                    new(reinterpret_cast<void*>(&(rhs.data_)))array_data(ptr);
                    break;
                }
            case tt::array_uinteger:
                {
                    auto ptr = array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))uinteger_data(*(rhs.uinteger_data_cast()));
                    new(reinterpret_cast<void*>(&(rhs.data_)))array_data(ptr);
                    break;
                }
            case tt::array_double:
                {
                    auto ptr = array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))double_data(*(rhs.double_data_cast()));
                    new(reinterpret_cast<void*>(&(rhs.data_)))array_data(ptr);
                    break;
                }
            case tt::array_smallstr:
                {
                    auto ptr = array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))small_string_data(*(rhs.small_string_data_cast()));
                    new(reinterpret_cast<void*>(&(rhs.data_)))array_data(ptr);
                    break;
                }
            case tt::array_string:
                {
                    auto ptr = array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))string_data(rhs.string_data_cast()->ptr_);
                    new(reinterpret_cast<void*>(&(rhs.data_)))array_data(ptr);
                    break;
                }
            case tt::array_array:
                {
                    auto ptr = array_data_cast()->ptr_;
                    array_data_cast()->ptr_ = rhs.array_data_cast()->ptr_;
                    rhs.array_data_cast()->ptr_ = ptr;
                    break;
                }
            case tt::array_emptyobj:
                {
                    auto ptr = array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))empty_object_data();
                    new(reinterpret_cast<void*>(&(rhs.data_)))array_data(ptr);
                    break;
                }
            case tt::array_object:
                {
                    auto ptr = array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))object_data(rhs.object_data_cast()->ptr_);
                    new(reinterpret_cast<void*>(&(rhs.data_)))array_data(ptr);
                    break;
                }
            // emptyobj_
            case tt::emptyobj_string:
                {
                    auto ptr = rhs.string_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))empty_object_data();
                    new(reinterpret_cast<void*>(&data_))string_data(ptr);
                    break;
                }
            case tt::emptyobj_array:
                {
                    auto ptr = rhs.array_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))empty_object_data();
                    new(reinterpret_cast<void*>(&data_))array_data(ptr);
                    break;
                }
            case tt::emptyobj_object:
                {
                    auto ptr = rhs.object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&(rhs.data_)))empty_object_data();
                    new(reinterpret_cast<void*>(&data_))object_data(ptr);
                    break;
                }
            // object_
            case tt::object_null:
                {
                    auto ptr = object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))null_data();
                    new(reinterpret_cast<void*>(&(rhs.data_)))object_data(ptr);
                    break;
                }
            case tt::object_bool:
                {
                    auto ptr = object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))bool_data(*(rhs.bool_data_cast()));
                    new(reinterpret_cast<void*>(&(rhs.data_)))object_data(ptr);
                    break;
                }
            case tt::object_integer:
                {
                    auto ptr = object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))integer_data(*(rhs.integer_data_cast()));
                    new(reinterpret_cast<void*>(&(rhs.data_)))object_data(ptr);
                    break;
                }
            case tt::object_uinteger:
                {
                    auto ptr = object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))uinteger_data(*(rhs.uinteger_data_cast()));
                    new(reinterpret_cast<void*>(&(rhs.data_)))object_data(ptr);
                    break;
                }
            case tt::object_double:
                {
                    auto ptr = object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))double_data(*(rhs.double_data_cast()));
                    new(reinterpret_cast<void*>(&(rhs.data_)))object_data(ptr);
                    break;
                }
            case tt::object_smallstr:
                {
                    auto ptr = object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))small_string_data(*(rhs.small_string_data_cast()));
                    new(reinterpret_cast<void*>(&(rhs.data_)))object_data(ptr);
                    break;
                }
            case tt::object_string:
                {
                    auto ptr = object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))string_data(rhs.string_data_cast()->ptr_);
                    new(reinterpret_cast<void*>(&(rhs.data_)))object_data(ptr);
                    break;
                }
            case tt::object_array:
                {
                    auto ptr = object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))array_data(rhs.array_data_cast()->ptr_);
                    new(reinterpret_cast<void*>(&(rhs.data_)))object_data(ptr);
                    break;
                }
            case tt::object_emptyobj:
                {
                    auto ptr = object_data_cast()->ptr_;
                    new(reinterpret_cast<void*>(&data_))empty_object_data();
                    new(reinterpret_cast<void*>(&(rhs.data_)))object_data(ptr);
                    break;
                }
            case tt::object_object:
                {
                    auto ptr = object_data_cast()->ptr_;
                    object_data_cast()->ptr_ = rhs.object_data_cast()->ptr_;
                    rhs.object_data_cast()->ptr_ = ptr;
                    break;
                }
            default:
                std::swap(data_,rhs.data_);
                break;
            }          
        }
    private:

        void Init_(const variant& val)
        {
            switch (val.type_id())
            {
            case json_type_tag::null_t:
                new(reinterpret_cast<void*>(&data_))null_data();
                break;
            case json_type_tag::empty_object_t:
                new(reinterpret_cast<void*>(&data_))empty_object_data();
                break;
            case json_type_tag::double_t:
                new(reinterpret_cast<void*>(&data_))double_data(*(val.double_data_cast()));
                break;
            case json_type_tag::integer_t:
                new(reinterpret_cast<void*>(&data_))integer_data(*(val.integer_data_cast()));
                break;
            case json_type_tag::uinteger_t:
                new(reinterpret_cast<void*>(&data_))uinteger_data(*(val.uinteger_data_cast()));
                break;
            case json_type_tag::bool_t:
                new(reinterpret_cast<void*>(&data_))bool_data(*(val.bool_data_cast()));
                break;
            case json_type_tag::small_string_t:
                new(reinterpret_cast<void*>(&data_))small_string_data(*(val.small_string_data_cast()));
                break;
            case json_type_tag::string_t:
                new(reinterpret_cast<void*>(&data_))string_data(*(val.string_data_cast()));
                break;
            case json_type_tag::object_t:
                new(reinterpret_cast<void*>(&data_))object_data(*(val.object_data_cast()));
                break;
            case json_type_tag::array_t:
                new(reinterpret_cast<void*>(&data_))array_data(*(val.array_data_cast()));
                break;
            default:
                break;
            }
        }

        void Init_(const variant& val, const Allocator& a)
        {
            switch (val.type_id())
            {
            case json_type_tag::null_t:
            case json_type_tag::empty_object_t:
            case json_type_tag::double_t:
            case json_type_tag::integer_t:
            case json_type_tag::uinteger_t:
            case json_type_tag::bool_t:
            case json_type_tag::small_string_t:
                Init_(val);
                break;
            case json_type_tag::string_t:
                new(reinterpret_cast<void*>(&data_))string_data(*(val.string_data_cast()),a);
                break;
            case json_type_tag::object_t:
                new(reinterpret_cast<void*>(&data_))object_data(*(val.object_data_cast()),a);
                break;
            case json_type_tag::array_t:
                new(reinterpret_cast<void*>(&data_))array_data(*(val.array_data_cast()),a);
                break;
            default:
                break;
            }
        }

        void Init_rv_(variant&& val) JSONCONS_NOEXCEPT
        {
            switch (val.type_id())
            {
            case json_type_tag::null_t:
            case json_type_tag::empty_object_t:
            case json_type_tag::double_t:
            case json_type_tag::integer_t:
            case json_type_tag::uinteger_t:
            case json_type_tag::bool_t:
            case json_type_tag::small_string_t:
                Init_(val);
                break;
            case json_type_tag::string_t:
                {
                    new(reinterpret_cast<void*>(&data_))string_data(val.string_data_cast()->ptr_);
                    val.string_data_cast()->type_id_ = json_type_tag::null_t;
                }
                break;
            case json_type_tag::object_t:
                {
                    new(reinterpret_cast<void*>(&data_))object_data(val.object_data_cast()->ptr_);
                    val.object_data_cast()->type_id_ = json_type_tag::null_t;
                }
                break;
            case json_type_tag::array_t:
                {
                    new(reinterpret_cast<void*>(&data_))array_data(val.array_data_cast()->ptr_);
                    val.array_data_cast()->type_id_ = json_type_tag::null_t;
                }
                break;
            default:
                break;
            }
        }

        void Init_rv_(variant&& val, const Allocator& a, std::true_type) JSONCONS_NOEXCEPT
        {
            Init_rv_(std::forward<variant>(val));
        }

        void Init_rv_(variant&& val, const Allocator& a, std::false_type) JSONCONS_NOEXCEPT
        {
            switch (val.type_id())
            {
            case json_type_tag::null_t:
            case json_type_tag::empty_object_t:
            case json_type_tag::double_t:
            case json_type_tag::integer_t:
            case json_type_tag::uinteger_t:
            case json_type_tag::bool_t:
            case json_type_tag::small_string_t:
                Init_(std::forward<variant>(val));
                break;
            case json_type_tag::string_t:
                {
                    if (a == val.string_data_cast()->get_allocator())
                    {
                        Init_rv_(std::forward<variant>(val), a, std::true_type());
                    }
                    else
                    {
                        Init_(val,a);
                    }
                }
                break;
            case json_type_tag::object_t:
                {
                    if (a == val.object_data_cast()->get_allocator())
                    {
                        Init_rv_(std::forward<variant>(val), a, std::true_type());
                    }
                    else
                    {
                        Init_(val,a);
                    }
                }
                break;
            case json_type_tag::array_t:
                {
                    if (a == val.array_data_cast()->get_allocator())
                    {
                        Init_rv_(std::forward<variant>(val), a, std::true_type());
                    }
                    else
                    {
                        Init_(val,a);
                    }
                }
                break;
            default:
                break;
            }
        }
    };

    template <class ParentT>
    class json_proxy 
    {
    private:
        typedef json_proxy<ParentT> proxy_type;

        ParentT& parent_;
        key_storage_type key_;

        json_proxy() = delete;
        json_proxy& operator = (const json_proxy& other) = delete; 

        json_proxy(ParentT& parent, key_storage_type&& name)
            : parent_(parent), key_(std::forward<key_storage_type>(name))
        {
        }

        basic_json& evaluate() 
        {
            return parent_.evaluate(string_view_type(key_.data(),key_.size()));
        }

        const basic_json& evaluate() const
        {
            return parent_.evaluate(string_view_type(key_.data(),key_.size()));
        }

        basic_json& evaluate_with_default()
        {
            basic_json& val = parent_.evaluate_with_default();
            auto it = val.find(string_view_type(key_.data(),key_.size()));
            if (it == val.object_range().end())
            {
                it = val.set_(val.object_range().begin(),std::move(key_),object(val.object_value().get_allocator()));            
            }
            return it->value();
        }

        basic_json& evaluate(size_t index)
        {
            return evaluate().at(index);
        }

        const basic_json& evaluate(size_t index) const
        {
            return evaluate().at(index);
        }

        basic_json& evaluate(string_view_type index)
        {
            return evaluate().at(index);
        }

        const basic_json& evaluate(string_view_type index) const
        {
            return evaluate().at(index);
        }
    public:

        friend class basic_json<CharT,JsonTraits,Allocator>;

        range<object_iterator> object_range()
        {
            return evaluate().object_range();
        }

        range<const_object_iterator> object_range() const
        {
            return evaluate().object_range();
        }

        range<array_iterator> array_range()
        {
            return evaluate().array_range();
        }

        range<const_array_iterator> array_range() const
        {
            return evaluate().array_range();
        }

        size_t size() const JSONCONS_NOEXCEPT
        {
            return evaluate().size();
        }

        json_type_tag type_id() const
        {
            return evaluate().type_id();
        }

        size_t count(string_view_type name) const
        {
            return evaluate().count(name);
        }

        bool has_key(string_view_type name) const
        {
            return evaluate().has_key(name);
        }

        bool is_null() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_null();
        }

        bool empty() const
        {
            return evaluate().empty();
        }

        size_t capacity() const
        {
            return evaluate().capacity();
        }

        void reserve(size_t n)
        {
            evaluate().reserve(n);
        }

        void resize(size_t n)
        {
            evaluate().resize(n);
        }

        template <class T>
        void resize(size_t n, T val)
        {
            evaluate().resize(n,val);
        }

        template<class T, class... Args>
        bool is(Args&&... args) const
        {
            return evaluate().template is<T>(std::forward<Args>(args)...);
        }

        bool is_string() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_string();
        }

        bool is_number() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_number();
        }
        bool is_bool() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_bool();
        }

        bool is_object() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_object();
        }

        bool is_array() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_array();
        }
        bool is_integer() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_integer();
        }

        bool is_uinteger() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_uinteger();
        }

        bool is_double() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_double();
        }

        string_view_type as_string_view() const 
        {
            return evaluate().as_string_view();
        }

        string_type as_string() const JSONCONS_NOEXCEPT
        {
            return evaluate().as_string();
        }

        template <class SAllocator>
        string_type as_string(const SAllocator& allocator) const JSONCONS_NOEXCEPT
        {
            return evaluate().as_string(allocator);
        }

        string_type as_string(const basic_serialization_options<char_type>& options) const
        {
            return evaluate().as_string(options);
        }

        template <class SAllocator>
        string_type as_string(const basic_serialization_options<char_type>& options,
                              const SAllocator& allocator) const
        {
            return evaluate().as_string(options,allocator);
        }

        template<class T, class... Args>
        T as(Args&&... args) const
        {
            return evaluate().template as<T>(std::forward<Args>(args)...);
        }

        template<class T>
        typename std::enable_if<std::is_same<string_type,T>::value,T>::type 
        as(const char_allocator_type& allocator) const
        {
            return evaluate().template as<T>(allocator);
        }
        bool as_bool() const
        {
            return evaluate().as_bool();
        }

        double as_double() const
        {
            return evaluate().as_double();
        }

        int64_t as_integer() const
        {
            return evaluate().as_integer();
        }

        unsigned long long as_ulonglong() const
        {
            return evaluate().as_ulonglong();
        }

        uint64_t as_uinteger() const
        {
            return evaluate().as_uinteger();
        }

        template <class T>
        json_proxy& operator=(T&& val) 
        {
            parent_.evaluate_with_default().set_(std::move(key_), std::forward<T>(val));
            return *this;
        }

        bool operator==(const basic_json& val) const
        {
            return evaluate() == val;
        }

        bool operator!=(const basic_json& val) const
        {
            return evaluate() != val;
        }

        basic_json& operator[](size_t i)
        {
            return evaluate_with_default().at(i);
        }

        const basic_json& operator[](size_t i) const
        {
            return evaluate().at(i);
        }

        json_proxy<proxy_type> operator[](string_view_type name)
        {
            return json_proxy<proxy_type>(*this,key_storage_type(name.begin(),name.end(),key_.get_allocator()));
        }

        const basic_json& operator[](string_view_type name) const
        {
            return at(name);
        }

        basic_json& at(string_view_type name)
        {
            return evaluate().at(name);
        }

        const basic_json& at(string_view_type name) const
        {
            return evaluate().at(name);
        }

        const basic_json& at(size_t index)
        {
            return evaluate().at(index);
        }

        const basic_json& at(size_t index) const
        {
            return evaluate().at(index);
        }

        object_iterator find(string_view_type name)
        {
            return evaluate().find(name);
        }

        const_object_iterator find(string_view_type name) const
        {
            return evaluate().find(name);
        }

        template <class T>
        basic_json get(string_view_type name, T&& default_val) const
        {
            return evaluate().get(name,std::forward<T>(default_val));
        }

        template <class T>
        T get_with_default(string_view_type name, const T& default_val) const
        {
            return evaluate().get_with_default(name,default_val);
        }

        const CharT* get_with_default(string_view_type name, const CharT* default_val) const
        {
            return evaluate().get_with_default(name,default_val);
        }

        void shrink_to_fit()
        {
            evaluate_with_default().shrink_to_fit();
        }

        void clear()
        {
            evaluate().clear();
        }
        // Remove all elements from an array or object

        void erase(const_object_iterator pos)
        {
            evaluate().erase(pos);
        }
        // Remove a range of elements from an object 

        void erase(const_object_iterator first, const_object_iterator last)
        {
            evaluate().erase(first, last);
        }
        // Remove a range of elements from an object 

        void erase(string_view_type name)
        {
            evaluate().erase(name);
        }

        void erase(const_array_iterator pos)
        {
            evaluate().erase(pos);
        }
        // Removes the element at pos 

        void erase(const_array_iterator first, const_array_iterator last)
        {
            evaluate().erase(first, last);
        }
        // Remove a range of elements from an array 

        // merge

        void merge(const basic_json& source)
        {
            return evaluate().merge(source);
        }

        void merge(basic_json&& source)
        {
            return evaluate().merge(std::forward<basic_json>(source));
        }

        void merge(object_iterator hint, const basic_json& source)
        {
            return evaluate().merge(hint, source);
        }

        void merge(object_iterator hint, basic_json&& source)
        {
            return evaluate().merge(hint, std::forward<basic_json>(source));
        }

        // merge_or_update

        void merge_or_update(const basic_json& source)
        {
            return evaluate().merge_or_update(source);
        }

        void merge_or_update(basic_json&& source)
        {
            return evaluate().merge_or_update(std::forward<basic_json>(source));
        }

        void merge_or_update(object_iterator hint, const basic_json& source)
        {
            return evaluate().merge_or_update(hint, source);
        }

        void merge_or_update(object_iterator hint, basic_json&& source)
        {
            return evaluate().merge_or_update(hint, std::forward<basic_json>(source));
        }

       // set

        template <class T>
        std::pair<object_iterator,bool> set(string_view_type name, T&& val)
        {
            return evaluate().set(name,std::forward<T>(val));
        }

        template <class T>
        std::pair<object_iterator,bool> insert_or_assign(string_view_type name, T&& val)
        {
            return evaluate().insert_or_assign(name,std::forward<T>(val));
        }

        template <class T>
        void set_(key_storage_type&& name, T&& val)
        {
            evaluate().set_(std::forward<key_storage_type>(name),std::forward<T>(val));
        }

       // emplace

        template <class ... Args>
        std::pair<object_iterator,bool> try_emplace(string_view_type name, Args&&... args)
        {
            return evaluate().try_emplace(name,std::forward<Args>(args)...);
        }

        template <class T>
        object_iterator set(object_iterator hint, string_view_type name, T&& val)
        {
            return evaluate().set(hint, name, std::forward<T>(val));
        }

        template <class T>
        object_iterator insert_or_assign(object_iterator hint, string_view_type name, T&& val)
        {
            return evaluate().insert_or_assign(hint, name, std::forward<T>(val));
        }

        template <class ... Args>
        object_iterator try_emplace(object_iterator hint, string_view_type name, Args&&... args)
        {
            return evaluate().try_emplace(hint, name, std::forward<Args>(args)...);
        }

        template <class T>
        object_iterator set_(object_iterator hint, key_storage_type&& name, T&& val)
        {
            return evaluate().set_(hint, std::forward<key_storage_type>(name), std::forward<T>(val));
        }

        template <class... Args> 
        array_iterator emplace(const_array_iterator pos, Args&&... args)
        {
            evaluate_with_default().emplace(pos, std::forward<Args>(args)...);
        }

        template <class... Args> 
        basic_json& emplace_back(Args&&... args)
        {
            return evaluate_with_default().emplace_back(std::forward<Args>(args)...);
        }

        template <class T>
        void add(T&& val)
        {
            evaluate_with_default().add(std::forward<T>(val));
        }

        template <class T>
        void push_back(T&& val)
        {
            evaluate_with_default().push_back(std::forward<T>(val));
        }

        template <class T>
        array_iterator add(const_array_iterator pos, T&& val)
        {
            return evaluate_with_default().add(pos, std::forward<T>(val));
        }

        template <class T>
        array_iterator insert(const_array_iterator pos, T&& val)
        {
            return evaluate_with_default().insert(pos, std::forward<T>(val));
        }

        template <class InputIt>
        array_iterator insert(const_array_iterator pos, InputIt first, InputIt last)
        {
            return evaluate_with_default().insert(pos, first, last);
        }

        template <class SAllocator>
        void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s) const
        {
            evaluate().dump(s);
        }

        template <class SAllocator>
        void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s,
                  const basic_serialization_options<char_type>& options) const
        {
            evaluate().dump(s,options);
        }
        void dump(basic_json_output_handler<char_type>& handler) const
        {
            evaluate().dump(handler);
        }

        void dump(std::basic_ostream<char_type>& os) const
        {
            evaluate().dump(os);
        }

        void dump(std::basic_ostream<char_type>& os, bool pprint) const
        {
            evaluate().dump(os, pprint);
        }

        void dump(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options) const
        {
            evaluate().dump(os,options);
        }

        void dump(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool pprint) const
        {
            evaluate().dump(os,options,pprint);
        }
#if !defined(JSONCONS_NO_DEPRECATED)

        string_type to_string(const char_allocator_type& allocator = char_allocator_type()) const JSONCONS_NOEXCEPT
        {
            return evaluate().to_string(allocator);
        }
        void write(basic_json_output_handler<char_type>& handler) const
        {
            evaluate().write(handler);
        }

        void write(std::basic_ostream<char_type>& os) const
        {
            evaluate().write(os);
        }

        void write(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options) const
        {
            evaluate().write(os,options);
        }

        void write(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool pprint) const
        {
            evaluate().write(os,options,pprint);
        }

        string_type to_string(const basic_serialization_options<char_type>& options, char_allocator_type& allocator = char_allocator_type()) const
        {
            return evaluate().to_string(options,allocator);
        }

        range<object_iterator> members()
        {
            return evaluate().members();
        }

        range<const_object_iterator> members() const
        {
            return evaluate().members();
        }

        range<array_iterator> elements()
        {
            return evaluate().elements();
        }

        range<const_array_iterator> elements() const
        {
            return evaluate().elements();
        }
        void to_stream(basic_json_output_handler<char_type>& handler) const
        {
            evaluate().to_stream(handler);
        }

        void to_stream(std::basic_ostream<char_type>& os) const
        {
            evaluate().to_stream(os);
        }

        void to_stream(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options) const
        {
            evaluate().to_stream(os,options);
        }

        void to_stream(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool pprint) const
        {
            evaluate().to_stream(os,options,pprint);
        }
#endif
        void swap(basic_json& val)
        {
            evaluate_with_default().swap(val);
        }

        friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const json_proxy& o)
        {
            o.dump(os);
            return os;
        }

#if !defined(JSONCONS_NO_DEPRECATED)

        void resize_array(size_t n)
        {
            evaluate().resize_array(n);
        }

        template <class T>
        void resize_array(size_t n, T val)
        {
            evaluate().resize_array(n,val);
        }

        object_iterator begin_members()
        {
            return evaluate().begin_members();
        }

        const_object_iterator begin_members() const
        {
            return evaluate().begin_members();
        }

        object_iterator end_members()
        {
            return evaluate().end_members();
        }

        const_object_iterator end_members() const
        {
            return evaluate().end_members();
        }

        array_iterator begin_elements()
        {
            return evaluate().begin_elements();
        }

        const_array_iterator begin_elements() const
        {
            return evaluate().begin_elements();
        }

        array_iterator end_elements()
        {
            return evaluate().end_elements();
        }

        const_array_iterator end_elements() const
        {
            return evaluate().end_elements();
        }

        const basic_json& get(string_view_type name) const
        {
            return evaluate().get(name);
        }

        bool is_ulonglong() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_ulonglong();
        }

        bool is_longlong() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_longlong();
        }

        int as_int() const
        {
            return evaluate().as_int();
        }

        unsigned int as_uint() const
        {
            return evaluate().as_uint();
        }

        long as_long() const
        {
            return evaluate().as_long();
        }

        unsigned long as_ulong() const
        {
            return evaluate().as_ulong();
        }

        long long as_longlong() const
        {
            return evaluate().as_longlong();
        }

        void add(size_t index, const basic_json& value)
        {
            evaluate_with_default().add(index, value);
        }

        void add(size_t index, basic_json&& value)
        {
            evaluate_with_default().add(index, std::forward<basic_json>(value));
        }

        bool has_member(const key_storage_type& name) const
        {
            return evaluate().has_member(name);
        }

        // Remove a range of elements from an array 
        void remove_range(size_t from_index, size_t to_index)
        {
            evaluate().remove_range(from_index, to_index);
        }
        // Remove a range of elements from an array 
        void remove(string_view_type name)
        {
            evaluate().remove(name);
        }
        void remove_member(string_view_type name)
        {
            evaluate().remove(name);
        }
        bool is_empty() const JSONCONS_NOEXCEPT
        {
            return empty();
        }
        bool is_numeric() const JSONCONS_NOEXCEPT
        {
            return is_number();
        }
#endif
    };

    static basic_json parse(std::basic_istream<char_type>& is);
    static basic_json parse(std::basic_istream<char_type>& is, parse_error_handler& err_handler);

    static basic_json parse(string_view_type s)
    {
        parse_error_handler_type err_handler;
        return parse(s,err_handler);
    }

    static basic_json parse(const char_type* s, size_t length)
    {
        parse_error_handler_type err_handler;
        return parse(s,length,err_handler);
    }

    static basic_json parse(const char_type* s, size_t length, parse_error_handler& err_handler)
    {
        return parse(string_view_type(s,length),err_handler);
    }

    static basic_json parse(string_view_type s, parse_error_handler& err_handler)
    {
        json_decoder<basic_json> handler;
        basic_json_parser<char_type> parser(handler,err_handler);

        auto result = unicons::skip_bom(s.begin(), s.end());
        if (result.ec != unicons::encoding_errc())
        {
            throw parse_error(result.ec,1,1);
        }
        size_t offset = result.it - s.begin();
        parser.set_source(s.data()+offset,s.size()-offset);
        parser.parse();
        parser.end_parse();
        parser.check_done();
        if (!handler.is_valid())
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json string");
        }
        return handler.get_result();
    }

    static basic_json parse_file(const std::basic_string<char_type,char_traits_type>& filename)
    {
        parse_error_handler_type err_handler;
        return parse_file(filename,err_handler);
    }

    static basic_json parse_file(const std::basic_string<char_type,char_traits_type>& filename,
                                 parse_error_handler& err_handler)
    {
        std::basic_ifstream<CharT> is(filename);
        return parse(is,err_handler);
    }

    static basic_json make_array()
    {
        return basic_json(variant(array()));
    }

    static basic_json make_array(const array& a)
    {
        return basic_json(variant(a));
    }

    static basic_json make_array(const array& a, allocator_type allocator)
    {
        return basic_json(variant(a,allocator));
    }

    static basic_json make_array(std::initializer_list<basic_json> init, const Allocator& allocator = Allocator())
    {
        return array(std::move(init),allocator);
    }

    static basic_json make_array(size_t n, const Allocator& allocator = Allocator())
    {
        return array(n,allocator);
    }

    template <class T>
    static basic_json make_array(size_t n, const T& val, const Allocator& allocator = Allocator())
    {
        return basic_json::array(n, val,allocator);
    }

    template <size_t dim>
    static typename std::enable_if<dim==1,basic_json>::type make_array(size_t n)
    {
        return array(n);
    }

    template <size_t dim, class T>
    static typename std::enable_if<dim==1,basic_json>::type make_array(size_t n, const T& val, const Allocator& allocator = Allocator())
    {
        return array(n,val,allocator);
    }

    template <size_t dim, typename... Args>
    static typename std::enable_if<(dim>1),basic_json>::type make_array(size_t n, Args... args)
    {
        const size_t dim1 = dim - 1;

        basic_json val = make_array<dim1>(args...);
        val.resize(n);
        for (size_t i = 0; i < n; ++i)
        {
            val[i] = make_array<dim1>(args...);
        }
        return val;
    }

    static const basic_json& null()
    {
        static basic_json a_null = basic_json(variant(null_type()));
        return a_null;
    }

    variant var_;

    basic_json() 
        : var_()
    {
    }

    explicit basic_json(const Allocator& allocator) 
        : var_(allocator)
    {
    }

    basic_json(const basic_json& val)
        : var_(val.var_)
    {
    }

    basic_json(const basic_json& val, const Allocator& allocator)
        : var_(val.var_,allocator)
    {
    }

    basic_json(basic_json&& other) JSONCONS_NOEXCEPT
        : var_(std::move(other.var_))
    {
    }

    basic_json(basic_json&& other, const Allocator& allocator) JSONCONS_NOEXCEPT
        : var_(std::move(other.var_) /*,allocator*/ )
    {
    }

    basic_json(const variant& val)
        : var_(val)
    {
    }

    basic_json(variant&& other)
        : var_(std::forward<variant>(other))
    {
    }

    basic_json(const array& val)
        : var_(val)
    {
    }

    basic_json(array&& other)
        : var_(std::forward<array>(other))
    {
    }

    basic_json(const object& other)
        : var_(other)
    {
    }

    basic_json(object&& other)
        : var_(std::forward<object>(other))
    {
    }

    template <class ParentT>
    basic_json(const json_proxy<ParentT>& proxy)
        : var_(proxy.evaluate().var_)
    {
    }

    template <class ParentT>
    basic_json(const json_proxy<ParentT>& proxy, const Allocator& allocator)
        : var_(proxy.evaluate().var_,allocator)
    {
    }

    template <class T>
    basic_json(const T& val)
        : var_(json_type_traits<basic_json,T>::to_json(val).var_)
    {
    }

    template <class T>
    basic_json(const T& val, const Allocator& allocator)
        : var_(json_type_traits<basic_json,T>::to_json(val,allocator).var_)
    {
    }

    basic_json(const char_type* s)
        : var_(s)
    {
    }

    basic_json(const char_type* s, const Allocator& allocator)
        : var_(s,allocator)
    {
    }

    basic_json(double val, uint8_t precision)
        : var_(val,precision)
    {
    }

    basic_json(const char_type *s, size_t length)
        : var_(s, length)
    {
    }

    basic_json(const char_type *s, size_t length, const Allocator& allocator)
        : var_(s, length, allocator)
    {
    }
#if !defined(JSONCONS_NO_DEPRECATED)
    template<class InputIterator>
    basic_json(InputIterator first, InputIterator last, const Allocator& allocator = Allocator())
        : var_(first,last,allocator)
    {
    }
#endif

    ~basic_json()
    {
    }

    basic_json& operator=(const basic_json& rhs)
    {
        var_ = rhs.var_;
        return *this;
    }

    basic_json& operator=(basic_json&& rhs) JSONCONS_NOEXCEPT
    {
        if (this != &rhs)
        {
            var_ = std::move(rhs.var_);
        }
        return *this;
    }

    template <class T>
    basic_json& operator=(const T& val)
    {
        var_ = json_type_traits<basic_json,T>::to_json(val).var_;
        return *this;
    }

    basic_json& operator=(const char_type* s)
    {
        var_ = variant(s);
        return *this;
    }

    bool operator!=(const basic_json& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator==(const basic_json& rhs) const
    {
        return var_ == rhs.var_;
    }

    size_t size() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            return 0;
        case json_type_tag::object_t:
            return object_value().size();
        case json_type_tag::array_t:
            return array_value().size();
        default:
            return 0;
        }
    }

    basic_json& operator[](size_t i)
    {
        return at(i);
    }

    const basic_json& operator[](size_t i) const
    {
        return at(i);
    }

    json_proxy<basic_json> operator[](string_view_type name)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t: 
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return json_proxy<basic_json>(*this, key_storage_type(name.begin(),name.end(),char_allocator_type(object_value().get_allocator())));
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
            break;
        }
    }

    const basic_json& operator[](string_view_type name) const
    {
        return at(name);
    }

    template <class SAllocator>
    void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s) const
    {
        std::basic_ostringstream<char_type,char_traits_type,SAllocator> os;
        os.imbue(std::locale::classic());
        {
            basic_json_serializer<char_type> serializer(os);
            dump(serializer);
        }
        s = os.str();
    }

    template <class SAllocator>
    void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s,
              const basic_serialization_options<char_type>& options) const
    {
        std::basic_ostringstream<char_type,char_traits_type,SAllocator> os;
        os.imbue(std::locale::classic());
        {
            basic_json_serializer<char_type> serializer(os,options);
            dump(serializer);
        }
        s = os.str();
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    void dump_body(basic_json_output_handler<char_type>& handler) const
    {
        dump_fragment(handler);
    }
#endif
    void dump_fragment(basic_json_output_handler<char_type>& handler) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            handler.string_value(as_string_view());
            break;
        case json_type_tag::double_t:
            handler.double_value(var_.double_data_cast()->value(), var_.double_data_cast()->precision());
            break;
        case json_type_tag::integer_t:
            handler.integer_value(var_.integer_data_cast()->value());
            break;
        case json_type_tag::uinteger_t:
            handler.uinteger_value(var_.uinteger_data_cast()->value());
            break;
        case json_type_tag::bool_t:
            handler.bool_value(var_.bool_data_cast()->value());
            break;
        case json_type_tag::null_t:
            handler.null_value();
            break;
        case json_type_tag::empty_object_t:
            handler.begin_object();
            handler.end_object();
            break;
        case json_type_tag::object_t:
            {
                handler.begin_object();
                const object& o = object_value();
                for (const_object_iterator it = o.begin(); it != o.end(); ++it)
                {
                    handler.name(string_view_type((it->key()).data(),it->key().length()));
                    it->value().dump_fragment(handler);
                }
                handler.end_object();
            }
            break;
        case json_type_tag::array_t:
            {
                handler.begin_array();
                const array& o = array_value();
                for (const_array_iterator it = o.begin(); it != o.end(); ++it)
                {
                    it->dump_fragment(handler);
                }
                handler.end_array();
            }
            break;
        default:
            break;
        }
    }
    void dump(basic_json_output_handler<char_type>& handler) const
    {
        handler.begin_json();
        dump_fragment(handler);
        handler.end_json();
    }

    void dump(std::basic_ostream<char_type>& os) const
    {
        basic_json_serializer<char_type> serializer(os);
        dump(serializer);
    }

    void dump(std::basic_ostream<char_type>& os, bool pprint) const
    {
        basic_json_serializer<char_type> serializer(os, pprint);
        dump(serializer);
    }

    void dump(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options) const
    {
        basic_json_serializer<char_type> serializer(os, options);
        dump(serializer);
    }

    void dump(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool pprint) const
    {
        basic_json_serializer<char_type> serializer(os, options, pprint);
        dump(serializer);
    }

    string_type to_string(const char_allocator_type& allocator=char_allocator_type()) const JSONCONS_NOEXCEPT
    {
        string_type s(allocator);
        std::basic_ostringstream<char_type,char_traits_type,char_allocator_type> os(s);
        os.imbue(std::locale::classic());
        {
            basic_json_serializer<char_type> serializer(os);
            dump_fragment(serializer);
        }
        return os.str();
    }

    string_type to_string(const basic_serialization_options<char_type>& options,
                          const char_allocator_type& allocator=char_allocator_type()) const
    {
        string_type s(allocator);
        std::basic_ostringstream<char_type> os(s);
        os.imbue(std::locale::classic());
        {
            basic_json_serializer<char_type> serializer(os, options);
            dump_fragment(serializer);
        }
        return os.str();
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    void write_body(basic_json_output_handler<char_type>& handler) const
    {
        dump(handler);
    }
    void write(basic_json_output_handler<char_type>& handler) const
    {
        dump(handler);
    }

    void write(std::basic_ostream<char_type>& os) const
    {
        dump(os);
    }

    void write(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options) const
    {
        dump(os,options);
    }

    void write(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool pprint) const
    {
        dump(os,options,pprint);
    }

    void to_stream(basic_json_output_handler<char_type>& handler) const
    {
        handler.begin_json();
        dump_fragment(handler);
        handler.end_json();
    }

    void to_stream(std::basic_ostream<char_type>& os) const
    {
        basic_json_serializer<char_type> serializer(os);
        to_stream(serializer);
    }

    void to_stream(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options) const
    {
        basic_json_serializer<char_type> serializer(os, options);
        to_stream(serializer);
    }

    void to_stream(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool pprint) const
    {
        basic_json_serializer<char_type> serializer(os, options, pprint);
        to_stream(serializer);
    }
#endif
    bool is_null() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::null_t;
    }

    bool has_key(string_view_type name) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::object_t:
            {
                const_object_iterator it = object_value().find(name);
                return it != object_range().end();
            }
            break;
        default:
            return false;
        }
    }

    size_t count(string_view_type name) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::object_t:
            {
                auto it = object_value().find(name);
                if (it == object_range().end())
                {
                    return 0;
                }
                size_t count = 0;
                while (it != object_range().end() && it->key() == name)
                {
                    ++count;
                    ++it;
                }
                return count;
            }
            break;
        default:
            return 0;
        }
    }

    template<class T, class... Args>
    bool is(Args&&... args) const
    {
        return json_type_traits<basic_json,T>::is(*this,std::forward<Args>(args)...);
    }

    bool is_string() const JSONCONS_NOEXCEPT
    {
        return (var_.type_id() == json_type_tag::string_t) || (var_.type_id() == json_type_tag::small_string_t);
    }


    bool is_bool() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::bool_t;
    }

    bool is_object() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::object_t || var_.type_id() == json_type_tag::empty_object_t;
    }

    bool is_array() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::array_t;
    }

    bool is_integer() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::integer_t || (var_.type_id() == json_type_tag::uinteger_t && (as_uinteger() <= static_cast<uint64_t>((std::numeric_limits<long long>::max)())));
    }

    bool is_uinteger() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::uinteger_t || (var_.type_id() == json_type_tag::integer_t && as_integer() >= 0);
    }

    bool is_double() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::double_t;
    }

    bool is_number() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::integer_t || var_.type_id() == json_type_tag::uinteger_t || var_.type_id() == json_type_tag::double_t;
    }

    bool empty() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
            return var_.small_string_data_cast()->length() == 0;
        case json_type_tag::string_t:
            return var_.string_data_cast()->length() == 0;
        case json_type_tag::array_t:
            return array_value().size() == 0;
        case json_type_tag::empty_object_t:
            return true;
        case json_type_tag::object_t:
            return object_value().size() == 0;
        default:
            return false;
        }
    }

    size_t capacity() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return array_value().capacity();
        case json_type_tag::object_t:
            return object_value().capacity();
        default:
            return 0;
        }
    }

    template<class U=Allocator>
    typename std::enable_if<is_stateless<U>::value,void>::type
    create_object_implicitly()
    {
        var_ = variant(Allocator());
    }

    template<class U=Allocator>
    typename std::enable_if<!is_stateless<U>::value,void>::type
    create_object_implicitly() const
    {
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Cannot create object implicitly - allocator is not default constructible.");
    }

    void reserve(size_t n)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().reserve(n);
            break;
        case json_type_tag::empty_object_t:
        {
            create_object_implicitly();
            object_value().reserve(n);
        }
        break;
        case json_type_tag::object_t:
        {
            object_value().reserve(n);
        }
            break;
        default:
            break;
        }
    }

    void resize(size_t n)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().resize(n);
            break;
        default:
            break;
        }
    }

    template <class T>
    void resize(size_t n, T val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().resize(n, val);
            break;
        default:
            break;
        }
    }

    template<class T, class... Args>
    T as(Args&&... args) const
    {
        return json_type_traits<basic_json,T>::as(*this,std::forward<Args>(args)...);
    }

    template<class T>
    typename std::enable_if<std::is_same<string_type,T>::value,T>::type 
    as(const char_allocator_type& allocator) const
    {
        return json_type_traits<basic_json,T>::as(*this,allocator);
    }

    bool as_bool() const 
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            try
            {
                basic_json j = basic_json::parse(as_string_view().data(),as_string_view().length());
                return j.as_bool();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a bool");
            }
            break;
        case json_type_tag::bool_t:
            return var_.bool_data_cast()->value();
        case json_type_tag::double_t:
            return var_.double_data_cast()->value() != 0.0;
        case json_type_tag::integer_t:
            return var_.integer_data_cast()->value() != 0;
        case json_type_tag::uinteger_t:
            return var_.uinteger_data_cast()->value() != 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a bool");
        }
    }

    int64_t as_integer() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            try
            {
                basic_json j = basic_json::parse(as_string_view().data(),as_string_view().length());
                return j.as<int64_t>();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an integer");
            }
            break;
        case json_type_tag::double_t:
            return static_cast<int64_t>(var_.double_data_cast()->value());
        case json_type_tag::integer_t:
            return static_cast<int64_t>(var_.integer_data_cast()->value());
        case json_type_tag::uinteger_t:
            return static_cast<int64_t>(var_.uinteger_data_cast()->value());
        case json_type_tag::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an integer");
        }
    }

    uint64_t as_uinteger() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            try
            {
                basic_json j = basic_json::parse(as_string_view().data(),as_string_view().length());
                return j.as<uint64_t>();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned integer");
            }
            break;
        case json_type_tag::double_t:
            return static_cast<uint64_t>(var_.double_data_cast()->value());
        case json_type_tag::integer_t:
            return static_cast<uint64_t>(var_.integer_data_cast()->value());
        case json_type_tag::uinteger_t:
            return static_cast<uint64_t>(var_.uinteger_data_cast()->value());
        case json_type_tag::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned integer");
        }
    }

    size_t double_precision() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::double_t:
            return var_.double_data_cast()->precision();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a double");
        }
    }

    double as_double() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            try
            {
                basic_json j = basic_json::parse(as_string_view().data(),as_string_view().length());
                return j.as<double>();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a double");
            }
            break;
        case json_type_tag::double_t:
            return var_.double_data_cast()->value();
        case json_type_tag::integer_t:
            return static_cast<double>(var_.integer_data_cast()->value());
        case json_type_tag::uinteger_t:
            return static_cast<double>(var_.uinteger_data_cast()->value());
        case json_type_tag::null_t:
            return std::numeric_limits<double>::quiet_NaN();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a double");
        }
    }

    string_view_type as_string_view() const
    {
        return var_.as_string_view();
    }

    string_type as_string() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            return string_type(as_string_view().data(),as_string_view().length());
        default:
            return to_string();
        }
    }

    template <class SAllocator>
    string_type as_string(const SAllocator& allocator) const JSONCONS_NOEXCEPT
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            return string_type(as_string_view().data(),as_string_view().length(),allocator);
        default:
            return to_string(allocator);
        }
    }

    string_type as_string(const basic_serialization_options<char_type>& options) const 
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            return string_type(as_string_view().data(),as_string_view().length());
        default:
            return to_string(options);
        }
    }

    template <class SAllocator>
    string_type as_string(const basic_serialization_options<char_type>& options,
                          const SAllocator& allocator) const 
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            return string_type(as_string_view().data(),as_string_view().length(),allocator);
        default:
            return to_string(options,allocator);
        }
    }

    const char_type* as_cstring() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
            return var_.small_string_data_cast()->c_str();
        case json_type_tag::string_t:
            return var_.string_data_cast()->c_str();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a cstring");
        }
    }
    basic_json& at(string_view_type name)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"%s not found", name);
        case json_type_tag::object_t:
            {
                auto it = object_value().find(name);
                if (it == object_range().end())
                {
                    JSONCONS_THROW_EXCEPTION_1(std::out_of_range, "%s not found", name);
                }
                return it->value();
            }
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    basic_json& evaluate() 
    {
        return *this;
    }

    basic_json& evaluate_with_default() 
    {
        return *this;
    }

    const basic_json& evaluate() const
    {
        return *this;
    }
    basic_json& evaluate(string_view_type name) 
    {
        return at(name);
    }

    const basic_json& evaluate(string_view_type name) const
    {
        return at(name);
    }

    const basic_json& at(string_view_type name) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"%s not found", name);
        case json_type_tag::object_t:
            {
                auto it = object_value().find(name);
                if (it == object_range().end())
                {
                    JSONCONS_THROW_EXCEPTION_1(std::out_of_range, "%s not found", name);
                }
                return it->value();
            }
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    basic_json& at(size_t i)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            if (i >= array_value().size())
            {
                JSONCONS_THROW_EXCEPTION(std::out_of_range,"Invalid array subscript");
            }
            return array_value().operator[](i);
        case json_type_tag::object_t:
            return object_value().at(i);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Index on non-array value not supported");
        }
    }

    const basic_json& at(size_t i) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            if (i >= array_value().size())
            {
                JSONCONS_THROW_EXCEPTION(std::out_of_range,"Invalid array subscript");
            }
            return array_value().operator[](i);
        case json_type_tag::object_t:
            return object_value().at(i);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Index on non-array value not supported");
        }
    }

    object_iterator find(string_view_type name)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            return object_range().end();
        case json_type_tag::object_t:
            return object_value().find(name);
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    const_object_iterator find(string_view_type name) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            return object_range().end();
        case json_type_tag::object_t:
            return object_value().find(name);
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    template<class T>
    basic_json get(string_view_type name, T&& default_val) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            {
                return basic_json(std::forward<T>(default_val));
            }
        case json_type_tag::object_t:
            {
                const_object_iterator it = object_value().find(name);
                if (it != object_range().end())
                {
                    return it->value();
                }
                else
                {
                    return basic_json(std::forward<T>(default_val));
                }
            }
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    template<class T>
    T get_with_default(string_view_type name, const T& default_val) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            {
                return default_val;
            }
        case json_type_tag::object_t:
            {
                const_object_iterator it = object_value().find(name);
                if (it != object_range().end())
                {
                    return it->value().template as<T>();
                }
                else
                {
                    return default_val;
                }
            }
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    const CharT* get_with_default(string_view_type name, const CharT* default_val) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            {
                return default_val;
            }
        case json_type_tag::object_t:
            {
                const_object_iterator it = object_value().find(name);
                if (it != object_range().end())
                {
                    return it->value().template as<const CharT*>();
                }
                else
                {
                    return default_val;
                }
            }
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    // Modifiers

    void shrink_to_fit()
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().shrink_to_fit();
            break;
        case json_type_tag::object_t:
            object_value().shrink_to_fit();
            break;
        default:
            break;
        }
    }

    void clear()
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().clear();
            break;
        case json_type_tag::object_t:
            object_value().clear();
            break;
        default:
            break;
        }
    }

    void erase(const_object_iterator pos)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            break;
        case json_type_tag::object_t:
            object_value().erase(pos);
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
            break;
        }
    }

    void erase(const_object_iterator first, const_object_iterator last)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            break;
        case json_type_tag::object_t:
            object_value().erase(first, last);
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
            break;
        }
    }

    void erase(const_array_iterator pos)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().erase(pos);
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
            break;
        }
    }

    void erase(const_array_iterator first, const_array_iterator last)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().erase(first, last);
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
            break;
        }
    }

    // Removes all elements from an array value whose index is between from_index, inclusive, and to_index, exclusive.

    void erase(string_view_type name)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            break;
        case json_type_tag::object_t:
            object_value().erase(name);
            break;
        default:
            JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to erase %s on a value that is not an object", name);
            break;
        }
    }

    template <class T>
    std::pair<object_iterator,bool> set(string_view_type name, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().insert_or_assign(name, std::forward<T>(val));
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class T>
    std::pair<object_iterator,bool> insert_or_assign(string_view_type name, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().insert_or_assign(name, std::forward<T>(val));
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class ... Args>
    std::pair<object_iterator,bool> try_emplace(string_view_type name, Args&&... args)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().try_emplace(name, std::forward<Args>(args)...);
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class T>
    void set_(key_storage_type&& name, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            object_value().set_(std::forward<key_storage_type>(name), std::forward<T>(val));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to call set on a value that is not an object");
            }
        }
    }

    // merge

    void merge(const basic_json& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge(source.object_value());
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge a value that is not an object");
            }
        }
    }

    void merge(basic_json&& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge(std::move(source.object_value()));
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge a value that is not an object");
            }
        }
    }

    void merge(object_iterator hint, const basic_json& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge(hint, source.object_value());
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge a value that is not an object");
            }
        }
    }

    void merge(object_iterator hint, basic_json&& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge(hint, std::move(source.object_value()));
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge a value that is not an object");
            }
        }
    }

    // merge_or_update

    void merge_or_update(const basic_json& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge_or_update(source.object_value());
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge_or_update a value that is not an object");
            }
        }
    }

    void merge_or_update(basic_json&& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge_or_update(std::move(source.object_value()));
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge_or_update a value that is not an object");
            }
        }
    }

    void merge_or_update(object_iterator hint, const basic_json& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge_or_update(hint, source.object_value());
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge_or_update a value that is not an object");
            }
        }
    }

    void merge_or_update(object_iterator hint, basic_json&& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge_or_update(hint, std::move(source.object_value()));
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge_or_update a value that is not an object");
            }
        }
    }

    // set

    template <class T>
    object_iterator set(object_iterator hint, string_view_type name, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().insert_or_assign(hint, name, std::forward<T>(val));
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class T>
    object_iterator insert_or_assign(object_iterator hint, string_view_type name, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().insert_or_assign(hint, name, std::forward<T>(val));
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class ... Args>
    object_iterator try_emplace(object_iterator hint, string_view_type name, Args&&... args)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().try_emplace(hint, name, std::forward<Args>(args)...);
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class T>
    object_iterator set_(object_iterator hint, key_storage_type&& name, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().set_(hint, std::forward<key_storage_type>(name), std::forward<T>(val));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to set on a value that is not an object");
            }
        }
    }

    template <class T>
    void add(T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().push_back(std::forward<T>(val));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    template <class T>
    void push_back(T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().push_back(std::forward<T>(val));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    template <class T>
    array_iterator add(const_array_iterator pos, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return array_value().insert(pos, std::forward<T>(val));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    template <class T>
    array_iterator insert(const_array_iterator pos, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return array_value().insert(pos, std::forward<T>(val));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    template <class InputIt>
    array_iterator insert(const_array_iterator pos, InputIt first, InputIt last)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return array_value().insert(pos, first, last);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    template <class... Args> 
    array_iterator emplace(const_array_iterator pos, Args&&... args)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return array_value().emplace(pos, std::forward<Args>(args)...);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    template <class... Args> 
    basic_json& emplace_back(Args&&... args)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return array_value().emplace_back(std::forward<Args>(args)...);
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    json_type_tag type_id() const
    {
        return var_.type_id();
    }

    void swap(basic_json& b)
    {
        var_.swap(b.var_);
    }

    friend void swap(basic_json& a, basic_json& b)
    {
        a.swap(b);
    }

    static basic_json make_string(string_view_type s)
    {
        return basic_json(variant(s.data(),s.length()));
    }

    static basic_json make_string(const char_type* rhs, size_t length)
    {
        return basic_json(variant(rhs,length));
    }

    static basic_json make_string(string_view_type s, allocator_type allocator)
    {
        return basic_json(variant(s.data(),s.length(),allocator));
    }

    static basic_json from_integer(int64_t val)
    {
        return basic_json(variant(val));
    }

    static basic_json from_integer(int64_t val, allocator_type)
    {
        return basic_json(variant(val));
    }

    static basic_json from_uinteger(uint64_t val)
    {
        return basic_json(variant(val));
    }

    static basic_json from_uinteger(uint64_t val, allocator_type)
    {
        return basic_json(variant(val));
    }

    static basic_json from_floating_point(double val)
    {
        return basic_json(variant(val, std::numeric_limits<double>::digits10));
    }

    static basic_json from_floating_point(double val, allocator_type)
    {
        return basic_json(variant(val,std::numeric_limits<double>::digits10));
    }

    static basic_json from_bool(bool val)
    {
        return basic_json(variant(val));
    }

    static basic_json make_object(const object& o)
    {
        return basic_json(variant(o));
    }

    static basic_json make_object(const object& o, allocator_type allocator)
    {
        return basic_json(variant(o,allocator));
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    static basic_json parse_stream(std::basic_istream<char_type>& is)
    {
        return parse(is);
    }
    static basic_json parse_stream(std::basic_istream<char_type>& is, parse_error_handler& err_handler)
    {
        return parse(is,err_handler);
    }

    static basic_json parse_string(const string_type& s)
    {
        return parse(s);
    }

    static basic_json parse_string(const string_type& s, parse_error_handler& err_handler)
    {
        return parse(s,err_handler);
    }

    void resize_array(size_t n)
    {
        resize(n);
    }

    template <class T>
    void resize_array(size_t n, T val)
    {
        resize(n,val);
    }

    object_iterator begin_members()
    {
        return object_range().begin();
    }

    const_object_iterator begin_members() const
    {
        return object_range().begin();
    }

    object_iterator end_members()
    {
        return object_range().end();
    }

    const_object_iterator end_members() const
    {
        return object_range().end();
    }

    array_iterator begin_elements()
    {
        return array_range().begin();
    }

    const_array_iterator begin_elements() const
    {
        return array_range().begin();
    }

    array_iterator end_elements()
    {
        return array_range().end();
    }

    const_array_iterator end_elements() const
    {
        return array_range().end();
    }

    const basic_json& get(string_view_type name) const
    {
        static const basic_json a_null = null_type();

        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            return a_null;
        case json_type_tag::object_t:
            {
                const_object_iterator it = object_value().find(name);
                return it != object_range().end() ? it->value() : a_null;
            }
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    bool is_longlong() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::integer_t;
    }

    bool is_ulonglong() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::uinteger_t;
    }

    long long as_longlong() const
    {
        return as_integer();
    }

    unsigned long long as_ulonglong() const
    {
        return as_uinteger();
    }

    int as_int() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::double_t:
            return static_cast<int>(var_.double_data_cast()->value());
        case json_type_tag::integer_t:
            return static_cast<int>(var_.integer_data_cast()->value());
        case json_type_tag::uinteger_t:
            return static_cast<int>(var_.uinteger_data_cast()->value());
        case json_type_tag::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an int");
        }
    }

    unsigned int as_uint() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::double_t:
            return static_cast<unsigned int>(var_.double_data_cast()->value());
        case json_type_tag::integer_t:
            return static_cast<unsigned int>(var_.integer_data_cast()->value());
        case json_type_tag::uinteger_t:
            return static_cast<unsigned int>(var_.uinteger_data_cast()->value());
        case json_type_tag::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned int");
        }
    }

    long as_long() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::double_t:
            return static_cast<long>(var_.double_data_cast()->value());
        case json_type_tag::integer_t:
            return static_cast<long>(var_.integer_data_cast()->value());
        case json_type_tag::uinteger_t:
            return static_cast<long>(var_.uinteger_data_cast()->value());
        case json_type_tag::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a long");
        }
    }

    unsigned long as_ulong() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::double_t:
            return static_cast<unsigned long>(var_.double_data_cast()->value());
        case json_type_tag::integer_t:
            return static_cast<unsigned long>(var_.integer_data_cast()->value());
        case json_type_tag::uinteger_t:
            return static_cast<unsigned long>(var_.uinteger_data_cast()->value());
        case json_type_tag::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned long");
        }
    }

    bool has_member(const key_storage_type& name) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::object_t:
            {
                const_object_iterator it = object_value().find(name);
                return it != object_range().end();
            }
            break;
        default:
            return false;
        }
    }

    void remove_range(size_t from_index, size_t to_index)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().remove_range(from_index, to_index);
            break;
        default:
            break;
        }
    }
    // Removes all elements from an array value whose index is between from_index, inclusive, and to_index, exclusive.

    void remove(string_view_type name)
    {
        erase(name);
    }
    void remove_member(string_view_type name)
    {
        erase(name);
    }
    // Removes a member from an object value

    bool is_empty() const JSONCONS_NOEXCEPT
    {
        return empty();
    }
    bool is_numeric() const JSONCONS_NOEXCEPT
    {
        return is_number();
    }

    template<int size>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array()
    {
        return make_array();
    }
    template<size_t size>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array(size_t n)
    {
        return make_array(n);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array(size_t n, T val)
    {
        return make_array(n,val);
    }
    template<size_t size>
    static typename std::enable_if<size==2,basic_json>::type make_multi_array(size_t m, size_t n)
    {
        return make_array<2>(m, n);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==2,basic_json>::type make_multi_array(size_t m, size_t n, T val)
    {
        return make_array<2>(m, n, val);
    }
    template<size_t size>
    static typename std::enable_if<size==3,basic_json>::type make_multi_array(size_t m, size_t n, size_t k)
    {
        return make_array<3>(m, n, k);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==3,basic_json>::type make_multi_array(size_t m, size_t n, size_t k, T val)
    {
        return make_array<3>(m, n, k, val);
    }
    range<object_iterator> members()
    {
        return object_range();
    }

    range<const_object_iterator> members() const
    {
        return object_range();
    }

    range<array_iterator> elements()
    {
        return array_range();
    }

    range<const_array_iterator> elements() const
    {
        return array_range();
    }
#endif

    range<object_iterator> object_range()
    {
        static basic_json empty_object = object();
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            return range<object_iterator>(empty_object.object_range().begin(), empty_object.object_range().end());
        case json_type_tag::object_t:
            return range<object_iterator>(object_value().begin(),object_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
        }
    }

    range<const_object_iterator> object_range() const
    {
        static const basic_json empty_object = object();
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            return range<const_object_iterator>(empty_object.object_range().begin(), empty_object.object_range().end());
        case json_type_tag::object_t:
            return range<const_object_iterator>(object_value().begin(),object_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
        }
    }

    range<array_iterator> array_range()
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return range<array_iterator>(array_value().begin(),array_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
        }
    }

    range<const_array_iterator> array_range() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return range<const_array_iterator>(array_value().begin(),array_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
        }
    }

    array& array_value() 
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return var_.array_data_cast()->value();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad array cast");
            break;
        }
    }

    const array& array_value() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return var_.array_data_cast()->value();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad array cast");
            break;
        }
    }

    object& object_value()
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return var_.object_data_cast()->value();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad object cast");
            break;
        }
    }

    const object& object_value() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            const_cast<basic_json*>(this)->create_object_implicitly(); // HERE
            // FALLTHRU
        case json_type_tag::object_t:
            return var_.object_data_cast()->value();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad object cast");
            break;
        }
    }

private:

    friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const basic_json& o)
    {
        o.dump(os);
        return os;
    }

    friend std::basic_istream<char_type>& operator<<(std::basic_istream<char_type>& is, basic_json& o)
    {
        json_decoder<basic_json> handler;
        basic_json_reader<char_type> reader(is, handler);
        reader.read_next();
        reader.check_done();
        if (!handler.is_valid())
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json stream");
        }
        o = handler.get_result();
        return is;
    }
};

template <class Json>
void swap(typename Json::key_value_pair_type & a, typename Json::key_value_pair_type & b)
{
    a.swap(b);
}

template<class CharT,class JsonTraits,class Allocator>
basic_json<CharT,JsonTraits,Allocator> basic_json<CharT,JsonTraits,Allocator>::parse(std::basic_istream<char_type>& is)
{
    parse_error_handler_type err_handler;
    return parse(is,err_handler);
}

template<class CharT,class JsonTraits,class Allocator>
basic_json<CharT,JsonTraits,Allocator> basic_json<CharT,JsonTraits,Allocator>::parse(std::basic_istream<char_type>& is, 
                                                                                            parse_error_handler& err_handler)
{
    json_decoder<basic_json<CharT,JsonTraits,Allocator>> handler;
    basic_json_reader<char_type> reader(is, handler, err_handler);
    reader.read_next();
    reader.check_done();
    if (!handler.is_valid())
    {
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json stream");
    }
    return handler.get_result();
}

template <class Json>
std::basic_istream<typename Json::char_type>& operator>>(std::basic_istream<typename Json::char_type>& is, Json& o)
{
    json_decoder<Json> handler;
    basic_json_reader<typename Json::char_type> reader(is, handler);
    reader.read_next();
    reader.check_done();
    if (!handler.is_valid())
    {
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json stream");
    }
    o = handler.get_result();
    return is;
}

template<class Json>
class json_printable
{
public:
    typedef typename Json::char_type char_type;

    json_printable(const Json& o,
                   bool is_pretty_print)
       : o_(&o), is_pretty_print_(is_pretty_print)
    {
    }

    json_printable(const Json& o,
                   bool is_pretty_print,
                   const basic_serialization_options<char_type>& options)
       : o_(&o), is_pretty_print_(is_pretty_print), options_(options)
    {
        ;
    }

    void dump(std::basic_ostream<char_type>& os) const
    {
        o_->dump(os, options_, is_pretty_print_);
    }

    friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const json_printable<Json>& o)
    {
        o.dump(os);
        return os;
    }

    const Json *o_;
    bool is_pretty_print_;
    basic_serialization_options<char_type> options_;
private:
    json_printable();
};

template<class Json>
json_printable<Json> print(const Json& val)
{
    return json_printable<Json>(val,false);
}

template<class Json>
json_printable<Json> print(const Json& val,
                            const basic_serialization_options<typename Json::char_type>& options)
{
    return json_printable<Json>(val, false, options);
}

template<class Json>
json_printable<Json> pretty_print(const Json& val)
{
    return json_printable<Json>(val,true);
}

template<class Json>
json_printable<Json> pretty_print(const Json& val,
                                   const basic_serialization_options<typename Json::char_type>& options)
{
    return json_printable<Json>(val, true, options);
}

typedef basic_json<char,json_traits<char>,std::allocator<char>> json;
typedef basic_json<wchar_t,json_traits<wchar_t>,std::allocator<wchar_t>> wjson;
typedef basic_json<char, o_json_traits<char>, std::allocator<char>> ojson;
typedef basic_json<wchar_t, o_json_traits<wchar_t>, std::allocator<wchar_t>> wojson;

#if !defined(JSONCONS_NO_DEPRECATED)
typedef basic_json<wchar_t, o_json_traits<wchar_t>, std::allocator<wchar_t>> owjson;
typedef json_decoder<json> json_deserializer;
typedef json_decoder<wjson> wjson_deserializer;
typedef json_decoder<ojson> ojson_deserializer;
typedef json_decoder<wojson> wojson_deserializer;
#endif

#if defined(JSONCONS_HAS_USER_DEFINED_LITERALS)
namespace literals {

inline 
jsoncons::json operator "" _json(const char* s, std::size_t n)
{
    return jsoncons::json::parse(s, n);
}

inline 
jsoncons::wjson operator "" _json(const wchar_t* s, std::size_t n)
{
    return jsoncons::wjson::parse(s, n);
}

inline
jsoncons::ojson operator "" _ojson(const char* s, std::size_t n)
{
    return jsoncons::ojson::parse(s, n);
}

inline
jsoncons::wojson operator "" _ojson(const wchar_t* s, std::size_t n)
{
    return jsoncons::wojson::parse(s, n);
}

}
#endif

}


#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
