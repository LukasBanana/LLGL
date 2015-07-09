/*
 * TextureFormat.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_TEXTURE_FORMAT_H__
#define __LLGL_TEXTURE_FORMAT_H__


#include <string>
#include <details/API.h>


namespace LLGL
{


enum class TextureFormat
{
    R8ub,
    RG8ub,
    RGB8ub,
    RGBA8ub,
    R32f,
    RG32f,
    RGB32f,
    RGBA32f,
};


LLGL_EXPORT LLGL_API std::string ToString(TextureFormat value);


} // /namespace LLGL


#endif



// ================================================================================
