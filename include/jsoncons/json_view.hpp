// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_VIEW_HPP
#define JSONCONS_JSON_VIEW_HPP

#include <functional> // std::reference_wrapper
#include <type_traits> // std::remove_const

#include <jsoncons/allocator_set.hpp>
#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/conversion_result.hpp>
#include <jsoncons/json_array.hpp>
#include <jsoncons/json_object.hpp>
#include <jsoncons/json_options.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/semantic_tag.hpp>
#include <jsoncons/utility/byte_string.hpp>

namespace jsoncons { 

template <typename Json>
class json_view
{
public:
    using json_type = Json;
    using key_type = typename json_type::key_type;
    using array = typename json_type::array;
    using object = typename json_type::object;
    using char_type = typename json_type::char_type;
    using char_traits_type = typename json_type::char_traits_type;
    using string_view_type = typename json_type::string_view_type;

    using const_array_iterator = typename json_type::const_array_iterator;
    using const_object_iterator = typename json_type::const_object_iterator;

    using const_array_range_type = typename json_type::const_array_range_type;
    using const_object_range_type = typename json_type::const_object_range_type;
private:
    const json_type* ptr_;
public:
    json_view(const json_type& j)
        : ptr_(addressof(j))
    {
    }

    json_view(const json_view<json_type>& jv)
        : ptr_(jv.ptr_)
    {
    }

    ~json_view() noexcept = default;

    json_view& operator=(const json_view& other) = default;

    void swap(json_view& other) noexcept
    {
        if (this == &other)
        {
            return;
        }
        std::swap(ptr_, other.ptr_);
    }

    json_storage_kind storage_kind() const 
    {
        return ptr_->storage_kind();
    }

    json_type type() const 
    {
        return ptr_->type();
    }

    semantic_tag tag() const
    {
        return ptr_->tag();
    }

    bool empty() const noexcept
    {
        return ptr_->empty();
    }

    std::size_t size() const
    {
        return ptr_->size();
    }

    std::size_t capacity() const noexcept
    {
        return ptr_->capacity();
    }

    const_object_range_type object_range() const
    {
        return ptr_->object_range();
    }

    const_array_range_type array_range() const
    {
        return ptr_->array_range();
    }

    const json_type& operator[](std::size_t i) const
    {
        return *ptr_[i];
    }

    const json_type& operator[](const string_view_type& key) const
    {
        return *ptr_[key];
    }

    const json_type& at(const string_view_type& key) const
    {
        return ptr_->at(key);
    }

    const json_type& at(std::size_t i) const
    {
        return ptr_->at(i);
    }

    const json_type& at_or_null(const string_view_type& key) const
    {
        return ptr_->at_or_null(key);
    }

    template <typename T,typename U>
    T get_value_or(const string_view_type& key, U&& default_value) const
    {
        return ptr_->get_value_or(key, std::forward<U>(default_value));
    }

    const_object_iterator find(const string_view_type& key) const
    {
        return ptr_->find(key);
    }

    template <typename T,typename... Args>
    bool is(Args&&... args) const noexcept
    {
        return ptr_->template is<T>(std::forward<Args>(args)...);
    }

    bool is_null() const
    {
        return ptr_->is_null();
    }

    bool is_bool() const noexcept
    {
        return ptr_->is_bool();
    }

    template <typename T>
    bool is_integer() const noexcept
    {
        return ptr_->template is_integer<T>();
    }

    bool is_int64() const noexcept
    {
        return ptr_->is_int64();
    }

    bool is_uint64() const noexcept
    {
        return ptr_->is_uint64();
    }

    bool is_half() const noexcept
    {
        return ptr_->is_half();
    }

    bool is_double() const noexcept
    {
        return ptr_->is_double();
    }

    bool is_number() const noexcept
    {
        return ptr_->is_number();
    }

    bool is_string() const noexcept
    {
        return ptr_->is_string();
    }

    bool is_string_view() const noexcept
    {
        return ptr_->is_string_view();
    }

    bool is_byte_string() const noexcept
    {
        return ptr_->is_byte_string();
    }

    bool is_byte_string_view() const noexcept
    {
        return ptr_->is_byte_string_view();
    }

    bool is_bignum() const
    {
        return ptr_->is_bignum();
    }

    bool is_object() const noexcept
    {
        return ptr_->is_object();
    }

    bool is_array() const noexcept
    {
        return ptr_->is_array();
    }

    int compare(const json_type& rhs) const noexcept
    {
        return ptr_>compare(rhs);
    }

    template <typename T>
    void dump(T& cont,
         const basic_json_encode_options<char_type>& options = basic_json_options<char_type>()) const
    {
        return ptr_->dump(cont, options);
    }

    template <typename T>
    void dump_pretty(T& cont,
        const basic_json_encode_options<char_type>& options = basic_json_options<char_type>()) const
    {
        return ptr_->dump_pretty(cont, options);
    }

    void dump(std::basic_ostream<char_type>& os,
        const basic_json_encode_options<char_type>& options = basic_json_options<char_type>()) const
    {
        return ptr_->dump(os, options);
    }

    void dump_pretty(std::basic_ostream<char_type>& os,
        const basic_json_encode_options<char_type>& options = basic_json_options<char_type>()) const
    {
        return ptr_->dump_pretty(os, options);
    }

    void dump(basic_json_visitor<char_type>& visitor) const
    {
        return ptr_->dump(visitor);
    }

    write_result try_dump(basic_json_visitor<char_type>& visitor) const
    {
        return ptr_->try_dump(visitor);
    }

    string_view_type as_string_view() const
    {
        return ptr_->as_string_view();
    }

    conversion_result<string_view_type> try_as_string_view() const
    {
        return ptr_->try_as_string_view();
    }
    template <typename T,typename Alloc, typename TempAlloc>
    conversion_result<T> try_as_byte_string(const allocator_set<Alloc,TempAlloc>& aset) const
    {
        return ptr_->try_as_byte_string(aset);
    }

    template <typename Alloc=std::allocator<uint8_t>>
    basic_byte_string<Alloc> as_byte_string() const
    {
        return ptr_->as_byte_string();
    }

    conversion_result<byte_string_view> try_as_byte_string_view() const
    {
        return ptr_->try_as_byte_string_view();
    }

    byte_string_view as_byte_string_view() const
    {
        return ptr_->as_byte_string_view();
    }

    template <typename T>
    T as(byte_string_arg_t, semantic_tag hint) const
    {
        return ptr_->as(byte_string_arg, hint);
    }

    bool as_bool() const 
    {
        return ptr_->as_bool();
    }

    template <typename T>
    conversion_result<T> try_as_integer() const
    {
        return ptr_->try_as_integer();
    }

    template <typename T>
    T as_integer() const
    {
        return ptr_->as_integer();
    }

    double as_double() const
    {
        return ptr_->as_double();
    }

    template <typename T,typename Alloc, typename TempAlloc>
    conversion_result<T> try_as_string(const allocator_set<Alloc,TempAlloc>& aset) const
    {
        return ptr_->try_as_string(aset);
    }

    template <typename CharsAlloc>
    std::basic_string<char_type,char_traits_type,CharsAlloc> as_string(const CharsAlloc& alloc) const 
    {
        return ptr_->as_string(alloc);
    }

    std::basic_string<char_type,char_traits_type> as_string() const 
    {
        return ptr_->as_string();
    }

    const char_type* as_cstring() const
    {
        return ptr_->as_cstring();
    }
};

} // namespace jsoncons

#endif // JSONCONS_JSON_VIEW_HPP
