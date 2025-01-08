// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CHUNK_READER_HPP
#define JSONCONS_CHUNK_READER_HPP

#include <cstddef>
#include <functional>
#include <system_error>

namespace jsoncons { 

template <typename CharT>
class basic_parser_input
{
public:
    using char_type = CharT;
    
    virtual ~basic_parser_input() = default;
    virtual void set_buffer(const CharT* data, std::size_t length) = 0;
};

template <typename CharT>
class chunk_reader
{
public:
    using char_type = CharT;

    virtual ~chunk_reader() = default;
    virtual bool read_chunk(basic_parser_input<char_type>&, std::error_code&)
    {
        return false;
    }
};

template <typename CharT>
class chunk_reader_adaptor : public chunk_reader<CharT>
{
    using char_type = CharT;
    using chunk_reader_type = std::function<bool(basic_parser_input<char_type>& input, std::error_code& ec)>;

    chunk_reader_type read_chunk_;

public:
    chunk_reader_adaptor()
        : read_chunk_([](basic_parser_input<char_type>&, std::error_code&){return false;})
    {
    }
    chunk_reader_adaptor(chunk_reader_type read_chunk)
        : read_chunk_(read_chunk)
    {
    }

    bool read_chunk(basic_parser_input<char_type>& input, std::error_code& ec)
    {
        return read_chunk_(input, ec);
    }
};

using parser_input = basic_parser_input<char>;
using wparser_input = basic_parser_input<wchar_t>;

} // namespace jsoncons

#endif // JSONCONS_CHUNK_READER_HPP
