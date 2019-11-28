/*
 * GLVertexArrayObject.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLVertexArrayObject.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionLoader.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../RenderState/GLStateManager.h"
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../../../Core/Exception.h"


namespace LLGL
{


GLVertexArrayObject::GLVertexArrayObject()
{
    if (HasExtension(GLExt::ARB_vertex_array_object))
        glGenVertexArrays(1, &id_);
}

GLVertexArrayObject::~GLVertexArrayObject()
{
    if (HasExtension(GLExt::ARB_vertex_array_object))
    {
        glDeleteVertexArrays(1, &id_);
        GLStateManager::Get().NotifyVertexArrayRelease(id_);
    }
}

void GLVertexArrayObject::BuildVertexAttribute(const VertexAttribute& attribute)
{
    if (!HasExtension(GLExt::ARB_vertex_array_object))
        ThrowNotSupportedExcept(__FUNCTION__, "OpenGL extension 'GL_ARB_vertex_array_object'");

    /* Get data type and components of vector type */
    const auto& formatAttribs = GetFormatAttribs(attribute.format);
    if ((formatAttribs.flags & FormatFlags::SupportsVertex) == 0)
        ThrowNotSupportedExcept(__FUNCTION__, "specified vertex attribute");

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
        if (HasExtension(GLExt::EXT_gpu_shader4))
        {
            glVertexAttribIPointer(
                attribIndex,
                components,
                dataType,
                stride,
                reinterpret_cast<const void*>(offsetPtrSized)
            );
        }
        else
            ThrowNotSupportedExcept(__FUNCTION__, "integral vertex attributes");
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
