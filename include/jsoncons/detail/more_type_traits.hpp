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

    // detector

    // primary template handles all types not supporting the archetypal Op
    template< 
        class Default, 
        class, // always void; supplied externally
        template<class...> class Op, 
        class... Args
    >
    struct detector
    {
        constexpr static auto value = false;
        using type = Default;
    };

    // specialization recognizes and handles only types supporting Op
    template< 
        class Default, 
        template<class...> class Op, 
        class... Args
    >
    struct detector<Default, void_t<Op<Args...>>, Op, Args...>
    {
        constexpr static auto value = true;
        using type = Op<Args...>;
    };

    // is_detected, is_detected_t

    template< template<class...> class Op, class... Args >
    using
    is_detected = detector<void, void, Op, Args...>;

    template< template<class...> class Op, class... Args >
    using
    is_detected_t = typename is_detected<Op, Args...>::type;

    // detected_or, detected_or_t

    template< class Default, template<class...> class Op, class... Args >
    using
    detected_or = detector<Default, void, Op, Args...>;

    template< class Default, template<class...> class Op, class... Args >
    using
    detected_or_t = typename detected_or<Default, Op, Args...>::type;

    // is_detected_exact

   template< class Expected, template<class...> class Op, class... Args >
   using
   is_detected_exact = std::is_same< Expected, is_detected_t<Op, Args...> >;

    // is_detected_convertible

    template< class To, template<class...> class Op, class... Args >
    using
    is_detected_convertible = std::is_convertible< is_detected_t<Op, Args...>, To >;

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

    template <class T>
    using container_npos_t = decltype(T::npos);

    template <class T>
    using container_allocator_type_t = typename T::allocator_type;

    template <class T, class Enable=void>
    struct is_string : std::false_type {};

    template <class T>
    struct is_string<T, 
                     typename std::enable_if<is_character<typename T::value_type>::value &&
                                             has_char_traits_member_type<T>::value && 
                                             is_detected<container_npos_t,T>::value &&
                                             is_detected<container_allocator_type_t,T>::value
    >::type> : std::true_type {};

    // is_string_view

    template <class T, class Enable=void>
    struct is_string_view : std::false_type {};

    template <class T>
    struct is_string_view<T, 
                          typename std::enable_if<is_character<typename T::value_type>::value &&
                                                  has_char_traits_member_type<T>::value && 
                                                  is_detected<container_npos_t,T>::value &&
                                                  !is_detected<container_allocator_type_t,T>::value
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

    template <class T>
    using container_mapped_type_t = typename T::mapped_type;

    template <class T>
    using container_key_type_t = typename T::key_type;

    template <class T>
    using container_value_type_t = typename std::iterator_traits<typename T::iterator>::value_type;

    template <class T, class Enable=void>
    struct is_map_like : std::false_type {};

    template <class T>
    struct is_map_like<T, 
                       typename std::enable_if<is_detected<container_mapped_type_t,T>::value &&
                                               is_detected<container_key_type_t,T>::value &&
                                               is_detected<container_value_type_t,T>::value 
        >::type> 
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
                          typename std::enable_if<is_detected<container_value_type_t,T>::value &&
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

    // has_reserve
    template<class Container>
    using
    container_reserve_t = decltype(std::declval<Container>().reserve(typename Container::size_type()));

    template<class Container>
    using
    has_reserve = is_detected<container_reserve_t, Container>;

    // has_data, has_data_exact
    template<class Container>
    using
    container_data_t = decltype(std::declval<Container>().data());

    template<class Container>
    using
    has_data = is_detected<container_data_t, Container>;

    template<class Ret, class Container>
    using
    has_data_exact = is_detected_exact<Ret, container_data_t, Container>;

    // has_size
    template<class Container>
    using
    container_size_t = decltype(std::declval<Container>().size());

    template<class Container>
    using
    has_size = is_detected<container_size_t, Container>;

    // has_data_and_size

    template<class Container>
    struct has_data_and_size
    {
        static constexpr bool value = has_data<Container>::value && has_size<Container>::value;
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
