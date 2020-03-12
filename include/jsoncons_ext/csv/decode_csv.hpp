/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CSV_DECODE_CSV_HPP
#define JSONCONS_CSV_DECODE_CSV_HPP

#include <jsoncons_ext/csv/csv_options.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>
#include <jsoncons_ext/csv/csv_encoder.hpp>
#include <jsoncons_ext/csv/csv_cursor.hpp>

namespace jsoncons { 
namespace csv {

    template <class T,class CharT>
    typename std::enable_if<is_basic_json_class<T>::value,T>::type 
    decode_csv(const std::basic_string<CharT>& s, const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>())
    {
        typedef CharT char_type;

        json_decoder<T> decoder;

        basic_csv_reader<char_type,jsoncons::string_source<char_type>> reader(s,decoder,options);
        reader.read();
        return decoder.get_result();
    }

    template <class T,class CharT>
    typename std::enable_if<!is_basic_json_class<T>::value,T>::type 
    decode_csv(const std::basic_string<CharT>& s, const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>())
    {
        basic_csv_cursor<CharT> cursor(s, options);
        jsoncons::json_decoder<basic_json<CharT>> decoder;

        std::error_code ec;
        T val = deser_traits<T>::deserialize(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

    template <class T,class CharT>
    typename std::enable_if<is_basic_json_class<T>::value,T>::type 
    decode_csv(std::basic_istream<CharT>& is, const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>())
    {
        typedef CharT char_type;

        json_decoder<T> decoder;

        basic_csv_reader<char_type,jsoncons::stream_source<char_type>> reader(is,decoder,options);
        reader.read();
        return decoder.get_result();
    }

    template <class T,class CharT>
    typename std::enable_if<!is_basic_json_class<T>::value,T>::type 
    decode_csv(std::basic_istream<CharT>& is, const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>())
    {
        basic_csv_cursor<CharT> cursor(is, options);
        jsoncons::json_decoder<basic_json<CharT>> decoder;

        std::error_code ec;
        T val = deser_traits<T>::deserialize(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

    // With leading allocator parameter

    template <class T,class CharT,class TempAllocator>
    typename std::enable_if<is_basic_json_class<T>::value,T>::type 
    decode_csv(temp_allocator_arg_t, const TempAllocator& temp_alloc,
               const std::basic_string<CharT>& s, 
               const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>())
    {
        typedef CharT char_type;

        json_decoder<T,TempAllocator> decoder(temp_alloc);

        basic_csv_reader<char_type,jsoncons::string_source<char_type>,TempAllocator> reader(s,decoder,options,temp_alloc);
        reader.read();
        return decoder.get_result();
    }

    template <class T,class CharT,class TempAllocator>
    typename std::enable_if<!is_basic_json_class<T>::value,T>::type 
    decode_csv(temp_allocator_arg_t, const TempAllocator& temp_alloc,
               const std::basic_string<CharT>& s, 
               const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>())
    {
        basic_csv_cursor<CharT,stream_source<CharT>,TempAllocator> cursor(s, options, temp_alloc);
        json_decoder<basic_json<CharT,sorted_policy,TempAllocator>,TempAllocator> decoder(temp_alloc, temp_alloc);

        std::error_code ec;
        T val = deser_traits<T>::deserialize(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

    template <class T,class CharT,class TempAllocator>
    typename std::enable_if<is_basic_json_class<T>::value,T>::type 
    decode_csv(temp_allocator_arg_t, const TempAllocator& temp_alloc,
               std::basic_istream<CharT>& is, 
               const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>())
    {
        typedef CharT char_type;

        json_decoder<T,TempAllocator> decoder(temp_alloc);

        basic_csv_reader<char_type,jsoncons::string_source<char_type>,TempAllocator> reader(is,decoder,options,temp_alloc);
        reader.read();
        return decoder.get_result();
    }

    template <class T,class CharT,class TempAllocator>
    typename std::enable_if<!is_basic_json_class<T>::value,T>::type 
    decode_csv(temp_allocator_arg_t, const TempAllocator& temp_alloc,
               std::basic_istream<CharT>& is, 
               const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>())
    {
        basic_csv_cursor<CharT,stream_source<CharT>,TempAllocator> cursor(is, options, temp_alloc);
        json_decoder<basic_json<CharT,sorted_policy,TempAllocator>,TempAllocator> decoder(temp_alloc, temp_alloc);

        std::error_code ec;
        T val = deser_traits<T>::deserialize(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

} // namespace csv 
} // namespace jsoncons

#endif
