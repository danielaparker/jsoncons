// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_STAJ_ITERATOR_HPP
#define JSONCONS_STAJ_ITERATOR_HPP

#include <memory>
#include <string>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <iterator> // std::input_iterator_tag
#include <jsoncons/json_exception.hpp>
#include <jsoncons/parse_error_handler.hpp>
#include <jsoncons/staj_reader.hpp>

namespace jsoncons {

template<class T, class CharT, class Json=basic_json<CharT>>
class basic_staj_array_iterator
{
    typedef CharT char_type;

    basic_staj_reader<char_type>* reader_;

    class Proxy {  // needed for operator->
       const T x;
       friend class basic_staj_array_iterator;
       Proxy( const T& xx ) : x(xx) {}
    public:
       const T* operator->() const { return &x; }
    };
public:
    typedef T value_type;
    typedef std::ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;
    typedef std::input_iterator_tag iterator_category;

    basic_staj_array_iterator() noexcept
        : reader_(nullptr)
    {
    }

    basic_staj_array_iterator(basic_staj_reader<char_type>& reader)
        : reader_(std::addressof(reader))
    {
        if (reader_->current().event_type() == staj_event_type::begin_array)
        {
            next();
        }
        else
        {
            reader_ = nullptr;
        }
    }

    basic_staj_array_iterator(basic_staj_reader<char_type>& reader,
                        std::error_code& ec)
        : reader_(std::addressof(reader))
    {
        if (reader_->current().event_type() == staj_event_type::begin_array)
        {
            next(ec);
            if (ec)
            {
                reader_ = nullptr;
            }
        }
        else
        {
            reader_ = nullptr;
        }
    }

    const T operator*() const
    {
        return read_from<T>(Json(), *reader_);
    }

    const Proxy operator->() const
    {
        return Proxy(read_from<T>(Json(), *reader_));
    }

    basic_staj_array_iterator& operator++()
    {
        next();
        return *this;
    }

    basic_staj_array_iterator& increment(std::error_code& ec)
    {
        next(ec);
        if (ec)
        {
            reader_ = nullptr;
        }
        return *this;
    }

    basic_staj_array_iterator operator++(int) // postfix increment
    {
        basic_staj_array_iterator temp(*this);
        next();
        return temp;
    }

    friend bool operator==(const basic_staj_array_iterator<T,CharT,Json>& a, const basic_staj_array_iterator<T,CharT,Json>& b)
    {
        return (!a.reader_ && !b.reader_)
            || (!a.reader_ && b.done())
            || (!b.reader_ && a.done());
    }

    friend bool operator!=(const basic_staj_array_iterator<T,CharT,Json>& a, const basic_staj_array_iterator<T,CharT,Json>& b)
    {
        return !(a == b);
    }

private:

    bool done() const
    {
        return reader_->done() || reader_->current().event_type() == staj_event_type::end_array;
    }

    void next();

    void next(std::error_code& ec);
};

template <class T, class CharT, class Json>
basic_staj_array_iterator<T,CharT,Json> begin(basic_staj_array_iterator<T,CharT,Json> iter) noexcept
{
    return iter;
}

template <class T, class CharT, class Json>
basic_staj_array_iterator<T,CharT,Json> end(const basic_staj_array_iterator<T,CharT,Json>&) noexcept
{
    return basic_staj_array_iterator<T,CharT,Json>();
}

template <class T, class CharT, class Json>
class basic_staj_object_iterator
{
public:
    typedef CharT char_type;
    typedef std::basic_string<char_type> key_type;
    typedef std::pair<key_type,T> value_type;
    typedef std::ptrdiff_t difference_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef std::input_iterator_tag iterator_category;

private:
    class Proxy {  // needed for operator->
       const value_type x;
       friend class basic_staj_object_iterator;
       Proxy( const value_type& xx ) : x(xx) {}
    public:
       const value_type* operator->() const { return &x; }
    };

    basic_staj_reader<char_type>* reader_;
    key_type key_;
public:

    basic_staj_object_iterator() noexcept
        : reader_(nullptr)
    {
    }

