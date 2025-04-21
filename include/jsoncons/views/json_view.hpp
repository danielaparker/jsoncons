/*==============================================================================
 Copyright (c) 2020 YaoYuan <ibireme@gmail.com>
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 *============================================================================*/

/** 
 @file json_ref.hpp
 @date 2019-03-09
 @author YaoYuan
 */

#ifndef JSONCONS_JSON_VIEW_HPP
#define JSONCONS_JSON_VIEW_HPP

#include <jsoncons/views/json_ref.hpp>
#include <jsoncons/views/write_json.hpp>

namespace jsoncons {

class json_view;

template <class JsonView>
class key_value_pair
{
public:
    using key_type = JsonView;
    using value_type = JsonView;
    using reference = JsonView;
private:

    std::string_view key_;
    const json_ref* value_ptr_;
public:
    template <std::size_t N>
    decltype(auto) get() const {
    if      constexpr (N == 0) return key();
    else if constexpr (N == 1) return value();
    }
    
    constexpr key_value_pair() noexcept
    {
    }

    constexpr key_value_pair(const json_ref* key, const json_ref* value) noexcept
        : key_(key->get_string_view()), value_ptr_(value)
    {
    }

    constexpr key_value_pair(const key_value_pair& other) = default;

    constexpr const std::string_view& key() const
    {
        return key_;
    }

    constexpr reference value() const
    {
        return value_type(value_ptr_);
    }

    /*constexpr void swap(key_value_pair& other) noexcept
    {
        key_.swap(other.key_);
        value_ptr_->swap(other.value_ptr_);
    }*/

    constexpr key_value_pair& operator=(const key_value_pair& other) = default;

    friend bool operator==(const key_value_pair& lhs, const key_value_pair& rhs) noexcept
    {
        return lhs.key_ == rhs.key_ && lhs.value_ptr_ == rhs.value_ptr_;
    }

    friend bool operator!=(const key_value_pair& lhs, const key_value_pair& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    friend bool operator<(const key_value_pair& lhs, const key_value_pair& rhs) noexcept
    {
        if (lhs.key_ < rhs.key_)
        {
            return true;
        }
        if (lhs.key_ == rhs.key_ && lhs.value_ptr_ < rhs.value_ptr_)
        {
            return true;
        }
        return false;
    }

    friend bool operator<=(const key_value_pair& lhs, const key_value_pair& rhs) noexcept
    {
        return !(rhs < lhs);
    }

    friend bool operator>(const key_value_pair& lhs, const key_value_pair& rhs) noexcept
    {
        return !(lhs <= rhs);
    }

    friend bool operator>=(const key_value_pair& lhs, const key_value_pair& rhs) noexcept
    {
        return !(lhs < rhs);
    }

    friend constexpr void swap(key_value_pair& a, key_value_pair& b) noexcept(
        noexcept(std::declval<key_value_pair&>().swap(std::declval<key_value_pair&>()))) 
    {
        a.swap(b);
    }
};

} // jsoncons

namespace std
{
    template <typename T>
    class tuple_size<jsoncons::key_value_pair<T>>
        : public std::integral_constant<std::size_t, 2> {};

    template <typename T> struct tuple_element<0, jsoncons::key_value_pair<T>> { using type = std::string_view; };
    template <typename T> struct tuple_element<1, jsoncons::key_value_pair<T>> { using type = T; };

}  // namespace std

namespace jsoncons {

template <class IteratorT, typename ConstIteratorT>
class range 
{
public:
    using iterator = IteratorT;
    using const_iterator = ConstIteratorT;
    using reverse_iterator = std::reverse_iterator<IteratorT>;
    using const_reverse_iterator = std::reverse_iterator<ConstIteratorT>;
private:
    iterator first_;
    iterator last_;
public:
    constexpr range(const IteratorT& first, const IteratorT& last) noexcept
        : first_(first), last_(last)
    {
    }

    constexpr iterator begin() const noexcept
    {
        return first_;
    }
    constexpr iterator end() const noexcept
    {
        return last_;
    }
    constexpr const_iterator cbegin() const noexcept
    {
        return first_;
    }
    constexpr const_iterator cend() const noexcept
    {
        return last_;
    }
    constexpr reverse_iterator rbegin() const noexcept
    {
        return reverse_iterator(last_);
    }
    constexpr reverse_iterator rend() const noexcept
    {
        return reverse_iterator(first_);
    }
    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return reverse_iterator(last_);
    }
    constexpr const_reverse_iterator crend() const noexcept
    {
        return reverse_iterator(first_);
    }
};

template <class JsonView>
class const_object_iter
{    
public:
    friend class json_view;

    using iterator_category = std::forward_iterator_tag;

