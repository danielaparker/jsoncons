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
    uint8=1,uint16,uint32,uint64,
    int8,int16,int32,int64, 
    half_float, float32, float64
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

} // namespace jsoncons

#endif // JSONCONS_TYPED_ARRAY_HPP

