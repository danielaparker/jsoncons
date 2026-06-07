// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_CBOR_TYPED_ARRAY_ITERATOR_HPP
#define JSONCONS_EXT_CBOR_TYPED_ARRAY_ITERATOR_HPP

#include <bitset> // std::bitset
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <system_error>
#include <utility> // std::move
#include <vector>

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/item_event_visitor.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/typed_array.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/semantic_tag.hpp>
#include <jsoncons/ser_utils.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/utility/binary.hpp>
#include <jsoncons/utility/unicode_traits.hpp>

#include <jsoncons_ext/cbor/cbor_detail.hpp>
#include <jsoncons_ext/cbor/cbor_error.hpp>
#include <jsoncons_ext/cbor/cbor_options.hpp>

namespace jsoncons { 
namespace cbor {

template <typename Source,typename Allocator>
class basic_cbor_parser;

class typed_array_iterator
{
public:
    virtual ~typed_array_iterator() = default;

    virtual std::size_t count() const = 0; 

    virtual bool done() const = 0;

    virtual void next(item_event_visitor& visitor, const ser_context& context, 
        std::error_code& ec) = 0;
protected:
    template <typename ValueType>
    static typename std::enable_if<ext_traits::is_signed_integer<ValueType>::value, void>::type
        write_value(ValueType val, semantic_tag tag, item_event_visitor& visitor,
            const ser_context& context, std::error_code& ec)
    {
        visitor.int64_value(val, tag, context, ec);
    }

    template <typename ValueType>
    static typename std::enable_if<ext_traits::is_unsigned_integer<ValueType>::value, void>::type
        write_value(ValueType val, semantic_tag tag, item_event_visitor& visitor,
            const ser_context& context, std::error_code& ec)
    {
        visitor.uint64_value(val, tag, context, ec);
    }

    template <typename ValueType>
    static typename std::enable_if<std::is_floating_point<ValueType>::value, void>::type
        write_value(ValueType val, semantic_tag tag, item_event_visitor& visitor,
            const ser_context& context, std::error_code& ec)
    {
        visitor.double_value(val, tag, context, ec);
    }
};

template <typename ValueType, typename Func=jsoncons::identity>
class oned_typed_array_iterator : public typed_array_iterator
{
    jsoncons::span<ValueType> data_;
    semantic_tag tag_;
    Func func_;
    bool first_{true};
    bool done_{false};
    std::size_t index_{0};
public:
    oned_typed_array_iterator(jsoncons::span<ValueType> data, 
        semantic_tag tag = semantic_tag{}, Func func = Func())
        : data_(data), tag_(tag), func_(func)
    {
    }
    bool done() const final
    {
        return done_;
    }

    void next(item_event_visitor& visitor, const ser_context& context, 
        std::error_code& ec) final
    {
        if (JSONCONS_UNLIKELY(first_))
        {
            visitor.begin_array(data_.size(), tag_, context, ec);
            first_ = false;
        }
        else if (JSONCONS_LIKELY(index_ < data_.size()))
        {
            this->write_value(func_(data_[index_]), semantic_tag::none, visitor, context, ec);
            ++index_;
        }
        else if (!done_)
        {
            visitor.end_array(context, ec);
            done_ = true;
        }
    }

    std::size_t count() const final 
    {
        return index_;
    }
};

struct mdarray_dimension
{
    std::size_t extent;
    std::size_t end{0};
    std::size_t stride{0};
    std::size_t index{0};
};

template <typename ValueType>
class mdarray_iterator : public typed_array_iterator
{
    jsoncons::span<ValueType> data_;
    std::vector<mdarray_dimension> dimensions_;
    semantic_tag tag_{};
    std::size_t dim_{0};
    bool first_{true};
    bool done_{false};
    std::size_t count_{0};
public:
    mdarray_iterator(jsoncons::span<ValueType> data, jsoncons::span<const std::size_t> extents,
        mdarray_order order = mdarray_order::row_major)
        : data_{data}, dimensions_(extents.size(), mdarray_dimension{})
    {
        std::vector<std::size_t> strides(extents.size(), 0);
        if (order == mdarray_order::column_major)
        {
            std::size_t stride = 1;
            const size_t num_extents = extents.size();
            for (size_t i = 0; i < num_extents; ++i)
            {
                strides[i] = stride;
                stride *= extents[i];
            }
        }
        else
        {
            std::size_t stride = 1;
            const size_t num_extents = extents.size();
            for (size_t i = 0; i < num_extents; ++i)
            {
                strides[num_extents-i-1] = stride;
                stride *= extents[num_extents-i-1];
            }
        }
        for (std::size_t i = 0; i < strides.size(); ++i)
        {
            dimensions_[i].extent = extents[i];
            dimensions_[i].stride = strides[i];
            dimensions_[i].index = 0;
            dimensions_[i].end = strides[i]*extents[i];
        }
    }

