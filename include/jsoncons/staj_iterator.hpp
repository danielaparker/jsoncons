// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_STAJ_ITERATOR_HPP
#define JSONCONS_STAJ_ITERATOR_HPP

#include <exception>
#include <ios>
#include <iterator> // std::input_iterator_tag
#include <memory>
#include <new> // placement new
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/basic_json.hpp>
#include <jsoncons/decode_traits.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/staj_event.hpp>
#include <jsoncons/staj_cursor.hpp>

namespace jsoncons {

    // staj_array_view

    template <typename T,typename Json>
    class staj_array_view;

    template <typename T,typename Json>
    class staj_array_iterator
    {
        using char_type = typename Json::char_type;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::input_iterator_tag;
        
    private:
        staj_array_view<T, Json>* view_{nullptr};
        bool done_{false};
        std::exception_ptr eptr_;
    public:

        staj_array_iterator() noexcept
            : done_(true)
        {
        }

        staj_array_iterator(staj_array_view<T, Json>& view)
            : view_(std::addressof(view))
        {
            if (view_->cursor_->done())
            {
                done_ = true;
            }
            else if (view_->cursor_->current().event_type() == staj_event_type::begin_array)
            {
                next();
            }
            else
            {
                done_ = true;
            }
        }

        staj_array_iterator(staj_array_view<T, Json>& view, std::error_code& ec)
            : view_(std::addressof(view))
        {
            if (view_->cursor_->done())
            {
                done_ = true;
            }
            else if (view_->cursor_->current().event_type() == staj_event_type::begin_array)
            {
                next(ec);
                if (JSONCONS_UNLIKELY(ec)) {done_ = true;}
            }
            else
            {
                done_ = true;
            }
        }

        ~staj_array_iterator() noexcept = default;

        bool has_value() const
        {
            return !eptr_;
        }

        const T& operator*() const
        {
            if (JSONCONS_UNLIKELY(eptr_))
            {
                 std::rethrow_exception(eptr_);
            }
            return *view_->value_;
        }

        const T* operator->() const
        {
            if (JSONCONS_UNLIKELY(eptr_))
            {
                 std::rethrow_exception(eptr_);
            }
            return view_->value_.operator->();
        }

        staj_array_iterator& operator++()
        {
            next();
            return *this;
        }

        staj_array_iterator& increment(std::error_code& ec)
        {
            next(ec);
            if (JSONCONS_UNLIKELY(ec)) {done_ = true;}
            return *this;
        }

        staj_array_iterator operator++(int) // postfix increment
        {
            staj_array_iterator temp(*this);
            next();
            return temp;
        }

        friend bool operator==(const staj_array_iterator& a, const staj_array_iterator& b)
        {
            return (a.done() && b.done());
        }

        friend bool operator!=(const staj_array_iterator& a, const staj_array_iterator& b)
        {
            return !(a == b);
        }

    private:

        bool done() const
        {
            return done_;
        }