    using value_type = key_value_pair<JsonView>;
    using pointer = const value_type*;
    using reference = value_type;
private:
    const json_ref* obj_; 
    std::size_t size_;
    std::size_t index_; 
    const json_ref* current_;

public:
    constexpr const_object_iter() noexcept : obj_(nullptr), size_(0), index_(0), current_(nullptr) 
    { 
    }

    constexpr explicit const_object_iter(const json_ref* obj, bool end = false) noexcept
        : obj_(obj), index_(0) 
    {
        size_ = obj_->size();
        if (end)
        {
            index_ = size_;
        }
        else
        {
            current_ = unsafe_get_first(obj_);
        };
    }

    constexpr const_object_iter(const const_object_iter&) noexcept = default;
    constexpr const_object_iter& operator=(const const_object_iter&) noexcept = default;

    constexpr reference operator*() const 
    {
        return value_type{current_, current_ != nullptr ? (current_ + 1) : nullptr};
    }
        
    class proxy
    {
        value_type value;
    public:
         constexpr proxy(value_type value) noexcept : value(std::move(value)) {}
         constexpr pointer operator->() const
         {
             return &value;
         }
    };    

    constexpr proxy operator->() const noexcept
    {
        return **this;
    }

    constexpr const_object_iter& operator++() noexcept
    {
        if (index_ < size_)
        {
            current_ = unsafe_get_next(current_);
            current_ = unsafe_get_next(current_);
            ++index_;
        }
        return *this;
    }

    constexpr const_object_iter operator++(int) noexcept
    {
        const_object_iter temp = *this;
        ++*this;
        return temp;
    }

    constexpr bool operator==(const const_object_iter& rhs) const noexcept
    {
        return obj_ == rhs.obj_ && index_ == rhs.index_;
    }

    constexpr bool operator!=(const const_object_iter& rhs) const noexcept
    {
        return !(*this == rhs);
    }
private:
    static constexpr const json_ref* unsafe_get_first(const json_ref* obj) 
    {
        return obj + 1;
    }

    static constexpr const json_ref* unsafe_get_next(const json_ref* val)
    {
        size_t index = val->is_container() ? val->uni.index : 1;
        return val + index;
    }
};

template <class JsonView>
class const_array_iter
{    
public:
    friend class json_view;
    
    using iterator_category = std::forward_iterator_tag;

    using value_type = JsonView;
    using pointer = const JsonView*;
    using reference = JsonView;

private:
    const json_ref* arr_; 
    std::size_t size_;
    std::size_t index_; 
    const json_ref* current_;

public:

    class proxy
    {
        value_type value;
    public:
        constexpr proxy(value_type value) noexcept : value(value) {}
        constexpr pointer operator->() const
        {
            return &value;
        }
    };
    constexpr const_array_iter() noexcept : arr_(nullptr), size_(0), index_(0), current_(nullptr) 
    { 
    }

    explicit constexpr const_array_iter(const json_ref* root, bool end = false) noexcept
        : arr_(root), index_(0), current_(nullptr)
    {
        size_ = arr_->size();
        if (end)
        {
            index_ = size_;
        }
        else if (size_ > 0)
        {
            current_ = unsafe_get_first(arr_);
        }
    }

    constexpr const_array_iter(const const_array_iter&) noexcept = default;
    constexpr const_array_iter& operator=(const const_array_iter&) noexcept = default;

    constexpr reference operator*() const noexcept
    {
        return json_view(current_);
    }

    constexpr proxy operator->() const noexcept
    {
        return **this;
    }

    constexpr const_array_iter& operator++() noexcept
    {
        if (index_ < size_)
        {
            current_ = unsafe_get_next(current_);
            ++index_;
        }
        return *this;
    }

    constexpr const_array_iter operator++(int) noexcept
    {
        const_array_iter temp = *this;
        ++*this;
        return temp;
    }

    bool operator==(const const_array_iter& rhs) const noexcept
    {
        return arr_ == rhs.arr_ && index_ == rhs.index_;
    }

    bool operator!=(const const_array_iter& rhs) const noexcept
    {
        return !(*this == rhs);
    }
    
private:

    static constexpr const json_ref* unsafe_get_first(const json_ref* obj) noexcept
    {
        return obj + 1;
    }

    static constexpr const json_ref* unsafe_get_next(const json_ref* val) noexcept
    {
        size_t index = val->is_container() ? val->uni.index : 1;
        return val + index;
    }
};

class json_view 
{   
public:
    const json_ref* element_;

    using reference = json_view;
    using const_reference = json_view;
    using const_object_iterator = const_object_iter<json_view>;
    using const_array_iterator = const_array_iter<json_view>;
    using const_object_range_type = range<const_object_iterator, const_object_iterator>;
    using const_array_range_type = range<const_array_iterator, const_array_iterator>;
public:
    constexpr json_view() noexcept
        : element_(nullptr)
    {
    }
    constexpr json_view(const json_view& other) noexcept
        : element_(other.element_) 
    {
    }
    constexpr json_view(const json_ref* element) noexcept
        : element_(element)
    {
    }

