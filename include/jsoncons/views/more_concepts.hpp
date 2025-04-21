#ifndef JSONCONS_UTILITY_MORE_CONCEPTS_HPP
#define JSONCONS_UTILITY_MORE_CONCEPTS_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/utility/more_type_traits.hpp>
#include <concepts>
#include <span>

namespace jsoncons { 
namespace utility {

template <typename T>
concept boolean = std::is_same_v<T,bool>;    

template <typename T>
concept signed_integral = std::is_integral_v<T> && std::is_signed_v<T> && !std::is_same_v<T,bool>;    

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T> && !std::is_same_v<T,bool>;    

template <typename T>
concept string_or_string_view = ext_traits::is_string_or_string_view<T>::value;    

template <typename T, typename CharT>
concept string_or_string_view_of = ext_traits::is_string_or_string_view<T>::value && std::is_same_v<CharT,char>;    

template <typename T>
concept string = ext_traits::is_string<T>::value;    

template <typename T>
concept byte_string_view = std::is_same_v<std::span<const typename T::value_type>, T> && sizeof(typename T::value_type) == sizeof(char);    

template <typename T>
concept int_most_64 = std::is_same_v<T,int8_t> || std::is_same_v<T,int16_t> || std::is_same_v<T,int32_t> || std::is_same_v<T,int64_t>;    

template <typename T>
concept uint_most_64 = std::is_same_v<T,uint8_t> || std::is_same_v<T,uint16_t> || std::is_same_v<T,uint32_t> || std::is_same_v<T,uint64_t>;

template <typename T>
concept extended_integral = ext_traits::integer_limits<T>::is_specialized;

template <typename T>
concept signed_extended_integral = ext_traits::integer_limits<T>::is_specialized && ext_traits::integer_limits<T>::is_signed;

template <typename T>
concept unsigned_extended_integral = ext_traits::integer_limits<T>::is_specialized && !ext_traits::integer_limits<T>::is_signed;

template <typename T>
concept trivially_copyable = std::is_trivially_copyable_v<T>;

} // namespace utility
} // namespace jsoncons

#endif // JSONCONS_UTILITY_MORE_CONCEPTS_HPP
