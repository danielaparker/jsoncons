// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <new>
#include <string>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json.hpp"

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_variant_test_suite)

template <size_t arg1, size_t ... argn>
struct static_max;

template <size_t arg>
struct static_max<arg>
{
    static const size_t value = arg;
};

template <size_t arg1, size_t arg2, size_t ... argn>
struct static_max<arg1,arg2,argn ...>
{
    static const size_t value = arg1 >= arg2 ? 
        static_max<arg1,argn...>::value :
        static_max<arg2,argn...>::value; 
};

typedef json json_type;
typedef char char_type;
typedef std::string string_type;
typedef std::allocator<char> string_allocator;
typedef std::allocator<char> Allocator;
typedef json_object<string_type,json,true,Allocator> object;
typedef json_array<json,Allocator> array;
typedef name_value_pair<string_type,json_type> member_type;
typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<json_type> array_allocator;
typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<member_type> object_allocator;

enum class value_types : uint8_t 
{
    // Simple types
    empty_object_t,
    small_string_t,
    double_t,
    integer_t,
    uinteger_t,
    bool_t,
    null_t,
    // Non simple types
    string_t,
    object_t,
    array_t
#if !defined(JSONCONS_NO_DEPRECATED)
    ,
    any_t
#endif
};

struct base_data
{
    value_types type_id_;

    base_data(value_types id)
        : type_id_(id)
    {}
};

struct null_data : public base_data
{
    null_data()
        : base_data(value_types::null_t)
    {
    }
};

struct empty_object_data : public base_data
{
    empty_object_data()
        : base_data(value_types::empty_object_t)
    {
    }
};

struct bool_data : public base_data
{
    bool val_;

    bool_data(bool val)
        : base_data(value_types::bool_t),val_(val)
    {
    }

    bool_data(const bool_data& val)
        : base_data(value_types::bool_t),val_(val.val_)
    {
    }
};

struct integer_data : public base_data
{
    int64_t val_;

    integer_data(int64_t val)
        : base_data(value_types::integer_t),val_(val)
    {
    }

    integer_data(const integer_data& val)
        : base_data(value_types::integer_t),val_(val.val_)
    {
    }
};

struct uinteger_data : public base_data
{
    uint64_t val_;

    uinteger_data(uint64_t val)
        : base_data(value_types::uinteger_t),val_(val)
    {
    }

    uinteger_data(const uinteger_data& val)
        : base_data(value_types::uinteger_t),val_(val.val_)
    {
    }
};

struct double_data : public base_data
{
    uint8_t precision_;
    double val_;

    double_data(double val, uint8_t precision)
        : base_data(value_types::double_t), 
          precision_(precision), 
          val_(val)
    {
    }

    double_data(const double_data& val)
        : base_data(value_types::double_t),
          precision_(val.precision_), 
          val_(val.val_)
    {
    }
};

struct small_string_data : public base_data
{
    static const size_t capacity = 14/sizeof(char_type);
    static const size_t max_length = (14 / sizeof(char_type)) - 1;

    uint8_t length_;
    char_type data_[capacity];

    small_string_data(const char_type* p, uint8_t length)
        : base_data(value_types::small_string_t), length_(length)
    {
        JSONCONS_ASSERT(length <= max_length);
        std::memcpy(data_,p,length*sizeof(char_type));
        data_[length] = 0;
    }

    small_string_data(const small_string_data& val)
        : base_data(value_types::small_string_t), length_(val.length_)
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
};

struct string_data : public base_data
{
    struct string_holder : public string_allocator
    {
        const char_type* c_str() const { return p_; }
        const char_type* data() const { return p_; }
        size_t length() const { return length_; }
        string_allocator get_allocator() const
        {
            return *this;
        }

        bool operator==(const string_holder& rhs) const
        {
            return length() == rhs.length() ? std::char_traits<char_type>::compare(data(), rhs.data(), length()) == 0 : false;
        }

