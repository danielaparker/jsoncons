// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_UBJSON_UBJSON_OPTIONS_HPP
#define JSONCONS_EXT_UBJSON_UBJSON_OPTIONS_HPP

#include <cwchar>

namespace jsoncons { 
namespace ubjson {

class ubjson_options;

class ubjson_options_common
{
    friend class ubjson_options;

    int max_nesting_depth_{1024};
protected:
    ubjson_options_common() = default;
    ubjson_options_common(const ubjson_options_common&) = default;
    virtual ~ubjson_options_common() = default;
public:
    int max_nesting_depth() const 
    {
        return max_nesting_depth_;
    }
};

class ubjson_decode_options : public virtual ubjson_options_common
{
    friend class ubjson_options;
    std::size_t max_items_{1 << 24};
public:
    ubjson_decode_options() = default;
    ubjson_decode_options(const ubjson_decode_options& other) = default;
protected:
    ubjson_decode_options& operator=(const ubjson_decode_options& other) = default;
public:
    std::size_t max_items() const
    {
        return max_items_;
    }
};

class ubjson_encode_options : public virtual ubjson_options_common
{
    friend class ubjson_options;
public:
    ubjson_encode_options() = default;
    ubjson_encode_options(const ubjson_encode_options& other) = default;
protected:
    ubjson_encode_options& operator=(const ubjson_encode_options& other) = default;
};

class ubjson_options final : public ubjson_decode_options, public ubjson_encode_options
{
public:
    using ubjson_options_common::max_nesting_depth;
    using ubjson_decode_options::max_items;

    ubjson_options() = default;
    ubjson_options(const ubjson_options& other) = default;

    ubjson_options& operator=(const ubjson_options& other) = default;

    ubjson_options& max_nesting_depth(int value)
    {
        this->max_nesting_depth_ = value;
        return *this;
    }

    ubjson_options& max_items(std::size_t value)
    {
        this->max_items_ = value;
        return *this;
    }
};

} // namespace ubjson
} // namespace jsoncons

#endif // JSONCONS_EXT_UBJSON_UBJSON_OPTIONS_HPP
