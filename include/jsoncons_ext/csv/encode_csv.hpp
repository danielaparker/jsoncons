/// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_CSV_ENCODE_CSV_HPP
#define JSONCONS_EXT_CSV_ENCODE_CSV_HPP

#include <ostream>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/json_exception.hpp>

#include <jsoncons/basic_json.hpp>
#include <jsoncons/reflect/encode_traits.hpp>
#include <jsoncons/ser_util.hpp>
#include <jsoncons/sink.hpp>
#include <jsoncons/utility/more_type_traits.hpp>
#include <jsoncons_ext/csv/csv_encoder.hpp>
#include <jsoncons_ext/csv/csv_options.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>

namespace jsoncons { 
namespace csv {

template <typename T,typename CharContainer>
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
    ext_traits::is_back_insertable_char_container<CharContainer>::value,write_result>::type 
try_encode_csv(const T& j, CharContainer& cont, const basic_csv_encode_options<typename CharContainer::value_type>& options = basic_csv_encode_options<typename CharContainer::value_type>())
{
    using char_type = typename CharContainer::value_type;
    basic_csv_encoder<char_type,jsoncons::string_sink<std::basic_string<char_type>>> encoder(cont,options);
    return j.try_dump(encoder);
}

template <typename T,typename CharContainer>
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
    ext_traits::is_back_insertable_char_container<CharContainer>::value,write_result>::type 
try_encode_csv(const T& val, CharContainer& cont, const basic_csv_encode_options<typename CharContainer::value_type>& options = basic_csv_encode_options<typename CharContainer::value_type>())
{
    using char_type = typename CharContainer::value_type;
    basic_csv_encoder<char_type,jsoncons::string_sink<std::basic_string<char_type>>> encoder(cont,options);
    return reflect::encode_traits<T,char_type>::try_encode(make_alloc_set(), val, encoder);
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::is_basic_json<T>::value,write_result>::type 
try_encode_csv(const T& j, std::basic_ostream<CharT>& os, const basic_csv_encode_options<CharT>& options = basic_csv_encode_options<CharT>())
{
    using char_type = CharT;
    basic_csv_encoder<char_type,jsoncons::stream_sink<char_type>> encoder(os,options);
    return j.try_dump(encoder);
}

template <typename T,typename CharT>
typename std::enable_if<!ext_traits::is_basic_json<T>::value,write_result>::type 
try_encode_csv(const T& val, std::basic_ostream<CharT>& os, const basic_csv_encode_options<CharT>& options = basic_csv_encode_options<CharT>())
{
    using char_type = CharT;
    basic_csv_encoder<char_type,jsoncons::stream_sink<char_type>> encoder(os,options);
    return reflect::encode_traits<T>::try_encode(make_alloc_set(), val, encoder);
}

// with aset.get_temp_allocator()ator_arg_t

template <typename T,typename CharContainer,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
    ext_traits::is_back_insertable_char_container<CharContainer>::value,write_result>::type 
try_encode_csv(const allocator_set<Alloc,TempAlloc>& aset,
           const T& j, CharContainer& cont, const basic_csv_encode_options<typename CharContainer::value_type>& options = basic_csv_encode_options<typename CharContainer::value_type>())
{
    using char_type = typename CharContainer::value_type;
    basic_csv_encoder<char_type,jsoncons::string_sink<std::basic_string<char_type>>,TempAlloc> encoder(cont, options, aset.get_temp_allocator());
    return j.try_dump(encoder);
}

template <typename T,typename CharContainer,typename Alloc,typename TempAlloc >
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
    ext_traits::is_back_insertable_char_container<CharContainer>::value,write_result>::type 
try_encode_csv(const allocator_set<Alloc,TempAlloc>& aset,
           const T& val, CharContainer& cont, const basic_csv_encode_options<typename CharContainer::value_type>& options = basic_csv_encode_options<typename CharContainer::value_type>())
{
    using char_type = typename CharContainer::value_type;
    basic_csv_encoder<char_type,jsoncons::string_sink<std::basic_string<char_type>>,TempAlloc> encoder(cont, options, aset.get_temp_allocator());
    return reflect::encode_traits<T,char_type>::try_encode(aset, val, encoder);
}

template <typename T,typename CharT,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value,write_result>::type 
try_encode_csv(const allocator_set<Alloc,TempAlloc>& aset,
    const T& j, std::basic_ostream<CharT>& os, const basic_csv_encode_options<CharT>& options = basic_csv_encode_options<CharT>())
{
    using char_type = CharT;
    basic_csv_encoder<char_type,jsoncons::stream_sink<char_type>,TempAlloc> encoder(os, options, aset.get_temp_allocator());
    return j.try_dump(encoder);
}

template <typename T,typename CharT,typename Alloc,typename TempAlloc >
typename std::enable_if<!ext_traits::is_basic_json<T>::value,write_result>::type 
try_encode_csv(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, std::basic_ostream<CharT>& os, const basic_csv_encode_options<CharT>& options = basic_csv_encode_options<CharT>())
{
    using char_type = CharT;
    basic_csv_encoder<char_type,jsoncons::stream_sink<char_type>,TempAlloc> encoder(os, options, aset.get_temp_allocator());
    return reflect::encode_traits<T>::try_encode(aset, val, encoder);
}

template <typename... Args>
void encode_csv(Args&& ... args)
{
    auto r = try_encode_csv(std::forward<Args>(args)...); 
    if (!r)
    {
        JSONCONS_THROW(ser_error(r.error()));
    }
}

} // namespace csv 
} // namespace jsoncons

#endif // JSONCONS_EXT_CSV_ENCODE_CSV_HPP
