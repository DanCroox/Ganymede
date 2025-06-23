#pragma once

#include <type_traits>

namespace bitsery { class Access; }

namespace Ganymede
{
    class Serializer;
    template<typename T, typename = void>
    struct is_serializable : std::false_type {};

    template<typename T>
    struct is_serializable<T, std::void_t<decltype(T::is_serializable)>>
        : std::bool_constant<T::is_serializable> {};
}

#define GM_PRIV_SERIALIZABLE_BASE(classname)            \
    static constexpr bool is_serializable = true;       \
    friend bitsery::Access;                             \
    friend Ganymede::Serializer;                        \
    template<typename U, typename = void>               \
    friend struct Ganymede::is_serializable;            \

// Mark any class as serializable by adding this makro to any section of a class definition.
// Every serializable type also needs a free "serialize" function which defines how to serialize:
// template <typename S>
// void serialize(S& s, MyClass& obj) {}
// 
// Also works for template types:
// template <typename S, typename M, typename N>
// void serialize(S& s, MyClass<M, N>& obj) {}
// 
// For this we use the bitsery library and don't bug around with a more abstract interface for serialization.
// Bitsery is header only, cross platform cpp11/17 ready and also works on all sorts of hardware out of the box.
// See "Ganymede/Data/SerializerTypes.h" for examples how to implement a free serialize function for a type.
#define GM_SERIALIZABLE(classname)                      \
    GM_PRIV_SERIALIZABLE_BASE(classname);               \
    template<typename BitseryS>                         \
    friend void serialize(BitseryS& s, classname& obj);

// Used to create serializable template types. For more check macro comment on "GM_SERIALIZABLE(..)"
#define GM_SERIALIZABLE_TEMPLATED(classname, ...)       \
    GM_PRIV_SERIALIZABLE_BASE(classname);               \
    template<typename BitseryS, __VA_ARGS__>            \
    friend void serialize(BitseryS& s, classname& obj);