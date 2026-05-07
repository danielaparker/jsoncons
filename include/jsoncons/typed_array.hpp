// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_TYPED_ARRAY_HPP
#define JSONCONS_TYPED_ARRAY_HPP

#include <array> // std::array
#include <cstddef>
#include <cstdint>
#include <functional> // std::function
#include <ios>
#include <system_error>

#include <jsoncons/utility/write_number.hpp>
#include <jsoncons/json_parser.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/ser_utils.hpp>
#include <jsoncons/sink.hpp>
#include <jsoncons/utility/bigint.hpp>
#include <jsoncons/utility/conversion.hpp>

namespace jsoncons {

enum class typed_array_element_types
{
    uint8 = 1,
    uint16 = 2,
    uint32 = 4,
    uint64 = 8,
    int8 = 16,
    int16 = 32,
    int32 = 64,
    int64 = 128, 
    half_float = 256, 
    float32 = 512, 
    float64 = 1024
};

template <typename T>
jsoncons::span<T> typed_array_cast(jsoncons::span<uint8_t> bytes);

template <> inline
jsoncons::span<uint8_t> typed_array_cast<uint8_t>(jsoncons::span<uint8_t> bytes)
{
    return bytes;
}

template <> inline
jsoncons::span<uint16_t> typed_array_cast<uint16_t>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<uint16_t>{reinterpret_cast<uint16_t*>(bytes.data()), bytes.size()/sizeof(uint16_t)};
}

template <> inline
jsoncons::span<uint32_t> typed_array_cast<uint32_t>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<uint32_t>{reinterpret_cast<uint32_t*>(bytes.data()), bytes.size()/sizeof(uint32_t)};
}

template <> inline
jsoncons::span<uint64_t> typed_array_cast<uint64_t>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<uint64_t>{reinterpret_cast<uint64_t*>(bytes.data()), bytes.size()/sizeof(uint64_t)};
}

template <> inline
jsoncons::span<int8_t> typed_array_cast<int8_t>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<int8_t>{reinterpret_cast<int8_t*>(bytes.data()), bytes.size()};
}

template <> inline
jsoncons::span<int16_t> typed_array_cast<int16_t>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<int16_t>{reinterpret_cast<int16_t*>(bytes.data()), bytes.size()/sizeof(int16_t)};
}

template <> inline
jsoncons::span<int32_t> typed_array_cast<int32_t>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<int32_t>{reinterpret_cast<int32_t*>(bytes.data()), bytes.size()/sizeof(int32_t)};
}

template <> inline
jsoncons::span<int64_t> typed_array_cast<int64_t>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<int64_t>{reinterpret_cast<int64_t*>(bytes.data()), bytes.size()/sizeof(int64_t)};
}

template <> inline
jsoncons::span<float> typed_array_cast<float>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<float>{reinterpret_cast<float*>(bytes.data()), bytes.size()/sizeof(float)};
}

template <> inline
jsoncons::span<double> typed_array_cast<double>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<double>{reinterpret_cast<double*>(bytes.data()), bytes.size()/sizeof(double)};
}

template <> inline
jsoncons::span<const uint8_t> typed_array_cast<const uint8_t>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<const uint8_t>{reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size()};
}

template <> inline
jsoncons::span<const uint16_t> typed_array_cast<const uint16_t>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<const uint16_t>{reinterpret_cast<const uint16_t*>(bytes.data()), bytes.size()/sizeof(uint16_t)};
}

template <> inline
jsoncons::span<const uint32_t> typed_array_cast<const uint32_t>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<const uint32_t>{reinterpret_cast<const uint32_t*>(bytes.data()), bytes.size()/sizeof(uint32_t)};
}

template <> inline
jsoncons::span<const uint64_t> typed_array_cast<const uint64_t>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<const uint64_t>{reinterpret_cast<const uint64_t*>(bytes.data()), bytes.size()/sizeof(uint64_t)};
}

template <> inline
jsoncons::span<const int8_t> typed_array_cast<const int8_t>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<const int8_t>{reinterpret_cast<const int8_t*>(bytes.data()), bytes.size()};
}

template <> inline
jsoncons::span<const int16_t> typed_array_cast<const int16_t>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<const int16_t>{reinterpret_cast<const int16_t*>(bytes.data()), bytes.size()/sizeof(int16_t)};
}

template <> inline
jsoncons::span<const int32_t> typed_array_cast<const int32_t>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<const int32_t>{reinterpret_cast<const int32_t*>(bytes.data()), bytes.size()/sizeof(int32_t)};
}

template <> inline
jsoncons::span<const int64_t> typed_array_cast<const int64_t>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<const int64_t>{reinterpret_cast<const int64_t*>(bytes.data()), bytes.size()/sizeof(int64_t)};
}

template <> inline
jsoncons::span<const float> typed_array_cast<const float>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<const float>{reinterpret_cast<const float*>(bytes.data()), bytes.size()/sizeof(float)};
}