        string_holder(const string_allocator& allocator)
            : string_allocator(allocator), length_(0), p_(nullptr)
        {
        }

        size_t length_;
        char_type* p_;
    private:
        string_holder(const string_holder&);
        string_holder& operator=(const string_holder&);
    };

    struct string_holderA
    {
        string_holder data;
        char_type c[1];
    };

    typedef typename std::aligned_storage<sizeof(string_holderA), JSONCONS_ALIGNOF(string_holderA)>::type storage_type;

    static size_t aligned_size(size_t n)
    {
        return sizeof(storage_type) + n;
    }

    string_holder* create_string_holder(const char_type* s, size_t length, const string_allocator& allocator)
    {
        size_t mem_size = aligned_size(length*sizeof(char_type));

        typename std::allocator_traits<string_allocator>:: template rebind_alloc<char> alloc(allocator);

        char* storage = alloc.allocate(mem_size);
        string_holder* ps = new(storage)string_holder(allocator);
        auto psa = reinterpret_cast<string_holderA*>(storage); 

        ps->p_ = new(&psa->c)char_type[length + 1];
        memcpy(ps->p_, s, length*sizeof(char_type));
        ps->p_[length] = 0;
        ps->length_ = length;
        return ps;
    }

    void destroy_string_holder(const string_allocator& allocator, string_holder* p)
    {
        size_t mem_size = aligned_size(p->length_*sizeof(char_type));
        typename std::allocator_traits<string_allocator>:: template rebind_alloc<char> alloc(allocator);
        alloc.deallocate(reinterpret_cast<char*>(p),mem_size);
    }
    string_holder* holder_;

    string_data(const char_type* s, size_t length, const string_allocator& alloc)
        : base_data(value_types::string_t)
    {
        holder_ = create_string_holder(s, length, alloc);
    }

    string_data(const string_data& val)
        : base_data(value_types::string_t)
    {
        holder_ = create_string_holder(val.holder_->p_, 
                                     val.holder_->length_, 
                                     val.holder_->get_allocator());
    }

    string_data(const string_data& val, string_allocator allocator)
        : base_data(value_types::string_t)
    {
        holder_ = create_string_holder(val.holder_->p_, 
                                       val.holder_->length_, 
                                       allocator);
    }
    ~string_data()
    {
        destroy_string_holder(holder_->get_allocator(), holder_);
    }

    size_t length() const
    {
        return holder_->length_;
    }

    const char_type* data() const
    {
        return holder_->p_;
    }
};


struct object_data : public base_data
{
    object* data_;

    explicit object_data(const Allocator& a)
        : base_data(value_types::object_t)
    {
        data_ = create_impl<object>(a, object_allocator(a));
    }

    explicit object_data(const object & val)
        : base_data(value_types::object_t)
    {
        data_ = create_impl<object>(val.get_allocator(), val);
    }

    explicit object_data(const object & val, const Allocator& a)
        : base_data(value_types::object_t)
    {
        data_ = create_impl<object>(a, val, object_allocator(a));
    }

    explicit object_data(const object_data & val)
        : base_data(value_types::object_t)
    {
        data_ = create_impl<object>(val.data_->get_allocator(), *(val.data_));
    }

    explicit object_data(const object_data & val, const Allocator& a)
        : base_data(value_types::object_t)
    {
        data_ = create_impl<object>(a, *(val.data_), object_allocator(a));
    }

    ~object_data()
    {
        destroy_impl(data_->get_allocator(), data_);
    }
};

struct array_data : public base_data
{
    array* data_;

    array_data(const array & val)
        : base_data(value_types::array_t)
    {
        data_ = create_impl<array>(val.get_allocator(), val);
    }

    array_data(const array & val, const Allocator& a)
        : base_data(value_types::array_t)
    {
        data_ = create_impl<array>(a, val, array_allocator(a));
    }

