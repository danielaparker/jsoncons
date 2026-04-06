// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_MSGPACK_MSGPACK_OPTIONS_HPP
#define JSONCONS_EXT_MSGPACK_MSGPACK_OPTIONS_HPP

#include <cwchar>

namespace jsoncons { 
namespace msgpack {

class msgpack_options;

class msgpack_options_common
{
    friend class msgpack_options;

    int max_nesting_depth_{1024};
protected:
    msgpack_options_common() = default;
    msgpack_options_common(const msgpack_options_common&) = default;

    virtual ~msgpack_options_common() = default;

    msgpack_options_common& operator=(const msgpack_options_common&) = default;
public:
    int max_nesting_depth() const 
    {
        return max_nesting_depth_;
    }
};

class msgpack_decode_options : public virtual msgpack_options_common
{
    friend class msgpack_options;
public:
    msgpack_decode_options() = default;
    msgpack_decode_options(const msgpack_decode_options& other) = default;
protected:
    msgpack_decode_options& operator=(const msgpack_decode_options& other) = default;
};

class msgpack_encode_options : public virtual msgpack_options_common
{
    friend class msgpack_options;
public:
    msgpack_encode_options() = default;
    msgpack_encode_options(const msgpack_encode_options& other) = default;
protected:
    msgpack_encode_options& operator=(const msgpack_encode_options& other) = default;
};

class msgpack_options final : public msgpack_decode_options, public msgpack_encode_options
{
public:
    using msgpack_options_common::max_nesting_depth;

    msgpack_options() = default;
    msgpack_options(const msgpack_options& other) = default;
    msgpack_options& operator=(const msgpack_options& other) = default;

    msgpack_options& max_nesting_depth(int value)
    {
        this->max_nesting_depth_ = value;
        return *this;
    }
};

} // namespace msgpack
} // namespace jsoncons

#endif // JSONCONS_EXT_MSGPACK_MSGPACK_OPTIONS_HPP
