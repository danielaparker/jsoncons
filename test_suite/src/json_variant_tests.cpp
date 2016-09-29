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
};

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

struct string_data : public base_data
{
    string_holder* data_;

    string_data(const char_type* s, size_t length, const string_allocator& alloc)
        : base_data(value_types::string_t)
    {
        data_ = create_string_holder(s, length, alloc);
    }
    ~string_data()
    {
        destroy_string_holder(data_->get_allocator(), data_);
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
            break;
        case value_types::uinteger_t:
            break;
        case value_types::bool_t:
            break;
        case value_types::small_string_t:
            break;
        case value_types::string_t:
            break;
        case value_types::array_t:
            break;
        case value_types::object_t:
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

    value_types type_id() const
    {
        return reinterpret_cast<const base_data*>(&data_)->type_id_;
    }

    null_data* null_data_cast()
    {
        return reinterpret_cast<null_data*>(&data_);
    }

    const null_data* null_data_cast() const
    {
        return reinterpret_cast<const null_data*>(&data_);
    }

    empty_object_data* empty_object_data_cast()
    {
        return reinterpret_cast<empty_object_data*>(&data_);
    }

    const empty_object_data* empty_object_data_cast() const
    {
        return reinterpret_cast<const empty_object_data*>(&data_);
    }

    bool_data* bool_data_cast()
    {
        return reinterpret_cast<bool_data*>(&data_);
    }

    const bool_data* bool_data_cast() const
    {
        return reinterpret_cast<const bool_data*>(&data_);
    }

    integer_data* integer_data_cast()
    {
        return reinterpret_cast<integer_data*>(&data_);
    }

    const integer_data* integer_data_cast() const
    {
        return reinterpret_cast<const integer_data*>(&data_);
    }

    uinteger_data* uinteger_data_cast()
    {
        return reinterpret_cast<uinteger_data*>(&data_);
    }

    const uinteger_data* uinteger_data_cast() const
    {
        return reinterpret_cast<const uinteger_data*>(&data_);
    }

    double_data* double_data_cast()
    {
        return reinterpret_cast<double_data*>(&data_);
    }

    const double_data* double_data_cast() const
    {
        return reinterpret_cast<const double_data*>(&data_);
    }

    small_string_data* small_string_data_cast()
    {
        return reinterpret_cast<small_string_data*>(&data_);
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
    variant var1(int64_t(-100));
    variant var2(std::move(var1));
    BOOST_CHECK(value_types::null_t == var1.type_id());
    BOOST_CHECK(value_types::integer_t == var2.type_id());
    BOOST_CHECK(var2.integer_data_cast()->val_ == -100);

    std::string s("Too long for small string");
    variant var3(s.data(),s.length());
    variant var4(std::move(var3));
    BOOST_CHECK(value_types::null_t == var3.type_id());
    BOOST_CHECK(value_types::string_t == var4.type_id());
    BOOST_CHECK(s == var4.string_data_cast()->data_->p_);
    BOOST_CHECK(s.length() == var4.string_data_cast()->data_->length_);

    std::string small("Small string");
    variant var5(small.data(), small.length());
    variant var6(std::move(var5));
    BOOST_CHECK(value_types::null_t == var5.type_id());
    BOOST_CHECK(value_types::small_string_t == var6.type_id());
    BOOST_CHECK(small == var6.small_string_data_cast()->data_);
    BOOST_CHECK(small.length() == var6.small_string_data_cast()->length_);

    double val7 = 123456789.9;
    variant var7(val7,0);
    variant var8(std::move(var7));
    BOOST_CHECK(value_types::null_t == var7.type_id());
    BOOST_CHECK(value_types::double_t == var8.type_id());
    BOOST_CHECK(var8.double_data_cast()->val_ == val7);
}

BOOST_AUTO_TEST_CASE(test_copy_constructor)
{

    double val7 = 123456789.9;
    variant var7(val7,0);
    variant var8(var7);
    BOOST_CHECK(value_types::double_t == var7.type_id());
    BOOST_CHECK(value_types::double_t == var8.type_id());
    BOOST_CHECK(var8.double_data_cast()->val_ == val7);
}

BOOST_AUTO_TEST_SUITE_END()