    std::size_t count() const final 
    {
        return count_;
    }

    bool done() const final
    {
        return done_;
    }

    void next(item_event_visitor& visitor, const ser_context& context, 
        std::error_code& ec) final
    {
        JSONCONS_ASSERT(!dimensions_.empty());

        if (dim_ == 0)
        {
            if (first_)
            {
                visitor.begin_array(dimensions_[dim_].extent, tag_, context, ec);
                first_ = false;
                return;
            }
            if (dimensions_[dim_].index == dimensions_[dim_].end)
            {
                visitor.end_array(context, ec);
                done_ = true;
                return;
            }
        }
        if (dim_+1 < dimensions_.size() && dimensions_[dim_].index < dimensions_[dim_].end)
        {
            visitor.begin_array(dimensions_[dim_].extent, semantic_tag::none, context, ec);
            ++dim_;
            return;
        }
        if (dimensions_[dim_].index < dimensions_[dim_].end)
        {
            this->write_value(data_[dimensions_[dim_].index], semantic_tag::none, visitor, context, ec);
            dimensions_[dim_].index += dimensions_[dim_].stride;
            ++count_;
            return;
        }
        if (dimensions_[dim_].index + dimensions_[dim_].stride >= dimensions_[dim_].end)
        {
            visitor.end_array(context, ec);
            if (dim_ > 0)
            {
                --dim_;
                dimensions_[dim_].index += dimensions_[dim_].stride;
                if (dimensions_[dim_].index < dimensions_[dim_].end)
                {
                    for (std::size_t i = dim_+1; i < dimensions_.size(); ++i)
                    {
                        dimensions_[i].index = dimensions_[i-1].index;
                        dimensions_[i].end = dimensions_[i].index + dimensions_[i].stride*dimensions_[i].extent;
                    }
                }
            }
        }
    }
};

template <typename Source, typename Allocator>
class cbor_mdarray_row_major_iterator : public typed_array_iterator
{
private:

    std::vector<mdarray_dimension> dimensions_;
    semantic_tag tag_{};
    std::size_t dim_{0};
    bool first_{true};
    bool done_{false};
    std::size_t count_{0};
    basic_cbor_parser<Source,Allocator>* parser_;
    bool cursor_mode_;
public:
    cbor_mdarray_row_major_iterator(jsoncons::span<const std::size_t> extents,
        basic_cbor_parser<Source,Allocator>* parser, bool cursor_mode)
        : dimensions_(extents.size(), mdarray_dimension{}),
          parser_(parser), cursor_mode_(cursor_mode)
    {
        std::vector<std::size_t> strides(extents.size(), 0);
        std::size_t stride = 1;
        const size_t num_extents = extents.size();
        for (size_t i = 0; i < num_extents; ++i)
        {
            strides[num_extents - i - 1] = stride;
            stride *= extents[num_extents - i - 1];
        }
        for (std::size_t i = 0; i < strides.size(); ++i)
        {
            dimensions_[i].extent = extents[i];
            dimensions_[i].stride = strides[i];
            dimensions_[i].index = 0;
            dimensions_[i].end = strides[i] * extents[i];
        }
    }

    bool done() const final
    {
        return done_;
    }

    std::size_t count() const final 
    {
        return count_;
    }

