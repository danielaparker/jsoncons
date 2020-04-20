// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_MORE_TYPE_TRAITS_HPP
#define JSONCONS_DETAIL_MORE_TYPE_TRAITS_HPP

#include <stdexcept>
#include <string>
#include <cmath>
#include <type_traits> // std::enable_if, std::true_type
#include <memory>
#include <iterator> // std::iterator_traits
#include <exception>
#include <array>
#include <utility> // std::declval
#include <jsoncons/config/compiler_support.hpp>

namespace jsoncons
{

#ifndef JSONCONS_HAS_VOID_T
// follows https://en.cppreference.com/w/cpp/types/void_t
template<typename... Ts> struct make_void { typedef void type;};
template<typename... Ts> using void_t = typename make_void<Ts...>::type;
#else
using void_t = std::void_t; 
#endif

// static_max

template <std::size_t arg1, std::size_t ... argn>
struct static_max;

template <std::size_t arg>
struct static_max<arg>
{
    static constexpr size_t value = arg;
};

template <std::size_t arg1, std::size_t arg2, std::size_t ... argn>
struct static_max<arg1,arg2,argn ...>
{
    static constexpr size_t value = arg1 >= arg2 ? 
        static_max<arg1,argn...>::value :
        static_max<arg2,argn...>::value; 
};

inline
char to_hex_character(uint8_t c)
{
    return (char)((c < 10) ? ('0' + c) : ('A' - 10 + c));
}

inline
bool is_control_character(uint32_t c)
{
    return c <= 0x1F || c == 0x7f;
}

inline
bool is_non_ascii_codepoint(uint32_t cp)
{
    return cp >= 0x80;
}

template <typename T>
struct is_stateless
 : public std::integral_constant<bool,  
      (std::is_default_constructible<T>::value &&
      std::is_empty<T>::value)>
{};

// type traits extensions

namespace detail {

    // to_plain_pointer

    template<class Pointer> inline
    typename std::pointer_traits<Pointer>::element_type* to_plain_pointer(Pointer ptr)
    {       
        return (std::addressof(*ptr));
    }

    template<class T> inline
    T * to_plain_pointer(T * ptr)
    {       
        return (ptr);
    }  

    // is_primitive

    template <class T, class Enable=void>
    struct is_primitive : std::false_type {};

    template <class T>
    struct is_primitive<T, 
           typename std::enable_if<std::is_integral<T>::value ||
                                   std::is_floating_point<T>::value
    >::type> : std::true_type {};

    // has_char_traits_member_type

    // is_character

    template <class T, class Enable=void>
    struct is_character : std::false_type {};

    template <class T>
    struct is_character<T, 
           typename std::enable_if<std::is_same<T,char>::value ||
                                   std::is_same<T,wchar_t>::value
    >::type> : std::true_type {};

    // has_char_traits_member_type

    template <class T, class Enable=void>
    struct has_char_traits_member_type : std::false_type {};

    template <class T>
    struct has_char_traits_member_type<T, 
                                       typename std::enable_if<std::is_same<typename T::traits_type::char_type, typename T::value_type>::value
    >::type> : std::true_type {};

    // is_int

    template <class T, class Enable=void>
    struct is_bool : std::false_type {};

    template <class T>
    struct is_bool<T, 
                   typename std::enable_if<std::is_same<T,bool>::value
    >::type> : std::true_type {};

    // is_uint

    template <class T, class Enable=void>
    struct is_u8_u16_u32_or_u64 : std::false_type {};

    template <class T>
    struct is_u8_u16_u32_or_u64<T, 
                                typename std::enable_if<std::is_same<T,uint8_t>::value ||
                                                        std::is_same<T,uint16_t>::value ||
                                                        std::is_same<T,uint32_t>::value ||
                                                        std::is_same<T,uint64_t>::value
    >::type> : std::true_type {};

    // is_int