    array_data(const array_data & val)
        : base_data(value_types::array_t)
    {
        data_ = create_impl<array>(val.data_->get_allocator(), *(val.data_));
    }

    array_data(const array_data & val, const Allocator& a)
        : base_data(value_types::array_t)
    {
        data_ = create_impl<array>(a, *(val.data_), array_allocator(a));
    }
    ~array_data()
    {
        destroy_impl(data_->get_allocator(), data_);
    }
};

struct variant
{
//private:
    static const size_t data_size = static_max<sizeof(uinteger_data),sizeof(double_data),sizeof(small_string_data), sizeof(string_data)>::value;
    static const size_t data_align = static_max<JSONCONS_ALIGNOF(uinteger_data),JSONCONS_ALIGNOF(double_data),JSONCONS_ALIGNOF(small_string_data),JSONCONS_ALIGNOF(string_data)>::value;

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
        switch (val.type_id())
        {
        case value_types::null_t:
            new(reinterpret_cast<void*>(&data_))null_data();
            break;
        case value_types::empty_object_t:
            new(reinterpret_cast<void*>(&data_))empty_object_data();
            break;
        case value_types::double_t:
            new(reinterpret_cast<void*>(&data_))double_data(*(val.double_data_cast()));
            break;
        case value_types::integer_t:
            new(reinterpret_cast<void*>(&data_))integer_data(*(val.integer_data_cast()));
            break;
        case value_types::uinteger_t:
            new(reinterpret_cast<void*>(&data_))uinteger_data(*(val.uinteger_data_cast()));
            break;
        case value_types::bool_t:
            new(reinterpret_cast<void*>(&data_))bool_data(*(val.bool_data_cast()));
            break;
        case value_types::small_string_t:
            new(reinterpret_cast<void*>(&data_))small_string_data(*(val.small_string_data_cast()));
            break;
        case value_types::string_t:
            new(reinterpret_cast<void*>(&data_))string_data(*(val.string_data_cast()));
            break;
        case value_types::object_t:
            new(reinterpret_cast<void*>(&data_))object_data(*(val.object_data_cast()));
            break;
        case value_types::array_t:
            new(reinterpret_cast<void*>(&data_))array_data(*(val.array_data_cast()));
            break;
        default:
            break;
        }
    }

    variant(const variant& val, const Allocator& allocator)
    {
        switch (val.type_id())
        {
        case value_types::null_t:
            new(reinterpret_cast<void*>(&data_))null_data();
            break;
        case value_types::empty_object_t:
            new(reinterpret_cast<void*>(&data_))empty_object_data();
            break;
        case value_types::double_t:
            new(reinterpret_cast<void*>(&data_))double_data(*(val.double_data_cast()));
            break;
        case value_types::integer_t:
            new(reinterpret_cast<void*>(&data_))integer_data(*(val.integer_data_cast()));
            break;
        case value_types::uinteger_t:
            new(reinterpret_cast<void*>(&data_))uinteger_data(*(val.uinteger_data_cast()));
            break;
        case value_types::bool_t:
            new(reinterpret_cast<void*>(&data_))bool_data(*(val.bool_data_cast()));
            break;
        case value_types::small_string_t:
            new(reinterpret_cast<void*>(&data_))string_data(val.small_string_data_cast()->data(), val.small_string_data_cast()->length(),allocator);
            break;
        case value_types::string_t:
            new(reinterpret_cast<void*>(&data_))string_data(*(val.string_data_cast()),allocator);
            break;
        case value_types::object_t:
            new(reinterpret_cast<void*>(&data_))object_data(*(val.object_data_cast()),allocator);
            break;
        case value_types::array_t:
            new(reinterpret_cast<void*>(&data_))array_data(*(val.array_data_cast()),allocator);
            break;
        default:
            break;
        }
    }

    variant(variant&& val)
    {
        new(reinterpret_cast<void*>(&data_))null_data();
        swap(val);
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
    explicit variant(uint64_t val)
    {
        new(reinterpret_cast<void*>(&data_))uinteger_data(val);
    }
    variant(double val)
    {
        new(reinterpret_cast<void*>(&data_))double_data(val,0);
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
            new(reinterpret_cast<void*>(&data_))string_data(s, length, string_allocator());
        }
    }
    variant(const char_type* s)
    {
        size_t length = std::char_traits<char_type>::length(s);
        if (length <= small_string_data::max_length)
        {
            new(reinterpret_cast<void*>(&data_))small_string_data(s, static_cast<uint8_t>(length));
        }
        else
        {
            new(reinterpret_cast<void*>(&data_))string_data(s, length, string_allocator());
        }
    }
    variant(const char_type* s, size_t length, const Allocator& alloc)
    {
        new(reinterpret_cast<void*>(&data_))string_data(s, length, string_allocator(alloc));
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

    ~variant()
    {
        value_types id = type_id();
        switch (id)
        {
        case value_types::string_t:
            reinterpret_cast<string_data*>(&data_)->~string_data();
            break;
        case value_types::object_t:
            reinterpret_cast<object_data*>(&data_)->~object_data();
            break;
        case value_types::array_t:
            reinterpret_cast<array_data*>(&data_)->~array_data();
            break;
        }
    }

    variant& operator=(const variant& val)
    {
        if (this != &val)
        {
            switch (val.type_id())
            {
            case value_types::null_t:
                new(reinterpret_cast<void*>(&data_))null_data();
                break;
            case value_types::empty_object_t:
                new(reinterpret_cast<void*>(&data_))empty_object_data();
                break;
            case value_types::double_t:
                new(reinterpret_cast<void*>(&data_))double_data(*(val.double_data_cast()));
                break;
            case value_types::integer_t:
                new(reinterpret_cast<void*>(&data_))integer_data(*(val.integer_data_cast()));
                break;
            case value_types::uinteger_t:
                new(reinterpret_cast<void*>(&data_))uinteger_data(*(val.uinteger_data_cast()));
                break;
            case value_types::bool_t:
                new(reinterpret_cast<void*>(&data_))bool_data(*(val.bool_data_cast()));
                break;
            case value_types::small_string_t:
                new(reinterpret_cast<void*>(&data_))small_string_data(*(val.small_string_data_cast()));
                break;
            case value_types::string_t:
                reinterpret_cast<string_data*>(&data_)->~string_data();
                new(reinterpret_cast<void*>(&data_))string_data(*(val.string_data_cast()));
                break;
            case value_types::object_t:
                reinterpret_cast<object_data*>(&data_)->~object_data();
                new(reinterpret_cast<void*>(&data_))object_data(*(val.object_data_cast()));
                break;
            case value_types::array_t:
                reinterpret_cast<array_data*>(&data_)->~array_data();
                new(reinterpret_cast<void*>(&data_))array_data(*(val.array_data_cast()));
                break;
            default:
                break;
            }
        }
        return *this;
    }

    variant& operator=(variant&& val)
    {
        if (this != &val)
        {
            swap(val);
        }
        return *this;
    }

    value_types type_id() const
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

    const string_data* string_data_cast() const
    {
        return reinterpret_cast<const string_data*>(&data_);
    }

    const object_data* object_data_cast() const
    {
        return reinterpret_cast<const object_data*>(&data_);
    }

    const array_data* array_data_cast() const
    {
        return reinterpret_cast<const array_data*>(&data_);
    }

    bool operator==(const variant& rhs) const
    {
        if (this == &rhs)
        {
            return true;
        }

        const value_types id = type_id();
        const value_types rhs_id = rhs.type_id();

        if (id == rhs_id)
        {
            switch (id)
            {
            case value_types::null_t:
                return true;
            case value_types::empty_object_t:
                return true;
            case value_types::double_t:
                return double_data_cast()->val_ == rhs.double_data_cast()->val_;
            case value_types::integer_t:
                return integer_data_cast()->val_ == rhs.integer_data_cast()->val_;
            case value_types::uinteger_t:
                return uinteger_data_cast()->val_ == rhs.uinteger_data_cast()->val_;
            case value_types::bool_t:
                return bool_data_cast()->val_ == rhs.bool_data_cast()->val_;
            case value_types::small_string_t:
                return small_string_data_cast()->length() == rhs.small_string_data_cast()->length() &&
                       std::char_traits<char_type>::compare(small_string_data_cast()->data(),rhs.small_string_data_cast()->data(),small_string_data_cast()->length()) == 0;
            case value_types::string_t:
                return string_data_cast()->length() == rhs.string_data_cast()->length() &&
                       std::char_traits<char_type>::compare(string_data_cast()->data(),rhs.string_data_cast()->data(),string_data_cast()->length()) == 0;
            case value_types::object_t:
                return *(object_data_cast()->data_) == *(rhs.object_data_cast()->data_);
            case value_types::array_t:
                return *(array_data_cast()->data_) == *(rhs.array_data_cast()->data_);
            default:
                return false;
            }
        }

        switch (id)
        {
        case value_types::integer_t:
            if (rhs_id == value_types::double_t)
            {
                return static_cast<double>(integer_data_cast()->val_) == rhs.double_data_cast()->val_;
            }
            else if (rhs_id == value_types::uinteger_t && integer_data_cast()->val_ > 0)
            {
                return static_cast<uint64_t>(integer_data_cast()->val_) == rhs.uinteger_data_cast()->val_;
            }
            break;
        case value_types::uinteger_t:
            if (rhs_id == value_types::double_t)
            {
                return static_cast<double>(uinteger_data_cast()->val_) == rhs.double_data_cast()->val_;
            }
            else if (rhs_id == value_types::integer_t && rhs.integer_data_cast()->val_ > 0)
            {
                return uinteger_data_cast()->val_ == static_cast<uint64_t>(rhs.integer_data_cast()->val_);
            }
            break;
        case value_types::double_t:
            if (rhs_id == value_types::integer_t)
            {
                return double_data_cast()->val_ == static_cast<double>(rhs.integer_data_cast()->val_);
            }
            else if (rhs_id == value_types::uinteger_t)
            {
                return double_data_cast()->val_ == static_cast<double>(rhs.uinteger_data_cast()->val_);
            }
            break;
        case value_types::empty_object_t:
            if (rhs_id == value_types::object_t && rhs.object_data_cast()->data_->size() == 0)
            {
                return true;
            }
            break;
        case value_types::object_t:
            if (rhs_id == value_types::empty_object_t && object_data_cast()->data_->size() == 0)
            {
                return true;
            }
            break;
        default:
            break;
        }
        
        return false;
    }

    bool operator!=(const variant& rhs) const
    {
        return !(*this == rhs);
    }


    void swap(variant& rhs)
    {
        std::swap(data_,rhs.data_);
    }
};

