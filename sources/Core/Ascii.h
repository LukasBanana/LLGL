/*
 * Ascii.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_ASCII_H
#define LLGL_ASCII_H


namespace LLGL
{

namespace Ascii
{


inline constexpr bool IsCharAlpha(char c)
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

inline constexpr bool IsCharNumeric(char c)
{
    return (c >= '0' && c <= '9');
}

inline constexpr bool IsCharNumericHex(char c)
{
    return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

inline constexpr bool IsCharIdentifier(char c)
{
    return (IsCharAlpha(c) || IsCharNumeric(c) || c == '_');
}

inline constexpr bool IsCharWhitespace(char c)
{
    return (c == ' ' || c == '\t' || c == '\v' || c == '\n' || c == '\r');
}


} // /namespace Ascii

} // /namespace LLGL


#endif



// ================================================================================
