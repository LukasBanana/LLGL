/*
 * Memory.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MEMORY_H
#define LLGL_MEMORY_H


#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <string.h>


namespace LLGL
{

namespace Utils
{


template <std::size_t ByteSize>
struct ByteCmpUtils
{
    static bool AllBytesEqual(const char* ptr)
    {
        /* Compare first 4 bytes individually */
        if (!ByteCmpUtils<ByteSize % 4u>::AllBytesEqual(ptr))
            return false;

        /* Compare tail as 32-bit values */
        const std::uint32_t* ptr32 = reinterpret_cast<const std::uint32_t*>(ptr + ByteSize - (ByteSize/sizeof(std::uint32_t))*sizeof(std::uint32_t));
        for (const std::uint32_t* ptr32End = ptr32 + ByteSize/sizeof(std::uint32_t) - 1; ptr32 != ptr32End; ++ptr32)
        {
            if (ptr32[0] != ptr32[1])
                return false;
        }

        return true;
    }
};

template <>
struct ByteCmpUtils<4>
{
    static bool AllBytesEqual(const char* ptr)
    {
        const std::uint16_t* ptr16 = reinterpret_cast<const std::uint16_t*>(ptr);
        return (ptr[0] == ptr[1] && ptr16[0] == ptr16[1]);
    }
};

template <>
struct ByteCmpUtils<3>
{
    static bool AllBytesEqual(const char* ptr)
    {
        return (ptr[0] == ptr[1] && ptr[1] == ptr[2]);
    }
};

template <>
struct ByteCmpUtils<2>
{
    static bool AllBytesEqual(const char* ptr)
    {
        return ptr[0] == ptr[1];
    }
};

template <>
struct ByteCmpUtils<1>
{
    static bool AllBytesEqual(const char* /*ptr*/)
    {
        return true;
    }
};

template <>
struct ByteCmpUtils<0>
{
    static bool AllBytesEqual(const char* /*ptr*/)
    {
        return true;
    }
};

template <typename TInner, bool IsTriviallyConstructible>
struct MemsetUtils;

template <typename TInner>
struct MemsetUtils<TInner, false>
{
    static void MemsetInternal(TInner* dst, const TInner& value, std::size_t count)
    {
        for (TInner* dstEnd = dst + count; dst != dstEnd; ++dst)
            *dst = value;
    }
};

template <typename TInner>
struct MemsetUtils<TInner, true>
{
    static void MemsetInternal(TInner* dst, const TInner& value, std::size_t count)
    {
        if (ByteCmpUtils<sizeof(value)>::AllBytesEqual(reinterpret_cast<const char*>(&value)))
            ::memset(dst, reinterpret_cast<const char*>(&value)[0], count * sizeof(TInner));
        else
            MemsetUtils<TInner, false>::MemsetInternal(dst, value, count);
    }
};


} // /namespace Utils

namespace Memory
{


/**
\brief Copies the specifies value 'count'-times into the specified memory location.
\param[out] dst Specifies the memory location where the input value will be copied to.
\param[in] value Specifies the input value that is meant to be copied into all entries.
\param[in] count Specifies how many entries will be set to the input value.
\remarks If the template typename \c <T> specifies a trivially constructible type, such as \c int or \c char,
and all bytes in the input value are equal, the standard library function \c ::memset will be used.
Otherwise, a standard for-loop will be used to initialize all entries.
*/
template <typename T>
void Memset(T* dst, const T& value, std::size_t count)
{
    Utils::MemsetUtils<T, std::is_trivially_copyable<T>::value>::MemsetInternal(dst, value, count);
}


} // /namespace Memory

} // /namespace LLGL


#endif



// ================================================================================