BOOST_AUTO_TEST_CASE(test_variant)
{
    variant var1(int64_t(-100));
    BOOST_CHECK(value_types::integer_t == var1.type_id());
    variant var2(uint64_t(100));
    BOOST_CHECK(value_types::uinteger_t == var2.type_id());
    variant var3("Small string",12);
    BOOST_CHECK(value_types::small_string_t == var3.type_id());
    variant var4("Too long to fit in small string",31);
    BOOST_CHECK(value_types::string_t == var4.type_id());
    variant var5(true);
    BOOST_CHECK(value_types::bool_t == var5.type_id());
    variant var6;
    BOOST_CHECK(value_types::empty_object_t == var6.type_id());
    variant var7{ null_type() };
    BOOST_CHECK(value_types::null_t == var7.type_id());
    variant var8{ Allocator() };
    BOOST_CHECK(value_types::object_t == var8.type_id());
    variant var9(123456789.9,0);
    BOOST_CHECK(value_types::double_t == var9.type_id());

    std::cout << sizeof(variant) << std::endl;
}

BOOST_AUTO_TEST_CASE(test_move_constructor)
{
    int64_t val1 = -100;
    variant var1(val1);
    variant var2(std::move(var1));
    BOOST_CHECK(value_types::null_t == var1.type_id());
    BOOST_CHECK(value_types::integer_t == var2.type_id());
    BOOST_CHECK(var2.integer_data_cast()->val_ == val1);

    uint64_t val3 = 9999;
    variant var3(val3);
    variant var4(std::move(var3));
    BOOST_CHECK(value_types::null_t == var3.type_id());
    BOOST_CHECK(value_types::uinteger_t == var4.type_id());
    BOOST_CHECK(var4.uinteger_data_cast()->val_ == val3);

    double val5 = 123456789.9;
    variant var5(val5,0);
    variant var6(std::move(var5));
    BOOST_CHECK(value_types::null_t == var5.type_id());
    BOOST_CHECK(value_types::double_t == var6.type_id());
    BOOST_CHECK(var6.double_data_cast()->val_ == val5);

    std::string val7("Too long for small string");
    variant var7(val7.data(),val7.length());
    variant var8(std::move(var7));
    BOOST_CHECK(value_types::null_t == var7.type_id());
    BOOST_CHECK(value_types::string_t == var8.type_id());
    BOOST_CHECK(val7 == var8.string_data_cast()->data());
    BOOST_CHECK(val7.length() == var8.string_data_cast()->length());

    std::string val9("Small string");
    variant var9(val9.data(), val9.length());
    variant var10(std::move(var9));
    BOOST_CHECK(value_types::null_t == var9.type_id());
    BOOST_CHECK(value_types::small_string_t == var10.type_id());
    BOOST_CHECK(val9 == var10.small_string_data_cast()->data_);
    BOOST_CHECK(val9.length() == var10.small_string_data_cast()->length_);

    bool val11 = true;
    variant var11(val11);
    variant var12(std::move(var11));
    BOOST_CHECK(value_types::null_t == var11.type_id());
    BOOST_CHECK(value_types::bool_t == var12.type_id());
    BOOST_CHECK(var12.bool_data_cast()->val_ == val11);

    std::string val13("Too long for small string");
    variant var13(val13.data(), val13.length());
    variant var14(std::move(var13));
    BOOST_CHECK(value_types::null_t == var13.type_id());
    BOOST_CHECK(value_types::string_t == var14.type_id());
    BOOST_CHECK(val13 == var14.string_data_cast()->data());

    object val15 = { {"first",1},{"second",2} };
    variant var15(val15);
    variant var16(std::move(var15));
    BOOST_CHECK(value_types::null_t == var15.type_id());
    BOOST_CHECK(value_types::object_t == var16.type_id());
    BOOST_CHECK(val15 == *(var16.object_data_cast()->data_));

    array val17 = {1,2,3,4};
    variant var17(val17);
    variant var18(std::move(var17));
    BOOST_CHECK(value_types::null_t == var17.type_id());
    BOOST_CHECK(value_types::array_t == var18.type_id());
    BOOST_CHECK(val17 == *(var18.array_data_cast()->data_));
}