    void next(item_event_visitor& visitor, const ser_context& context, 
        std::error_code& ec) final
    {
        JSONCONS_ASSERT(!dimensions_.empty());

        if (dim_ == 0)
        {
            if (first_)
            {
                visitor.begin_array(dimensions_[dim_].extent, tag_, context, ec);
                first_ = false;
                return;
            }
            if (dimensions_[dim_].index == dimensions_[dim_].end)
            {
                visitor.end_array(context, ec);
                done_ = true;
                return;
            }
        }
        if (dim_+1 < dimensions_.size() && dimensions_[dim_].index < dimensions_[dim_].end)
        {
            if (!cursor_mode_)
            {
                visitor.begin_array(dimensions_[dim_].extent, semantic_tag::none, context, ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
            }
            ++dim_;
            return;
        }
        if (dimensions_[dim_].index < dimensions_[dim_].end)
        {
            parser_->read_item(visitor, ec);
            dimensions_[dim_].index += dimensions_[dim_].stride;
            ++count_;
            return;
        }
        if (dimensions_[dim_].index + dimensions_[dim_].stride >= dimensions_[dim_].end)
        {
            if (!cursor_mode_)
            {
                visitor.end_array(context, ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
            }
            if (JSONCONS_UNLIKELY(ec))
            {
                return;
            }
            if (dim_ > 0)
            {
                --dim_;
                dimensions_[dim_].index += dimensions_[dim_].stride;
                if (dimensions_[dim_].index < dimensions_[dim_].end)
                {
                    for (std::size_t i = dim_+1; i < dimensions_.size(); ++i)
                    {
                        dimensions_[i].index = dimensions_[i-1].index;
                        dimensions_[i].end = dimensions_[i].index + dimensions_[i].stride*dimensions_[i].extent;
                    }
                }
            }
        }
    }
};

template <typename Source, typename Allocator>
class cbor_mdarray_column_major_iterator  : public typed_array_iterator
{
private:

    std::vector<mdarray_dimension> dimensions_;
    semantic_tag tag_{};
    std::size_t dim_{0};
    bool first_{true};
    bool done_{false};
    std::size_t count_{0};
    basic_cbor_parser<Source,Allocator>* parser_;
    bool cursor_mode_;
public:
    cbor_mdarray_column_major_iterator(jsoncons::span<const std::size_t> extents,
        basic_cbor_parser<Source,Allocator>* parser, bool cursor_mode)
        : dimensions_(extents.size(), mdarray_dimension{}),
          parser_(parser), cursor_mode_(cursor_mode)
    {
        std::vector<std::size_t> strides(extents.size(), 0);
        std::size_t stride = 1;
        const size_t num_extents = extents.size();
        for (size_t i = 0; i < num_extents; ++i)
        {
            strides[num_extents - i - 1] = stride;
            stride *= extents[num_extents - i - 1];
        }
        for (std::size_t i = 0; i < strides.size(); ++i)
        {
            dimensions_[i].extent = extents[i];
            dimensions_[i].stride = strides[i];
            dimensions_[i].index = 0;
            dimensions_[i].end = strides[i] * extents[i];
        }
    }

    bool done() const final
    {
        return done_;
    }

    std::size_t count() const final
    {
        return count_;
    }

    void next(item_event_visitor& visitor, const ser_context& context, 
        std::error_code& ec) final 
    {
        JSONCONS_ASSERT(!dimensions_.empty());

        if (dim_ == 0)
        {
            if (first_)
            {
                if (!cursor_mode_)
                {
                    visitor.begin_array(dimensions_[dim_].extent, semantic_tag::multi_dim_column_major, context, ec);
                    visitor.begin_array(dimensions_[dim_].extent, semantic_tag::none, context, ec);
                    for (auto item : dimensions_)
                    {
                        visitor.uint64_value(item.extent, semantic_tag::none, context, ec);
                    }
                    visitor.end_array(context, ec);
                }
                visitor.begin_array(dimensions_[dim_].extent, tag_, context, ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                first_ = false;
                return;
            }
            if (dimensions_[dim_].index == dimensions_[dim_].end)
            {
                visitor.end_array(context, ec);
                if (!cursor_mode_)
                {
                    visitor.end_array(context, ec);
                }
                done_ = true;
                return;
            }
        }
        if (dim_+1 < dimensions_.size() && dimensions_[dim_].index < dimensions_[dim_].end)
        {
            ++dim_;
            return;
        }
        if (dimensions_[dim_].index < dimensions_[dim_].end)
        {
            parser_->read_item(visitor, ec);
            dimensions_[dim_].index += dimensions_[dim_].stride;
            ++count_;
            return;
        }
        if (dimensions_[dim_].index + dimensions_[dim_].stride >= dimensions_[dim_].end)
        {
            if (JSONCONS_UNLIKELY(ec))
            {
                return;
            }
            if (dim_ > 0)
            {
                --dim_;
                dimensions_[dim_].index += dimensions_[dim_].stride;
                if (dimensions_[dim_].index < dimensions_[dim_].end)
                {
                    for (std::size_t i = dim_+1; i < dimensions_.size(); ++i)
                    {
                        dimensions_[i].index = dimensions_[i-1].index;
                        dimensions_[i].end = dimensions_[i].index + dimensions_[i].stride*dimensions_[i].extent;
                    }
                }
            }
        }
    }
};

} // namespace cbor
} // namespace jsoncons

#endif