template <> inline
jsoncons::span<const double> typed_array_cast<const double>(jsoncons::span<uint8_t> bytes)
{
    return jsoncons::span<const double>{reinterpret_cast<const double*>(bytes.data()), bytes.size()/sizeof(double)};
}

class slice
{
    static constexpr size_t npos = size_t(-1);

    size_t start_;
    size_t stop_;
    size_t step_;
public:
    constexpr explicit slice(size_t start = npos, size_t stop = npos, size_t step=1)
        : start_(start), stop_(stop), step_(step)
    {
    }

    slice(const slice& other) = default;
    slice(slice&& other) = default;

    slice& operator=(const slice& other) = default;
    slice& operator=(slice&& other) = default;

    size_t start(size_t origin) const
    {
        return start_ == npos ? origin : start_;
    }

    size_t stop(size_t origin, size_t n) const
    {
        return stop_ == npos ? (origin+n) : stop_;
    }
    size_t step() const
    {
        return step_;
    }

    size_t length(size_t origin, size_t n) const
    {
        size_t y = stop(origin, n);
        size_t x = start(origin);

        assert(y >= x);
        size_t w = y - x;
        return w/step_ + (w % step_ != 0);
    }
};

enum class typed_array_layout_kind {row_major, column_major};

struct row_major_layout
{
    std::vector<std::size_t> operator()(jsoncons::span<const std::size_t> extents) const
    {
        std::vector<std::size_t> strides(extents.size(), 0);

        std::size_t size = 1;
        const size_t num_extents = extents.size();
        for (size_t i = 0; i < num_extents; ++i)
        {
            strides[num_extents-i-1] = size;
            size *= extents[num_extents-i-1];
        }

        return strides;
    }
};

struct column_major_layout
{
    std::vector<std::size_t> operator()(jsoncons::span<const std::size_t> extents) const
    {
        std::vector<std::size_t> strides(extents.size(), 0);

        std::size_t size = 1;
        const size_t num_extents = extents.size();
        for (size_t i = 0; i < num_extents; ++i)
        {
            strides[i] = size;
            size *= extents[i];
        }

        return strides;
    }
};

class typed_array_iterator
{
public:
    virtual ~typed_array_iterator() = default;

    virtual bool done() const = 0;

    virtual void next(typed_array_visitor& visitor, const ser_context& context, 
        std::error_code& ec) = 0;
protected:
    template <typename ValueType>
    static typename std::enable_if<ext_traits::is_signed_integer<ValueType>::value, void>::type
        write_value(ValueType val, semantic_tag tag, typed_array_visitor& visitor,
            const ser_context& context, std::error_code& ec)
    {
        visitor.int64_value(val, tag, context, ec);
    }

    template <typename ValueType>
    static typename std::enable_if<ext_traits::is_unsigned_integer<ValueType>::value, void>::type
        write_value(ValueType val, semantic_tag tag, typed_array_visitor& visitor,
            const ser_context& context, std::error_code& ec)
    {
        visitor.uint64_value(val, tag, context, ec);
    }

    template <typename ValueType>
    static typename std::enable_if<std::is_floating_point<ValueType>::value, void>::type
        write_value(ValueType val, semantic_tag tag, typed_array_visitor& visitor,
            const ser_context& context, std::error_code& ec)
    {
        visitor.double_value(val, tag, context, ec);
    }
};

template <typename ValueType, typename Func=jsoncons::identity>
class typed_array_span_iterator : public typed_array_iterator
{
    jsoncons::span<ValueType> data_;
    semantic_tag tag_;
    Func func_;
    bool first_{true};
    bool done_{false};
    std::size_t index_{0};
public:
    typed_array_span_iterator(jsoncons::span<ValueType> data, 
        semantic_tag tag = semantic_tag{}, Func func = Func())
        : data_(data), tag_(tag), func_(func)
    {
    }
    bool done() const final
    {
        return done_;
    }

    void next(typed_array_visitor& visitor, const ser_context& context, 
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
};

template <typename ValueType>
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
    std::vector<mdarray_dimension<ValueType>> dimensions_;
    semantic_tag tag_{};
    std::size_t dim_{0};
    bool first_{true};
    bool done_{false};
public:
    template <typename Layout= jsoncons::row_major_layout>
    mdarray_iterator(jsoncons::span<ValueType> data, const std::vector<std::size_t>& extents,
        Layout layout = Layout())
        : data_{data}, dimensions_(extents.size(), mdarray_dimension<ValueType>{})
    {
        std::vector<std::size_t> strides = layout(extents);
        for (std::size_t i = 0; i < strides.size(); ++i)
        {
            dimensions_[i].extent = extents[i];
            dimensions_[i].stride = strides[i];
            dimensions_[i].index = 0;
            dimensions_[i].end = strides[i]*extents[i];
        }
    }

    bool done() const final
    {
        return done_;
    }

    void next(typed_array_visitor& visitor, const ser_context& context, 
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

} // namespace jsoncons

#endif // JSONCONS_TYPED_ARRAY_HPP

