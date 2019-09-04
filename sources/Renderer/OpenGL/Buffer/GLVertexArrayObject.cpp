/*
 * GLVertexArrayObject.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLVertexArrayObject.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionLoader.h"
#include "../RenderState/GLStateManager.h"
#include "../../GLCommon/GLTypes.h"
#include "../../GLCommon/GLCore.h"
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

void GLVertexArrayObject::BuildVertexAttribute(const VertexAttribute& attribute, GLsizei stride, GLuint index)
{
    if (!HasExtension(GLExt::ARB_vertex_array_object))
        ThrowNotSupportedExcept(__FUNCTION__, "OpenGL extension 'GL_ARB_vertex_array_object'");

    /* Enable array index in currently bound VAO */
    glEnableVertexAttribArray(index);

    /* Set instance divisor */
    if (attribute.instanceDivisor > 0)
        glVertexAttribDivisor(index, attribute.instanceDivisor);

    /* Get data type and components of vector type */
    const auto& formatAttribs = GetFormatAttribs(attribute.format);

    /* Convert offset to pointer sized type (for 32- and 64 bit builds) */
    const GLsizeiptr offsetPtrSized = attribute.offset;

    /* Use currently bound VBO for VertexAttribPointer functions */
    if ((formatAttribs.flags & FormatFlags::IsNormalized) == 0 && !IsFloatFormat(attribute.format))
    {
        if (HasExtension(GLExt::EXT_gpu_shader4))
        {
            glVertexAttribIPointer(
                index,
                static_cast<GLint>(formatAttribs.components),
                GLTypes::Map(formatAttribs.dataType),
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
            index,
            static_cast<GLint>(formatAttribs.components),
            GLTypes::Map(formatAttribs.dataType),
            GLBoolean((formatAttribs.flags & FormatFlags::IsNormalized) != 0),
            stride,
            reinterpret_cast<const void*>(offsetPtrSized)
        );
    }
}


} // /namespace LLGL



// ================================================================================
