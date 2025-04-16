#ifndef JSONCONS2_UTILITY_MORE_CONCEPTS_HPP
#define JSONCONS2_UTILITY_MORE_CONCEPTS_HPP

#include <jsoncons/views/jsoncons_config.hpp>
#include <jsoncons/views/more_type_traits.hpp>
#include <concepts>
#include <span>

namespace jsoncons { namespace utility {

template <typename T>
concept boolean = std::is_same_v<T,bool>;    

template <typename T>
concept signed_integral = std::is_integral_v<T> && std::is_signed_v<T> && !std::is_same_v<T,bool>;    

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T> && !std::is_same_v<T,bool>;    

template <typename T>
concept string_or_string_view = is_string_or_string_view_v<T>;    

template <typename T, typename CharT>
concept string_or_string_view_of = is_string_or_string_view_v<T> && std::is_same_v<CharT,char>;    

template <typename T>
concept string = is_string_v<T>;    

template <typename T>
concept byte_string_view = std::is_same_v<std::span<const typename T::value_type>, T> && sizeof(typename T::value_type) == sizeof(char);    

template <typename T>
concept int_most_64 = std::is_same_v<T,int8_t> || std::is_same_v<T,int16_t> || std::is_same_v<T,int32_t> || std::is_same_v<T,int64_t>;    

template <typename T>
concept uint_most_64 = std::is_same_v<T,uint8_t> || std::is_same_v<T,uint16_t> || std::is_same_v<T,uint32_t> || std::is_same_v<T,uint64_t>;

template <typename T>
concept extended_integral = utility::integer_limits<T>::is_specialized;

template <typename T>
concept signed_extended_integral = utility::integer_limits<T>::is_specialized && utility::integer_limits<T>::is_signed;

template <typename T>
concept unsigned_extended_integral = utility::integer_limits<T>::is_specialized && !utility::integer_limits<T>::is_signed;

template <typename T>
concept trivially_copyable = std::is_trivially_copyable_v<T>;

} // namespace utility
} // namespace jsoncons

#endif // JSONCONS2_JSON_ELEMENT_HPP
