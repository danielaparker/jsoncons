// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_STAJ_ITERATOR_HPP
#define JSONCONS_STAJ_ITERATOR_HPP

#include <new> // placement new
#include <memory>
#include <string>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <iterator> // std::input_iterator_tag
#include <jsoncons/json_exception.hpp>
#include <jsoncons/staj_cursor.hpp>
#include <jsoncons/basic_json.hpp>
#include <jsoncons/deser_traits.hpp>

namespace jsoncons {

    template<class Json, class T=Json>
    class staj_array_iterator
    {
        using char_type = typename Json::char_type;

        basic_staj_cursor<char_type>* reader_;
        optional<T> value_;
        json_decoder<Json> decoder_;
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::input_iterator_tag;

        staj_array_iterator() noexcept
            : reader_(nullptr)
        {
        }

        staj_array_iterator(basic_staj_cursor<char_type>& reader)
            : reader_(std::addressof(reader))
        {
            if (reader_->current().event_type() == staj_event_type::begin_array)
            {
                reader_->next(); // skip past array
                next();
            }
            else
            {
                reader_ = nullptr;
            }
        }

        staj_array_iterator(basic_staj_cursor<char_type>& reader,
                            std::error_code& ec)
            : reader_(std::addressof(reader))
        {
            if (reader_->current().event_type() == staj_event_type::begin_array)
            {
                reader_->next(ec); // skip past array
                if (ec) {reader_ = nullptr;}
                next(ec);
                if (ec) {reader_ = nullptr;}
            }
            else
            {
                reader_ = nullptr;
            }
        }

        staj_array_iterator(const staj_array_iterator& other)
            : reader_(other.reader_), value_(other.value_)
        {
        }

        staj_array_iterator(staj_array_iterator&& other) noexcept
            : reader_(nullptr), value_(std::move(other.value_))
        {
            std::swap(reader_,other.reader_);
        }

        ~staj_array_iterator() noexcept
        {
        }

        staj_array_iterator& operator=(const staj_array_iterator& other)
        {
            reader_ = other.reader_;
            value_ = other.value;
            return *this;
        }

        staj_array_iterator& operator=(staj_array_iterator&& other) noexcept
        {
            std::swap(reader_,other.reader_);
            value_.swap(other.value_);
            return *this;
        }

        const T& operator*() const
        {
            return *value_;
        }

        const T* operator->() const
        {
            return value_.operator->();
        }

        staj_array_iterator& operator++()
        {
            next();
            return *this;
        }

        staj_array_iterator& increment(std::error_code& ec)
        {
            next(ec);
            if (ec) {reader_ = nullptr;}
            return *this;
        }

        staj_array_iterator operator++(int) // postfix increment
        {
            staj_array_iterator temp(*this);
            next();
            return temp;
        }

        friend bool operator==(const staj_array_iterator<Json,T>& a, const staj_array_iterator<Json,T>& b)
        {
            return (!a.reader_ && !b.reader_)
                || (!a.reader_ && b.done())
                || (!b.reader_ && a.done());
        }

        friend bool operator!=(const staj_array_iterator<Json,T>& a, const staj_array_iterator<Json,T>& b)
        {
            return !(a == b);
        }

    private:

        bool done() const
        {
            return reader_->done() || reader_->current().event_type() == staj_event_type::end_array;
        }


        void next()
        {
            using char_type = typename Json::char_type;

            if (!done())
            {
                std::error_code ec;
                value_ = deser_traits<T,char_type>::deserialize(*reader_, decoder_, ec);
                if (ec)
                {
                    JSONCONS_THROW(ser_error(ec, reader_->context().line(), reader_->context().column()));
                }
            }
        }

        void next(std::error_code& ec)
        {
            using char_type = typename Json::char_type;

            if (!done())
            {
                value_ = deser_traits<T,char_type>::deserialize(*reader_, decoder_, ec);
            }
        }
    };

    template <class Json, class T>
    staj_array_iterator<Json,T> begin(staj_array_iterator<Json,T> iter) noexcept
    {
        return iter;
    }

    template <class Json, class T>
    staj_array_iterator<Json,T> end(const staj_array_iterator<Json,T>&) noexcept
    {
        return staj_array_iterator<Json,T>();
    }

    template <class Json, class T=Json>
    class staj_object_iterator
    {
    public:
        using char_type = typename Json::char_type;
        using key_type = std::basic_string<char_type>;
        using value_type = std::pair<key_type,T>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::input_iterator_tag;

    private:
        basic_staj_cursor<char_type>* reader_;
        optional<value_type> key_value_;
        json_decoder<Json> decoder_;
    public:

        staj_object_iterator() noexcept
            : reader_(nullptr)
        {
        }

        staj_object_iterator(basic_staj_cursor<char_type>& reader)
            : reader_(std::addressof(reader))
        {
            if (reader_->current().event_type() == staj_event_type::begin_object)
            {
                reader_->next(); // advance past begin_object
                next();
            }
            else
            {
                reader_ = nullptr;
            }
        }

        staj_object_iterator(basic_staj_cursor<char_type>& reader, 
                             std::error_code& ec)
            : reader_(std::addressof(reader))
        {
            if (reader_->current().event_type() == staj_event_type::begin_object)
            {
                reader_->next(ec); // advance past begin_object
                if (ec) {reader_ = nullptr;}
                next(ec);
                if (ec) {reader_ = nullptr;}
            }
            else
            {
                reader_ = nullptr;
            }
        }

        staj_object_iterator(const staj_object_iterator& other)
            : reader_(other.reader_), key_value_(other.key_value_)
        {
        }