        void next()
        {
            if (JSONCONS_UNLIKELY(done_))
            {
                return;
            }
            std::error_code ec;
            next(ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, view_->cursor_->context().line(), view_->cursor_->context().column()));
            }
        }

        void next(std::error_code& ec)
        {
            if (JSONCONS_UNLIKELY(done_))
            {
                return;
            }
            if (view_->cursor_->done())
            {
                done_ = true;
                return;
            }
            view_->cursor_->next(ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                done_ = true;
                return;
            }
            if (JSONCONS_UNLIKELY(view_->cursor_->current().event_type() == staj_event_type::end_array))
            {
                done_ = true;
                return;
            }
            eptr_ = std::exception_ptr();
            JSONCONS_TRY
            {
                view_->value_ = decode_traits<T,char_type>::decode(*view_->cursor_, view_->decoder_, ec);
            }
            JSONCONS_CATCH(const conv_error&)
            {
                eptr_ = std::current_exception();
            }           
        }            
    };

    template <typename T,typename Json>
    class staj_array_view
    {
        friend class staj_array_iterator<T, Json>;
    public:
        using char_type = typename Json::char_type;
        using iterator = staj_array_iterator<T, Json>;
    private:
        basic_staj_cursor<char_type>* cursor_;
        json_decoder<Json> decoder_;
        jsoncons::optional<T> value_;
    public:
        staj_array_view(basic_staj_cursor<char_type>& cursor) 
            : cursor_(std::addressof(cursor))
        {
        }

        iterator begin()
        {
            return staj_array_iterator<T, Json>(*this);
        }

        iterator end()
        {
            return staj_array_iterator<T, Json>();
        }
    };

    // staj_object_view
    
    template <typename Key,typename Json,typename T=Json>
    class staj_object_view;

    template <typename Key,typename T,typename Json>
    class staj_object_iterator
    {
        using char_type = typename Json::char_type;
        using key_type = std::basic_string<char_type>;
        using value_type = std::pair<key_type,T>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::input_iterator_tag;

    private:
        staj_object_view<Key, T, Json>* view_{nullptr};
        bool done_{false};
        std::exception_ptr eptr_;
    public:

        staj_object_iterator() noexcept
            : done_(true)
        {
        }

        staj_object_iterator(staj_object_view<Key, T, Json>& view)
            : view_(std::addressof(view))
        {
            if (view_->cursor_->done())
            {
                done_ = true;
            }
            else if (view_->cursor_->current().event_type() == staj_event_type::begin_object)
            {
                next();
            }
            else
            {
                done_ = true;
            }
        }

        staj_object_iterator(staj_object_view<Key, T, Json>& view, 
            std::error_code& ec)
            : view_(std::addressof(view))
        {
            if (view_->cursor_->done())
            {
                done_ = true;
            }
            else if (view_->cursor_->current().event_type() == staj_event_type::begin_object)
            {
                next(ec);
                if (JSONCONS_UNLIKELY(ec)) {done_ = true;}
            }
            else
            {
                done_ = true;
            }
        }

        ~staj_object_iterator() noexcept = default;

        bool has_value() const
        {
            return !eptr_;
        }

        const value_type& operator*() const
        {
            if (JSONCONS_UNLIKELY(eptr_))
            {
                 std::rethrow_exception(eptr_);
            }
            return *view_->key_value_;
        }

        const value_type* operator->() const
        {
            if (JSONCONS_UNLIKELY(eptr_))
            {
                 std::rethrow_exception(eptr_);
            }
            else
            {
                return view_->key_value_.operator->();
            }
        }

        staj_object_iterator& operator++()
        {
            next();
            return *this;
        }

        staj_object_iterator& increment(std::error_code& ec)
        {
            next(ec);
            if (JSONCONS_UNLIKELY(ec)){done_ = true;}
            return *this;
        }

        staj_object_iterator operator++(int) // postfix increment
        {
            staj_object_iterator temp(*this);
            next();
            return temp;
        }

        friend bool operator==(const staj_object_iterator& a, const staj_object_iterator& b)
        {
            return (a.done() && b.done());
        }

        friend bool operator!=(const staj_object_iterator& a, const staj_object_iterator& b)
        {
            return !(a == b);
        }

    private:

        bool done() const
        {
            return done_;
        }

        void next()
        {
            if (JSONCONS_UNLIKELY(done_))
            {
                return;
            }
            std::error_code ec;
            next(ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, view_->cursor_->context().line(), view_->cursor_->context().column()));
            }
        }

        void next(std::error_code& ec)
        {
            if (JSONCONS_UNLIKELY(done_))
            {
                return;
            }
            if (view_->cursor_->done())
            {
                done_ = true;
                return;
            }
            
            view_->cursor_->next(ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                done_ = true;
                return;
            }
            if (JSONCONS_UNLIKELY(view_->cursor_->current().event_type() == staj_event_type::end_object))
            {
                done_ = true;
                return;
            }
            JSONCONS_ASSERT(view_->cursor_->current().event_type() == staj_event_type::key);
            auto key = view_->cursor_->current(). template get<key_type>();
            view_->cursor_->next(ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                done_ = true;
                return;
            }
            eptr_ = std::exception_ptr();
            JSONCONS_TRY
            {
                view_->key_value_ = value_type(std::move(key),decode_traits<T,char_type>::decode(*view_->cursor_, view_->decoder_, ec));
            }
            JSONCONS_CATCH(const conv_error&)
            {
                eptr_ = std::current_exception();
            }
        }
    };

    // staj_object_view

    template <typename Key,typename T,typename Json>
    class staj_object_view
    {
        friend class staj_object_iterator<Key,T,Json>;
    public:
        using char_type = typename Json::char_type;
        using iterator = staj_object_iterator<Key,T,Json>;
        using key_type = std::basic_string<char_type>;
        using mapped_type = Json;
        using value_type = std::pair<key_type,T>;
    private:
        basic_staj_cursor<char_type>* cursor_;
        json_decoder<Json> decoder_;
        jsoncons::optional<value_type> key_value_;
    public:
        staj_object_view(basic_staj_cursor<char_type>& cursor) 
            : cursor_(std::addressof(cursor))
        {
        }

        iterator begin()
        {
            return staj_object_iterator<Key,T,Json>(*this);
        }

        iterator end()
        {
            return staj_object_iterator<Key,T,Json>();
        }
    };

    template <typename T,typename CharT,typename Json=typename std::conditional<ext_traits::is_basic_json<T>::value,T,basic_json<CharT>>::type>
    staj_array_view<T, Json> staj_array(basic_staj_cursor<CharT>& cursor)
    {
        return staj_array_view<T, Json>(cursor);
    }

    template <typename Key,typename T,typename CharT,typename Json=typename std::conditional<ext_traits::is_basic_json<T>::value,T,basic_json<CharT>>::type>
    staj_object_view<Key, T, Json> staj_object(basic_staj_cursor<CharT>& cursor)
    {
        return staj_object_view<Key, T, Json>(cursor);
    }

} // namespace jsoncons

#endif // JSONCONS_STAJ_ITERATOR_HPP

