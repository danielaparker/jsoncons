// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_VIEW_HPP
#define JSONCONS_JSON_VIEW_HPP

#include <functional> // std::reference_wrapper
#include <type_traits> // std::remove_const

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_array.hpp>
#include <jsoncons/json_object.hpp>
#include <jsoncons/json_type.hpp>

namespace jsoncons { 

template <typename Json>
class json_view
{
public:
    using json_type = Json;
    using key_type = typename Json::key_type;
    using policy_type = typename Json::policy_type;
    using array = typename policy_type::template array<json_view>;
    using object = typename policy_type::template object<key_type,json_view>;
    using char_type = typename Json::char_type;
    using string_view_type = typename Json::string_view_type;

    using const_array_iterator = typename Json::const_array_iterator;
    using const_object_iterator = typename Json::const_object_iterator;

    using const_array_range_type = typename Json::const_array_range_type;
    using const_object_range_type = typename Json::const_object_range_type;
private:
    const Json* ptr_;
public:
    json_view(const Json& j)
        : ptr_(addressof(j))
    {
    }

    json_view(const json_view<Json>& jv)
        : ptr_(jv.ptr_)
    {
    }

    ~json_view() noexcept = default;

    json_view& operator=(const json_view& other)
    {
        if (this != &other)
        {
            ptr_ = other.ptr_;
        }
        return *this;
    }

    json_storage_kind storage_kind() const noexcept
    {
        return ptr_->storage_kind();
    }

    bool empty() const noexcept
    {
        return ptr_->empty();
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

    template <typename T,typename... Args>
    bool is(Args&&... args) const noexcept
    {
        return ptr_->template is<T>(std::forward<Args>(args)...);
    }

    bool is_object() const noexcept
    {
        return ptr_->is_object();
    }

    bool is_array() const noexcept
    {
        return ptr_->is_array();
    }
};

} // namespace jsoncons

#endif // JSONCONS_JSON_VIEW_HPP