        staj_object_iterator(staj_object_iterator&& other) noexcept
            : reader_(other.reader_), key_value_(std::move(other.key_value_))
        {
        }

        ~staj_object_iterator() noexcept
        {
        }

        staj_object_iterator& operator=(const staj_object_iterator& other)
        {
            reader_ = other.reader_;
            key_value_ = other.key_value_;
            return *this;
        }

        staj_object_iterator& operator=(staj_object_iterator&& other) noexcept
        {
            std::swap(reader_,other.reader_);
            std::swap(key_value_,other.key_value_);
            return *this;
        }

        const value_type& operator*() const
        {
            return *key_value_;
        }

        const value_type* operator->() const
        {
            return key_value_.operator->();
        }

        staj_object_iterator& operator++()
        {
            next();
            return *this;
        }

        staj_object_iterator& increment(std::error_code& ec)
        {
            next(ec);
            if (ec)
            {
                reader_ = nullptr;
            }
            return *this;
        }

        staj_object_iterator operator++(int) // postfix increment
        {
            staj_object_iterator temp(*this);
            next();
            return temp;
        }

        friend bool operator==(const staj_object_iterator<Json,T>& a, const staj_object_iterator<Json,T>& b)
        {
            return (!a.reader_ && !b.reader_)
                   || (!a.reader_ && b.done())
                   || (!b.reader_ && a.done());
        }

        friend bool operator!=(const staj_object_iterator<Json,T>& a, const staj_object_iterator<Json,T>& b)
        {
            return !(a == b);
        }

    private:

        bool done() const
        {
            return reader_->done() || reader_->current().event_type() == staj_event_type::end_object;
        }

        void next()
        {
            using char_type = typename Json::char_type;

            //reader_->next();
            if (!done())
            {
                JSONCONS_ASSERT(reader_->current().event_type() == staj_event_type::key);
                key_type key = reader_->current().template get<key_type>();
                reader_->next();
                if (!done())
                {
                    std::error_code ec;
                    key_value_ = value_type(std::move(key),deser_traits<T,char_type>::deserialize(*reader_, decoder_, ec));
                    if (ec)
                    {
                        JSONCONS_THROW(ser_error(ec, reader_->context().line(), reader_->context().column()));
                    }
                }
            }
        }

        void next(std::error_code& ec)
        {
            using char_type = typename Json::char_type;

            if (!done())
            {
                JSONCONS_ASSERT(reader_->current().event_type() == staj_event_type::key);
                auto key = reader_->current().template get<key_type>();
                reader_->next(ec);
                if (ec)
                {
                    return;
                }
                if (!done())
                {
                    key_value_ = value_type(std::move(key),deser_traits<T,char_type>::deserialize(*reader_, decoder_, ec));
                }
            }
        }
    };

    template<class Json, class T>
    staj_object_iterator<Json,T> begin(staj_object_iterator<Json,T> iter) noexcept
    {
        return iter;
    }

    template<class Json, class T>
    staj_object_iterator<Json,T> end(const staj_object_iterator<Json,T>&) noexcept
    {
        return staj_object_iterator<Json,T>();
    }

    template <class T, class CharT>
    typename std::enable_if<is_basic_json<T>::value,staj_array_iterator<T,T>>::type
    make_array_iterator(basic_staj_cursor<CharT>& reader)
    {
        return staj_array_iterator<T,T>(reader);
    }

    template <class T, class CharT>
    typename std::enable_if<!is_basic_json<T>::value,staj_array_iterator<basic_json<CharT>,T>>::type
    make_array_iterator(basic_staj_cursor<CharT>& reader)
    {
        return staj_array_iterator<basic_json<CharT>,T>(reader);
    }

    template <class T, class CharT>
    typename std::enable_if<is_basic_json<T>::value,staj_array_iterator<T,T>>::type
    make_array_iterator(basic_staj_cursor<CharT>& reader, std::error_code& ec)
    {
        return staj_array_iterator<T,T>(reader, ec);
    }

    template <class T, class CharT>
    typename std::enable_if<!is_basic_json<T>::value,staj_array_iterator<basic_json<CharT>,T>>::type
    make_array_iterator(basic_staj_cursor<CharT>& reader, std::error_code& ec)
    {
        return staj_array_iterator<basic_json<CharT>,T>(reader, ec);
    }

    template <class T, class CharT>
    typename std::enable_if<is_basic_json<T>::value,staj_object_iterator<T,T>>::type
    make_object_iterator(basic_staj_cursor<CharT>& reader)
    {
        return staj_object_iterator<T,T>(reader);
    }

    template <class T, class CharT>
    typename std::enable_if<!is_basic_json<T>::value,staj_object_iterator<basic_json<CharT>,T>>::type
    make_object_iterator(basic_staj_cursor<CharT>& reader)
    {
        return staj_object_iterator<basic_json<CharT>,T>(reader);
    }

    template <class T, class CharT>
    typename std::enable_if<is_basic_json<T>::value,staj_object_iterator<T,T>>::type
    make_object_iterator(basic_staj_cursor<CharT>& reader, std::error_code& ec)
    {
        return staj_object_iterator<T,T>(reader, ec);
    }

    template <class T, class CharT>
    typename std::enable_if<!is_basic_json<T>::value,staj_object_iterator<basic_json<CharT>,T>>::type
    make_object_iterator(basic_staj_cursor<CharT>& reader, std::error_code& ec)
    {
        return staj_object_iterator<basic_json<CharT>,T>(reader, ec);
    }

} // namespace jsoncons

#endif