    template <class T, class Enable=void>
    struct is_i8_i16_i32_or_i64 : std::false_type {};

    template <class T>
    struct is_i8_i16_i32_or_i64<T, 
                                typename std::enable_if<std::is_same<T,int8_t>::value ||
                                                        std::is_same<T,int16_t>::value ||
                                                        std::is_same<T,int32_t>::value ||
                                                        std::is_same<T,int64_t>::value
    >::type> : std::true_type {};

    // is_float_or_double

    template <class T, class Enable=void>
    struct is_float_or_double : std::false_type {};

    template <class T>
    struct is_float_or_double<T, 
                              typename std::enable_if<std::is_same<T,float>::value ||
                                                      std::is_same<T,double>::value
    >::type> : std::true_type {};

    // is_string

    template <class T, class Enable=void>
    struct is_string : std::false_type {};

    template <class T>
    struct is_string<T, 
                     typename std::enable_if<is_character<typename T::value_type>::value &&
                                             has_char_traits_member_type<T>::value && 
                                             !std::is_void<decltype(T::npos)>::value && 
                                             !std::is_void<typename T::allocator_type>::value
    >::type> : std::true_type {};

    // is_string_view

    template <class T, class Enable=void>
    struct is_string_view : std::false_type {};

    template <class T>
    struct is_string_view<T, 
                          typename std::enable_if<is_character<typename T::value_type>::value &&
                                                  has_char_traits_member_type<T>::value && 
                                                  !std::is_void<decltype(T::npos)>::value && !is_string<T>::value
    >::type> : std::true_type {};

    // is_integer_like

    template <class T, class Enable=void>
    struct is_integer_like : std::false_type {};

    template <class T>
    struct is_integer_like<T, 
                           typename std::enable_if<std::is_integral<T>::value && 
                           std::is_signed<T>::value && 
                           !std::is_same<T,bool>::value>::type> : std::true_type {};

    // is_uinteger_like

    template <class T, class Enable=void>
    struct is_uinteger_like : std::false_type {};

    template <class T>
    struct is_uinteger_like<T, 
                            typename std::enable_if<std::is_integral<T>::value && 
                            !std::is_signed<T>::value && 
                            !std::is_same<T,bool>::value>::type> : std::true_type {};

    // is_floating_point_like

    template <class T, class Enable=void>
    struct is_floating_point_like : std::false_type {};

    template <class T>
    struct is_floating_point_like<T, 
                                  typename std::enable_if<std::is_floating_point<T>::value>::type> : std::true_type {};

    // is_map_like

    template <class T, class Enable=void>
    struct is_map_like : std::false_type {};

    template <class T>
    struct is_map_like<T, 
                       typename std::enable_if<!std::is_void<typename T::mapped_type>::value>::type> 
        : std::true_type {};

    // is_std_array
    template<class T>
    struct is_std_array : std::false_type {};

    template<class E, std::size_t N>
    struct is_std_array<std::array<E, N>> : std::true_type {};

    // is_vector_like

    template <class T, class Enable=void>
    struct is_vector_like : std::false_type {};

    template <class T>
    struct is_vector_like<T, 
                          typename std::enable_if<!std::is_void<typename T::value_type>::value &&
                                                  !std::is_void<typename std::iterator_traits<typename T::iterator>::value_type>::value &&
                                                  !is_std_array<T>::value && 
                                                  !has_char_traits_member_type<T>::value && 
                                                  !is_map_like<T>::value 
    >::type> 
        : std::true_type {};

    // is_constructible_from_const_pointer_and_size

    template <class T, class Enable=void>
    struct is_constructible_from_const_pointer_and_size : std::false_type {};

    template <class T>
    struct is_constructible_from_const_pointer_and_size<T, 
        typename std::enable_if<std::is_constructible<T,typename T::const_pointer,typename T::size_type>::value
    >::type> 
        : std::true_type {};

