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


GLVertexArrayObject::GLVertexArrayObject()
{
    if (HasNativeVAO())
        glGenVertexArrays(1, &id_);
}

GLVertexArrayObject::~GLVertexArrayObject()
{
    if (HasNativeVAO())
    {
        glDeleteVertexArrays(1, &id_);
        GLStateManager::Get().NotifyVertexArrayRelease(id_);
    }
}

void GLVertexArrayObject::BuildVertexAttribute(const VertexAttribute& attribute)
{
    LLGL_ASSERT_GL_EXT(ARB_vertex_array_object);

    /* Get data type and components of vector type */
    const auto& formatAttribs = GetFormatAttribs(attribute.format);
    if ((formatAttribs.flags & FormatFlags::SupportsVertex) == 0)
    {
        if (auto formatStr = ToString(attribute.format))
            LLGL_TRAP("LLGL::Format::%s cannot be used for vertex attributes", formatStr);
        else
            LLGL_TRAP("unknown format cannot be used for vertex attributes");
    }

    /* Convert offset to pointer sized type (for 32- and 64 bit builds) */
    auto dataType       = GLTypes::Map(formatAttribs.dataType);
    auto components     = static_cast<GLint>(formatAttribs.components);
    auto attribIndex    = static_cast<GLuint>(attribute.location);
    auto stride         = static_cast<GLsizei>(attribute.stride);
    auto offsetPtrSized = static_cast<GLsizeiptr>(attribute.offset);

    /* Enable array index in currently bound VAO */
    glEnableVertexAttribArray(attribIndex);

    /* Set instance divisor */
    if (attribute.instanceDivisor > 0)
        glVertexAttribDivisor(attribIndex, attribute.instanceDivisor);

    /* Use currently bound VBO for VertexAttribPointer functions */
    if ((formatAttribs.flags & FormatFlags::IsNormalized) == 0 && !IsFloatFormat(attribute.format))
    {
        LLGL_ASSERT_GL_EXT(EXT_gpu_shader4, "integral vertex attributes");
        glVertexAttribIPointer(
            attribIndex,
            components,
            dataType,
            stride,
            reinterpret_cast<const void*>(offsetPtrSized)
        );
    }
    else
    {
        glVertexAttribPointer(
            attribIndex,
            components,
            dataType,
            GLBoolean((formatAttribs.flags & FormatFlags::IsNormalized) != 0),
            stride,
            reinterpret_cast<const void*>(offsetPtrSized)
        );
    }
}


} // /namespace LLGL



// ================================================================================
