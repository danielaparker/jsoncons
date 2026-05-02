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

enum class typed_array_element_type
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

struct row_major
{
    static std::vector<std::size_t> calculate_strides(jsoncons::span<const std::size_t> extents)
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

struct column_major
{
    static std::vector<std::size_t> calculate_strides(jsoncons::span<const std::size_t> extents)
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

#if 0
struct row_major
{
    static void calculate_strides(jsoncons::span<const std::size_t> extents, 
        std::vector<std::size_t>& strides, size_t& size)
    {
        size = 1;
        const size_t num_extents = extents.size();
        for (size_t i = 0; i < num_extents; ++i)
        {
            strides[num_extents-i-1] = size;
            size *= extents[num_extents-i-1];
        }
    }
/*
    template <size_t N>
    static void update_offsets(size_t rel, indices_t<N>& offsets)
    {
        offsets[N-1] += rel;
    }
*/
    template <size_t N>
    static std::vector<std::size_t> calculate_offsets(size_t rel,
        jsoncons::span<const std::size_t> strides, 
        jsoncons::span<const std::size_t> slices)
    {
        indices_t<N> offsets;
        offsets[N-1] = rel + slices[N-1].start(Base::origin()) * strides[N-1];
        for (size_t i = 0; i+1 < N; ++i)
        {
            offsets[i] = slices[i].start(Base::origin()) * strides[i]; 
        }
        return offsets;
    }
};

struct column_major
{
    template <size_t N>
    static void calculate_strides(const extents_t<N>& extents, indices_t<N>& strides, size_t& size)
    {
        size = 1;
        for (size_t i = 0; i < N; ++i)
        {
            strides[i] = size;
            size *= extents[i];
        }
    }
/*
    template <size_t N>
    static void update_offsets(size_t rel, indices_t<N>& offsets)
    {
        offsets[0] += rel;
    }
*/
    template <size_t N, typename Base>
    static indices_t<N> calculate_offsets(size_t rel,
        const indices_t<N>& strides, 
        const std::array<slice,N>& slices)
    {
        indices_t<N> offsets;
        offsets[0] = rel + Base::rebase_to_zero(slices[0].start(Base::origin())) * strides[0];
        for (size_t i = 1; i < N; ++i)
        {
            offsets[i] = Base::rebase_to_zero(slices[i].start(Base::origin())) * strides[i]; 
        }
        return offsets;
    }
};
#endif

inline
std::size_t get_offset(const std::vector<std::size_t>& strides, 
    const std::vector<std::size_t>& indices)
{
    size_t offset = 0;

    const std::size_t num_strides = strides.size(); 
    const std::size_t num_indices = indices.size(); 
    JSONCONS_ASSERT(num_indices <= num_strides);

    for (size_t i = 0; i < num_indices; ++i)
    {
        offset += indices[i]*strides[i];
    }

    return offset;
}

} // namespace jsoncons

#endif // JSONCONS_TYPED_ARRAY_HPP