    basic_staj_object_iterator(basic_staj_reader<char_type>& reader)
        : reader_(std::addressof(reader))
    {
        if (reader_->current().event_type() == staj_event_type::begin_object)
        {
            next();
        }
        else
        {
            reader_ = nullptr;
        }
    }

    basic_staj_object_iterator(basic_staj_reader<char_type>& reader, 
                         std::error_code& ec)
        : reader_(std::addressof(reader))
    {
        if (reader_->current().event_type() == staj_event_type::begin_object)
        {
            next(ec);
            if (ec)
            {
                reader_ = nullptr;
            }
        }
        else
        {
            reader_ = nullptr;
        }
    }

    const value_type operator*() const
    {
        return value_type(key_,read_from<T>(Json(), *reader_));
    }

    const Proxy operator->() const
    {
        return Proxy(value_type(key_,read_from<T>(Json(), *reader_)));
    }

    basic_staj_object_iterator& operator++()
    {
        next();
        return *this;
    }

    basic_staj_object_iterator& increment(std::error_code& ec)
    {
        next(ec);
        if (ec)
        {
            reader_ = nullptr;
        }
        return *this;
    }

    basic_staj_object_iterator operator++(int) // postfix increment
    {
        basic_staj_object_iterator temp(*this);
        next();
        return temp;
    }

    friend bool operator==(const basic_staj_object_iterator<T,CharT,Json>& a, const basic_staj_object_iterator<T,CharT,Json>& b)
    {
        return (!a.reader_ && !b.reader_)
               || (!a.reader_ && b.done())
               || (!b.reader_ && a.done());
    }

    friend bool operator!=(const basic_staj_object_iterator<T,CharT,Json>& a, const basic_staj_object_iterator<T,CharT,Json>& b)
    {
        return !(a == b);
    }

private:

    bool done() const
    {
        return reader_->done() || reader_->current().event_type() == staj_event_type::end_object;
    }

    void next();

    void next(std::error_code& ec);
};

template <class T,class CharT,class Json>
basic_staj_object_iterator<T,CharT,Json> begin(basic_staj_object_iterator<T,CharT,Json> iter) noexcept
{
    return iter;
}

template <class T,class CharT,class Json>
basic_staj_object_iterator<T,CharT,Json> end(const basic_staj_object_iterator<T,CharT,Json>&) noexcept
{
    return basic_staj_object_iterator<T,CharT,Json>();
}

template <class T>
using staj_array_iterator = basic_staj_array_iterator<T,char,json>;

template <class T>
using wstaj_array_iterator = basic_staj_array_iterator<T,wchar_t,wjson>;

template <class T>
using staj_object_iterator = basic_staj_object_iterator<T,char,json>;

template <class T>
using wstaj_object_iterator = basic_staj_object_iterator<T,wchar_t,wjson>;

}

#include <jsoncons/json_conversion_traits.hpp>

namespace jsoncons {

template <class T, class CharT, class Json>
void basic_staj_array_iterator<T,CharT,Json>::next()
{
    if (!done())
    {
        reader_->next();
    }
}

template<class T, class CharT, class Json>
void basic_staj_array_iterator<T,CharT,Json>::next(std::error_code& ec)
{
    if (!done())
    {
        reader_->next(ec);
        if (ec)
        {
            return;
        }
    }
}

template<class T, class CharT, class Json>
void basic_staj_object_iterator<T,CharT,Json>::next()
{
    reader_->next();
    if (!done())
    {
        JSONCONS_ASSERT(reader_->current().event_type() == staj_event_type::name);
        key_ =reader_->current(). template as<key_type>();
        reader_->next();
    }
}

template<class T, class CharT, class Json>
void basic_staj_object_iterator<T,CharT,Json>::next(std::error_code& ec)
{
    reader_->next(ec);
    if (ec)
    {
        return;
    }
    if (!done())
    {
        JSONCONS_ASSERT(reader_->current().event_type() == staj_event_type::name);
        key_ =reader_->current(). template as<key_type>();
        reader_->next(ec);
        if (ec)
        {
            return;
        }
    }
}

}

#endif

