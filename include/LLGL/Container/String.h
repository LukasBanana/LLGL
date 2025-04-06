/*
 * String.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_STRING_H
#define LLGL_STRING_H

#include <LLGL/Container/STLAllocator.h>

#include <string>

namespace LLGL{

template < typename CharType >
using string_type = std::basic_string< CharType, std::char_traits< CharType >, llgl_allocator< CharType > >;

typedef string_type< char >     string;
typedef string_type< wchar_t >  wstring;
#ifdef __cpp_char8_t
typedef string_type< char8_t >  u8string;
#endif
typedef string_type< char16_t > u16string;
typedef string_type< char32_t > u32string;

} // /namespace LLGL

#endif

// ================================================================================