    constexpr json_view& operator=(const json_view& other) noexcept
    {
        if (this != &other)
        {
            element_ = other.element_;
        }
        return *this;
    }

    constexpr json_type type() const
    {
        return element_->type();
    }

    constexpr semantic_tag tag() const
    {
        return element_->tag();
    }

    constexpr json_view at(std::size_t index) const
    {
        const json_ref* val = unsafe_get(index);
        if (JSONCONS_LIKELY(val))
        {
            return json_view(val);
        }
        throw std::out_of_range("Out of range");
    }
    
    constexpr json_view operator[](std::size_t index) const
    {
        const json_ref* val = unsafe_get(index);
        if (JSONCONS_LIKELY(val))
        {
            return json_view(val);
        }
        throw std::out_of_range("Out of range");
    }

    constexpr json_view at(std::string_view name) const
    {
        const json_ref* val = unsafe_object_get(name);
        if (JSONCONS_LIKELY(val))
        {
            return json_view(val);
        }
        throw std::out_of_range("Out of range");
    }

    constexpr json_view operator[](std::string_view key) const
    {
        const json_ref* val = unsafe_object_get(key);
        if (JSONCONS_LIKELY(val))
        {
            return json_view(val);
        }
        throw std::out_of_range("Out of range");
    }

    constexpr std::size_t size() const noexcept
    {
        return element_->size();
    }

    constexpr bool empty() const noexcept
    {
        return size() == 0;
    }
    
    constexpr bool is_null() const noexcept
    {
        return type() == json_type::null_value;
    }

    constexpr bool is_true() const noexcept
    {
        return type() == json_type::bool_value && element_->get_bool();
    }

    constexpr bool is_false() const noexcept
    {
        return type() == json_type::bool_value && !element_->get_bool();
    }

    constexpr bool is_bool() const noexcept
    {
        return type() == json_type::bool_value;
    }

    constexpr bool is_uint64() const noexcept
    {
        return type() == json_type::uint64_value;
    }

    constexpr bool is_int64() const noexcept
    {
        return type() == json_type::int64_value;
    }

    constexpr bool is_double() const noexcept
    {
        return type() == json_type::double_value;
    }

    constexpr bool is_number() const noexcept
    {
        return type() == json_type::uint64_value || type() == json_type::int64_value || type() == json_type::double_value;
    }

    constexpr bool is_integer() const noexcept
    {
        return type() == json_type::uint64_value || type() == json_type::int64_value;
    }
    
    constexpr bool is_string() const noexcept
    {
        return type() == json_type::string_value;
    }

    constexpr bool is_array() const noexcept
    {
        return type() == json_type::array_value;
    }

    constexpr bool is_object() const noexcept
    {
        return type() == json_type::object_value;
    }
    
    constexpr bool is_container() const noexcept
    {
        return element_->is_container();
    }

    constexpr bool is_flat() const noexcept
    {
        return element_->is_flat();
    }
    
    constexpr const_object_range_type object_range() const
    {
        switch (type())
        {
            case json_type::object_value:
                return const_object_range_type(const_object_iterator(element_), const_object_iterator(element_, true));
            default:
                throw std::domain_error("Not an object");
        }
    }

    constexpr const_array_range_type array_range() const
    {
        switch (type())
        {
            case json_type::array_value:
                return const_array_range_type(const_array_iterator(element_), const_array_iterator(element_, true));
            default:
                throw std::domain_error("Not an array");
        }
    }

    constexpr bool get_bool() const
    {
        return element_->get_bool();
    }
    
    constexpr std::string_view get_string_view() const
    {
        return element_->get_string_view();
    }

    constexpr const char* get_cstring() const
    {
        return element_->get_cstring();
    }

    template <typename T>
    constexpr T cast() const
    {
        return element_->cast<T>();
    }
    
    constexpr double get_double() const
    {
        return element_->cast<double>();
    }

    constexpr double get_number() const
    {
        return element_->get_number();
    }
    
    constexpr json_view front() const
    {
        switch (type())
        {
            case json_type::array_value:
                if (JSONCONS_LIKELY(size() > 0)) 
                {
                    return json_view(const_array_iter<json_view>::unsafe_get_first(element_));
                }
                else
                {
                    throw std::out_of_range("Out of range");
                }
                break;
            case json_type::object_value:
                if (JSONCONS_LIKELY(size() > 0)) 
                {
                    const json_ref* first = const_object_iter<json_view>::unsafe_get_first(element_);
                    return json_view(first + 1);
                }
                else
                {
                    throw std::out_of_range("Out of range");
                }
                break;
            default:
                throw std::domain_error("Not an array or object");
                break;
        }
    }

