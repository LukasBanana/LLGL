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
