/*
 * GLBufferConverter.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_BUFFER_CONVERTER_H
#define LLGL_GL_BUFFER_CONVERTER_H


#include <LLGL/Format.h>
#include "../OpenGL.h"
#include <cstdint>
#include <cstddef>


namespace LLGL
{

namespace GLBufferConverter
{


// Sized internal format for GLBuffer.
union FormatData
{
    std::int8_t     int8[4];
    std::uint8_t    uint8[4];
    std::int16_t    int16[4];
    std::uint16_t   uint16[4];
    std::int32_t    int32[4];
    std::uint32_t   uint32[4];
    float           real32[4];
};


// Describes the FormatData structure.
struct FormatDataDescriptor
{
    DataType    baseType;
    int         components;
    bool        normalized;
};

/*
Returns the size (in bytes) of the specified internal buffer format.
see https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glClearBufferSubData.xhtml#description
*/
FormatDataDescriptor GetFormatDataDesc(GLenum format);

// Converst the specified GLBuffer format data and returns the size of the destination buffer (in bytes).
std::size_t ConvertFormatData(
    FormatData&                 dst,
    const FormatDataDescriptor& dstDesc,
    const FormatData&           src,
    const FormatDataDescriptor& srcDesc
);


} // /namespace GLBufferConverter

} // /namespace LLGL


#endif



// ================================================================================
