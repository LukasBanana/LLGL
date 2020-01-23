/*
 * Helper.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_HELPER_H
#define LLGL_HELPER_H


#include "../Renderer/CheckedCast.h"
#include <LLGL/Export.h>
#include <algorithm>
#include <type_traits>
#include <memory>
#include <vector>
#include <list>
#include <algorithm>
#include <set>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <functional>
#include <cstdint>


//TODO: separate this header into multiple "*Utils.h" files, such as "ContainerUtils.h"

namespace LLGL
{


/* ----- Templates ----- */

// Alternative to std::make_unique<T> for strict C++11 support.
template <typename T, typename... Args>
std::unique_ptr<T> MakeUnique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Alternative to std::make_unique<T[]> for strict C++11 support.
template <typename T>
std::unique_ptr<T[]> MakeUniqueArray(std::size_t size)
{
    return std::unique_ptr<T[]>(new T[size]);
}

// Initializes the specified data of basic type of POD structure type with zeros (using ::memset).
template <class T>
void InitMemory(T& data)
{
    static_assert(!std::is_pointer<T>::value, "'InitMemory' does not allow pointer types");
    static_assert(std::is_pod<T>::value, "'InitMemory' does only allow plain-old-data (POD)");
    ::memset(&data, 0, sizeof(T));
}

// Fills the specified container with 'value' (using std::fill).
template <class Container, class T>
void Fill(Container& cont, const T& value)
{
    std::fill(std::begin(cont), std::end(cont), value);
}

// Returns true if the specified container contains the entry specified by 'value' (using std::find).
template <class Container, class T>
bool Contains(const Container& cont, const T& value)
{
    return (std::find(std::begin(cont), std::end(cont), value) != std::end(cont));
}

template <class Container, class T>
void RemoveFromList(Container& cont, const T& entry)
{
    auto it = std::find(std::begin(cont), std::end(cont), entry);
    if (it != std::end(cont))
        cont.erase(it);
}

template <class Container, class UnaryPredicate>
void RemoveFromListIf(Container& cont, UnaryPredicate pred)
{
    auto it = std::find_if(std::begin(cont), std::end(cont), pred);
    if (it != std::end(cont))
        cont.erase(it);
}

template <class Container, class T>
void RemoveAllFromList(Container& cont, const T& entry)
{
    cont.erase(
        std::remove(std::begin(cont), std::end(cont), entry),
        cont.end()
    );
}

template <class Container, class UnaryPredicate>
void RemoveAllFromListIf(Container& cont, UnaryPredicate pred)
{
    cont.erase(
        std::remove_if(std::begin(cont), std::end(cont), pred),
        cont.end()
    );
}

template <typename T>
void AddOnceToSharedList(std::vector<std::shared_ptr<T>>& cont, const std::shared_ptr<T>& entry)
{
    if (entry && std::find(cont.begin(), cont.end(), entry) == cont.end())
        cont.push_back(entry);
}

template <typename T, typename TBase>
void RemoveFromSharedList(std::vector<std::shared_ptr<T>>& cont, const TBase* entry)
{
    if (entry)
    {
        RemoveFromListIf(
            cont,
            [entry](const std::shared_ptr<T>& e)
            {
                return (e.get() == entry);
            }
        );
    }
}

template <typename T, typename TBase>
void RemoveFromUniqueSet(std::set<std::unique_ptr<T>>& cont, const TBase* entry)
{
    if (entry)
    {
        RemoveFromListIf(
            cont,
            [entry](const std::unique_ptr<T>& e)
            {
                return (e.get() == entry);
            }
        );
    }
}

template <typename BaseType, typename SubType>
SubType* TakeOwnership(std::set<std::unique_ptr<BaseType>>& objectSet, std::unique_ptr<SubType>&& object)
{
    auto ref = object.get();
    objectSet.emplace(std::forward<std::unique_ptr<SubType>>(object));
    return ref;
}

template <typename BaseType, typename SubType>
SubType* TakeOwnership(std::vector<std::unique_ptr<BaseType>>& objectSet, std::unique_ptr<SubType>&& object)
{
    auto ref = object.get();
    objectSet.emplace_back(std::forward<std::unique_ptr<SubType>>(object));
    return ref;
}