BOOST_AUTO_TEST_CASE(test_copy_constructor)
{
    int64_t val1 = 123456789.9;
    variant var1(val1);
    variant var2(var1);
    BOOST_CHECK(value_types::integer_t == var1.type_id());
    BOOST_CHECK(value_types::integer_t == var2.type_id());
    BOOST_CHECK(var2.integer_data_cast()->val_ == val1);

    uint64_t val3 = 123456789.9;
    variant var3(val3);
    variant var4(var3);
    BOOST_CHECK(value_types::uinteger_t == var3.type_id());
    BOOST_CHECK(value_types::uinteger_t == var4.type_id());
    BOOST_CHECK(var4.uinteger_data_cast()->val_ == val3);

    double val5 = 123456789.9;
    variant var5(val5,0);
    variant var6(var5);
    BOOST_CHECK(value_types::double_t == var5.type_id());
    BOOST_CHECK(value_types::double_t == var6.type_id());
    BOOST_CHECK(var6.double_data_cast()->val_ == val5);

    std::string val9 = "Small string";
    variant var9(val9.data(),val9.length());
    variant var10(var9);
    BOOST_CHECK(value_types::small_string_t == var9.type_id());
    BOOST_CHECK(value_types::small_string_t == var10.type_id());
    BOOST_CHECK(var10.small_string_data_cast()->data_ == val9);

    bool val11 = true;
    variant var11(val11);
    variant var12(var11);
    BOOST_CHECK(value_types::bool_t == var11.type_id());
    BOOST_CHECK(value_types::bool_t == var12.type_id());
    BOOST_CHECK(var12.bool_data_cast()->val_ == val11);

    std::string val13 = "Too long for small string";
    variant var13(val13.data(),val13.length());
    variant var14(var13);
    BOOST_CHECK(value_types::string_t == var13.type_id());
    BOOST_CHECK(value_types::string_t == var14.type_id());
    BOOST_CHECK(var14.string_data_cast()->data() == val13);

    object val15 = { {"first",1},{"second",2} };
    variant var15(val15);
    variant var16(var15);
    BOOST_CHECK(value_types::object_t == var15.type_id());
    BOOST_CHECK(value_types::object_t == var16.type_id());
    BOOST_CHECK(val15 == *(var16.object_data_cast()->data_));

    array val17 = {1,2,3,4};
    variant var17(val17);
    variant var18(var17);
    BOOST_CHECK(value_types::array_t == var17.type_id());
    BOOST_CHECK(value_types::array_t == var18.type_id());
    BOOST_CHECK(val17 == *(var18.array_data_cast()->data_));
}