    // has_data_exact
    template<typename, typename T>
    struct has_data_exact {
    };

    template<typename Container, typename Ret, typename... Args>
    struct has_data_exact<Container, Ret(Args...)> {
    private:
        template<class U>
        static constexpr auto Test(U*)
        -> typename
            std::is_same<
                decltype( std::declval<U>().data( std::declval<Args>()... ) ),
                Ret    
            >::type; 

        template<class U>
        static constexpr std::false_type Test(...);
    public:
        static constexpr bool value = std::is_same<decltype(Test<Container>((Container*)0)),std::true_type>::value;
    };

    // is_contiguous_container

    // follows boost https://github.com/boostorg/beast/blob/develop/include/boost/beast/core/detail/type_traits.hpp
    template<class Container, class ElementType, class = void>
    struct is_contiguous_container: std::false_type {};

    template<class Container, class ElementType>
    struct is_contiguous_container<Container, ElementType, jsoncons::void_t<
        decltype(
            std::declval<std::size_t&>() = std::declval<const Container&>().size(),
            std::declval<ElementType*&>() = std::declval<Container&>().data()),
        typename std::enable_if<
            std::is_same<
                typename std::remove_cv<ElementType>::type,
                typename std::remove_cv<
                    typename std::remove_pointer<
                        decltype(std::declval<Container&>().data())
                    >::type
                >::type
            >::value
        >::type>>: std::true_type
    {};

    // has_reserve_exact
    template<typename, typename T>
    struct has_reserve_exact {
    };

    template<typename Container, typename Ret, typename... Args>
    struct has_reserve_exact<Container, Ret(Args...)> {
    private:
        template<class U>
        static constexpr auto Test(U*)
        -> typename
            std::is_same<
                decltype( std::declval<U>().reserve( std::declval<Args>()... ) ),
                Ret    
            >::type; 

        template<class U>
        static constexpr std::false_type Test(...);
    public:
        static constexpr bool value = std::is_same<decltype(Test<Container>((Container*)0)),std::true_type>::value;
    };

    // is_reservable_container

    template<class Container>
    struct is_reservable_container
    {
        static constexpr bool value = has_reserve_exact<Container,void(typename Container::size_type)>::value;
    };

    // is_c_array

    template<class T>
    struct is_c_array : std::false_type {};

    template<class T>
    struct is_c_array<T[]> : std::true_type {};

    template<class T, std::size_t N>
    struct is_c_array<T[N]> : std::true_type {};

    // is_compatible_element

    template<class C, class E, class Enable=void>
    struct is_compatible_element : std::false_type {};

    template<class C, class E>
    struct is_compatible_element
    <
        C, E, 
        typename std::enable_if<!std::is_void<decltype(std::declval<C>().data())>::value>::type>
            : std::is_convertible< typename std::remove_pointer<decltype(std::declval<C>().data() )>::type(*)[], E(*)[]>
    {};

    template<class C, class Enable=void>
    struct is_typed_array : std::false_type {};

    template<class T>
    struct is_typed_array
    <
        T, 
        typename std::enable_if<jsoncons::detail::is_vector_like<T>::value && 
                                (std::is_same<typename T::value_type,uint8_t>::value ||  
                                 std::is_same<typename T::value_type,uint16_t>::value ||
                                 std::is_same<typename T::value_type,uint32_t>::value ||
                                 std::is_same<typename T::value_type,uint64_t>::value ||
                                 std::is_same<typename T::value_type,int8_t>::value ||  
                                 std::is_same<typename T::value_type,int16_t>::value ||
                                 std::is_same<typename T::value_type,int32_t>::value ||
                                 std::is_same<typename T::value_type,int64_t>::value ||
                                 std::is_same<typename T::value_type,float_t>::value ||
                                 std::is_same<typename T::value_type,double_t>::value)>::type
    > : std::true_type{};

} // detail
} // jsoncons

#endif
