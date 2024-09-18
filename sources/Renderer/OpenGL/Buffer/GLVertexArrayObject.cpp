/*
 * GLVertexArrayObject.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLVertexArrayObject.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../RenderState/GLStateManager.h"
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../../../Core/Exception.h"
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

#if LLGL_GL3PLUS_SUPPORTED

void GLVertexArrayObject::Release()
{
    //TODO: this must use some form of deferred deletion as this d'tor is not guaranteed to be invoked with the correct GL context in place
    if (id_ != 0)
    {
        glDeleteVertexArrays(1, &id_);
        GLStateManager::Get().NotifyVertexArrayRelease(id_);
        id_ = 0;
    }
}

void GLVertexArrayObject::BuildVertexLayout(const ArrayView<GLVertexAttribute>& attributes)
{
    LLGL_ASSERT_GL_EXT(ARB_vertex_array_object);

    /* Generate a VAO if not already done */
    if (id_ == 0)
        glGenVertexArrays(1, &id_);

    /* Build vertex attributes for this VAO */
    GLStateManager::Get().BindVertexArray(id_);
    {
        for (const GLVertexAttribute& attrib : attributes)
            BuildVertexAttribute(attrib);
    }
    GLStateManager::Get().BindVertexArray(0);
}


/*
 * ======= Private: =======
 */

void GLVertexArrayObject::BuildVertexAttribute(const GLVertexAttribute& attribute)
{
    GLStateManager::Get().BindBuffer(GLBufferTarget::ArrayBuffer, attribute.buffer);

    /* Enable array index in currently bound VAO */
    glEnableVertexAttribArray(attribute.index);

    /* Set instance divisor */
    if (attribute.divisor > 0)
        glVertexAttribDivisor(attribute.index, attribute.divisor);

    /* Use currently bound VBO for VertexAttribPointer functions */
    if (attribute.isInteger)
    {
        LLGL_ASSERT_GL_EXT(EXT_gpu_shader4, "integral vertex attributes");
        glVertexAttribIPointer(
            attribute.index,
            attribute.size,
            attribute.type,
            attribute.stride,
            reinterpret_cast<const void*>(attribute.offsetPtrSized)
        );
    }
    else
    {
        glVertexAttribPointer(
            attribute.index,
            attribute.size,
            attribute.type,
            attribute.normalized,
            attribute.stride,
            reinterpret_cast<const void*>(attribute.offsetPtrSized)
        );
    }
}

#endif // /LLGL_GL3PLUS_SUPPORTED


} // /namespace LLGL



// ================================================================================
