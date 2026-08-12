/*
 * GLVertexAttribute.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLVertexAttribute.h"
#include "../GLCore.h"
#include "../GLTypes.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/TypeNames.h>


namespace LLGL
{


void GLConvertVertexAttrib(GLVertexAttribute& dst, const VertexAttribute& src, GLuint srcBuffer)
{
    /* D3DCOLOR-style vertex data (memory byte order B,G,R,A) needs GL_BGRA;
       only GL_ARB_vertex_attrib_bgra allows it in core profiles, which the
       NVIDIA/AMD/Intel Windows drivers expose even in GL 4.x core contexts. */
    if (src.format == Format::BGRA8UNorm)
    {
        dst.buffer          = srcBuffer;
        dst.index           = static_cast<GLuint>(src.location);
        // GL_ARB_vertex_array_bgra encodes BGRA in the size parameter.
        // The component type remains GL_UNSIGNED_BYTE. Passing GL_BGRA as
        // the type produces GL_INVALID_ENUM and disables the attribute.
        dst.size            = static_cast<GLint>(GL_BGRA);
        dst.type            = GL_UNSIGNED_BYTE;
        dst.normalized      = GL_TRUE;
        dst.stride          = static_cast<GLsizei>(src.stride);
        dst.offsetPtrSized  = static_cast<GLsizeiptr>(src.offset);
        dst.divisor         = static_cast<GLuint>(src.instanceDivisor);
        dst.isInteger       = false;
        return;
    }

    /* Get data type and components of vector type */
    const FormatAttributes& formatAttribs = GetFormatAttribs(src.format);
    if ((formatAttribs.flags & FormatFlags::SupportsVertex) == 0)
    {
        if (const char* formatStr = ToString(src.format))
            LLGL_TRAP("LLGL::Format::%s cannot be used for vertex attributes", formatStr);
        else
            LLGL_TRAP("unknown format cannot be used for vertex attributes");
    }

    /* Convert offset to pointer sized type (for 32- and 64 bit builds) */
    dst.buffer          = srcBuffer;
    dst.index           = static_cast<GLuint>(src.location);
    dst.size            = static_cast<GLint>(formatAttribs.components);
    dst.type            = GLTypes::Map(formatAttribs.dataType);
    dst.normalized      = GLBoolean((formatAttribs.flags & FormatFlags::IsNormalized) != 0);
    dst.stride          = static_cast<GLsizei>(src.stride);
    dst.offsetPtrSized  = static_cast<GLsizeiptr>(src.offset);
    dst.divisor         = static_cast<GLuint>(src.instanceDivisor);
    dst.isInteger       = IsIntegerFormat(src.format);
}


} // /namespace LLGL



// ================================================================================