template <typename BaseType, typename SubType>
SubType* TakeOwnership(std::list<std::unique_ptr<BaseType>>& objectSet, std::unique_ptr<SubType>&& object)
{
    auto ref = object.get();
    objectSet.emplace_back(std::forward<std::unique_ptr<SubType>>(object));
    return ref;
}

// Similar to std::unqiue but with predicate that allows to modify elements.
template <typename ForwardIt, typename BinaryPredicate>
static ForwardIt UniqueMerge(ForwardIt begin, ForwardIt end, BinaryPredicate pred)
{
    if (begin == end)
        return end;

    ForwardIt result = begin;
    while (++begin != end)
    {
        if (!pred(*result, *begin) && ++result != begin)
            *result = std::move(*begin);
    }

    return ++result;
}

// Returns the specified integral value as hexadecimal string.
template <typename T>
std::string ToHex(T value)
{
    static_assert(std::is_integral<T>::value, "input parameter of 'LLGL::ToHex' must be an integral type");
    std::stringstream s;
    s << std::setfill('0') << std::setw(sizeof(T)*2) << std::hex << std::uppercase << static_cast<std::uint64_t>(value);
    return s.str();
}

/*
\brief Returns the next resource from the specified resource array.
\param[in,out] numResources Specifies the remaining number of resources in the array.
\param[in,out] resourceArray Pointer to the remaining array of resource pointers.
\remarks If the last element in the array is reached,
'resourceArray' is points to the location after the last entry, and 'numResources' is 0.
*/
template <typename TSub, typename TBase>
TSub* NextArrayResource(std::uint32_t& numResources, TBase* const * & resourceArray)
{
    if (numResources > 0)
    {
        --numResources;
        return LLGL_CAST(TSub*, (*(resourceArray++)));
    }
    return nullptr;
}

// Returns the adjusted size with the specified alignment, which is always greater or equal to 'size' (T can be UINT or VkDeviceSize for instance).
template <typename T>
T GetAlignedSize(T size, T alignment)
{
    if (alignment > 1)
    {
        const T alignmentBitmask = alignment - 1;
        return ((size + alignmentBitmask) & ~alignmentBitmask);
    }
    return size;
}

// Clamps the value x into the range [minimum, maximum].
template <typename T>
T Clamp(const T& x, const T& minimum, const T& maximum)
{
    return std::max(minimum, std::min(x, maximum));
}

// Returns the raw function pointer of the specified member function, e.g. GetMemberFuncPtr(&Foo::Bar).
template <typename T>
const void* GetMemberFuncPtr(T pfn)
{
    union
    {
        T           func;
        const void* addr;
    }
    ptr;
    ptr.func = pfn;
    return ptr.addr;
}

// Returns the length of the specified null-terminated string.
template <typename T>
inline std::size_t StrLength(const T* s)
{
    std::size_t len = 0;
    while (*s++ != 0)
        ++len;
    return len;
}

// Specialization of StrLength template for ANSI strings.
template <>
inline std::size_t StrLength<char>(const char* s)
{
    return std::strlen(s);
}

// Specialization of StrLength template for Unicode strings.
template <>
inline std::size_t StrLength<wchar_t>(const wchar_t* s)
{
    return std::wcslen(s);
}

// Advances the specified pointer with a byte-aligned offset.
template <typename T>
inline T* AdvancePtr(T* ptr, std::size_t offset)
{
    using TByteAligned = typename std::conditional
    <
        std::is_const<T>::value,
        const std::int8_t,
        std::int8_t
    >::type;
    return reinterpret_cast<T*>(reinterpret_cast<TByteAligned*>(ptr) + offset);
}


/* ----- Functions ----- */

// Reads the specified text file into a string.
LLGL_EXPORT std::string ReadFileString(const char* filename);

// Reads the specified binary file into a buffer.
LLGL_EXPORT std::vector<char> ReadFileBuffer(const char* filename);

// Converts the UTF16 input string to UTF8 string.
LLGL_EXPORT std::string ToUTF8String(const std::wstring& utf16);
LLGL_EXPORT std::string ToUTF8String(const wchar_t* utf16);

// Converts the UTF8 input string to UTF16 string.
LLGL_EXPORT std::wstring ToUTF16String(const std::string& utf8);
LLGL_EXPORT std::wstring ToUTF16String(const char* utf8);


} // /namespace LLGL


#endif



// ================================================================================