BOOST_AUTO_TEST_CASE(test_equals)
{
    variant var1;
    variant var2{ object() };
    BOOST_CHECK(var1 == var1 && var2 == var2);
    BOOST_CHECK(var1 == var2 && var2 == var1);

    variant var3;
    BOOST_CHECK(var3 == var1 && var1 == var3);
    variant var4{ object({{"first",1},{"second",2}}) };
    variant var5{ object({ { "first",1 },{ "second",2 } }) };
    BOOST_CHECK(var3 != var4 && var4 != var3);
    BOOST_CHECK(var2 != var4 && var4 != var2);
    BOOST_CHECK(var4 == var4);
    BOOST_CHECK(var4 == var5);
    BOOST_CHECK(var5 == var4);

    variant var6(int64_t(100));
    variant var7(uint64_t(100));
    BOOST_CHECK(var6 == var7 && var7 == var6);

    variant var8(100.0);
    BOOST_CHECK(var8 == var8 && var6 == var8 && var8 == var6 && var7 == var8 && var8 == var7);

    std::string val9("small string");
    std::string val11("small string 2");
    variant var9(val9.data(),val9.length());
    variant var10(val9.data(),val9.length());
    variant var11(val11.data(),val11.length());

    std::string val12("too long for small string");
    std::string val14("too long for small string 2");
    variant var12(val12.data(),val12.length());
    variant var13(val12.data(),val12.length());
    variant var14(val14.data(),val14.length());
    BOOST_CHECK(var9 == var10 && var10 == var9);
    BOOST_CHECK(var9 != var11 && var11 != var9);
    BOOST_CHECK(var12 == var13 && var13 == var12);
    BOOST_CHECK(var12 != var14 && var14 != var12);

}

BOOST_AUTO_TEST_SUITE_END()