    constexpr json_view back() const
    {
        switch (type())
        {
            case json_type::array_value:
                if (JSONCONS_LIKELY(size() > 0)) 
                {
                    const json_ref* first = const_array_iter<json_view>::unsafe_get_first(element_);
                    if (is_flat()) 
                    {
                        return json_view(first + (size() - 1));
                    } 
                    else 
                    {
                        for (std::size_t len = size(); len-- > 1;) 
                        {
                            first = const_array_iter<json_view>::unsafe_get_next(first);
                        }
                        return json_view(first);
                    }
                }
                else
                {
                    throw std::out_of_range("Out of range");
                }
                break;
            case json_type::object_value:
                if (JSONCONS_LIKELY(size() > 0)) 
                {
                    const json_ref* first = const_object_iter<json_view>::unsafe_get_first(element_);
                    if (is_flat()) 
                    {
                        return json_view(first + 2*(size() - 1));
                    } 
                    else 
                    {
                        for (std::size_t len = size(); len-- > 1;) 
                        {
                            first = const_object_iter<json_view>::unsafe_get_next(first);
                        }
                        return json_view(first+1);
                    }
                }
                else
                {
                    throw std::out_of_range("Out of range");
                }
                break;
            default:
                throw std::domain_error("Not an array or object");
                break;
        }
    }
    
    constexpr bool contains(std::string_view key) const noexcept
    {
        if (JSONCONS_LIKELY(is_object() && !empty()))
        {
            auto it = find(key);
            return it != object_range().end() ? true : false; 
        }
        else
        {
            return false;
        }
    }

    constexpr const_object_iterator find(std::string_view name) const noexcept
    {
        return find(object_range().begin(), name);
    }

    constexpr const_object_iterator find(const_object_iterator iter, std::string_view name) const noexcept
    {
        if (JSONCONS_LIKELY(is_object() && !empty()))
        {
            auto end = object_range().end();
            const char* key = name.data();
            std::size_t key_len = name.size();
            size_t index = iter.index_;
            if (JSONCONS_UNLIKELY(index == iter.size_)) 
            {
                index = 0;
                iter = object_range().begin();
            }
            while (iter != end) 
            {
                if (iter->key() == key) 
                {
                    return iter;
                }
                ++iter;
                if (iter == end && index > 0) 
                {
                    index = 0;
                    iter = object_range().begin();
                }
            }
            return end;
        }
        return const_object_iterator{};
    }
    
    friend
    bool operator==(const json_view& lhs, std::string_view str) noexcept
    {
        return lhs.equal_string(str.data(), str.size());
    }

    friend
    bool operator!=(const json_view& lhs, std::string_view str) noexcept
    {
        return !lhs.equal_string(str.data(), str.size());
    }

    friend
    bool operator==(const json_view& lhs, const json_view& rhs) noexcept
    {
        return lhs.equal(rhs);
    }

    friend
    bool operator!=(const json_view& lhs, const json_view& rhs) noexcept
    {
        return !lhs.equal(rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const json_view& val)
    {
        std::string buffer;
        write_json(val, buffer);
        os << buffer;
        return os;
    }

private:
    bool equal(const json_view& other) const; 
    
    constexpr const json_ref* unsafe_get(std::size_t index) const
    {
        if (JSONCONS_LIKELY(size() > index)) 
        {
            const json_ref *val = const_array_iter<json_view>::unsafe_get_first(element_);
            if (is_flat()) 
            {
                return val + index;
            } 
            else 
            {
                while (index-- > 0) val = const_array_iter<json_view>::unsafe_get_next(val); // this is issue
                return val;
            }
        }
        return nullptr;
    }

    constexpr const json_ref* unsafe_object_get(std::string_view name) const
    {
        size_t len = size();
        const char* name_data = name.data();
        std::size_t name_len = name.size();
        const json_ref* key = const_object_iter<json_view>::unsafe_get_first(element_);
        while (len-- > 0) {
            if (key->equal_string(name_data, name_len)) 
            {
                return key + 1;
            }
            key = const_object_iter<json_view>::unsafe_get_next(key + 1);
        }

        return nullptr;
    }

    constexpr bool equal_string(std::string_view sv) const noexcept
    {
        return equal_string(sv.data(), sv.size());
    }

    constexpr bool equal_string(const char* str, std::size_t len) const noexcept
    {
        return element_->equal_string(str, len);
    }
    
    constexpr bool equal_num(const json_view& rhs) const noexcept
    {
        return element_->equal_num(*(rhs.element_));
    }
};

} // jsoncons

#endif // JSONCONS_JSON_VIEW_HPP
