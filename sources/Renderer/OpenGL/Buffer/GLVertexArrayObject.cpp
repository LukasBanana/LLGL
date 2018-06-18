/*
 * GLVertexArrayObject.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLVertexArrayObject.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionLoader.h"
#include "../../../Core/Exception.h"
#include "../RenderState/GLStateManager.h"
#include "../../GLCommon/GLTypes.h"


namespace LLGL
{


GLVertexArrayObject::GLVertexArrayObject()
{
    glGenVertexArrays(1, &id_);
}

GLVertexArrayObject::~GLVertexArrayObject()
{
    glDeleteVertexArrays(1, &id_);
    GLStateManager::active->NotifyVertexArrayRelease(id_);
}

void GLVertexArrayObject::BuildVertexAttribute(const VertexAttribute& attribute, std::uint32_t stride, std::uint32_t index)
{
    /* Enable array index in currently bound VAO */
    glEnableVertexAttribArray(index);

    /* Set instance divisor */
    if (attribute.instanceDivisor > 0)
        glVertexAttribDivisor(index, attribute.instanceDivisor);

    /* Get data type and components of vector type */
    DataType        dataType    = DataType::Float;
    std::uint32_t   components  = 0;
    VectorTypeFormat(attribute.vectorType, dataType, components);

    /* Convert offset to pointer sized type (for 32- and 64 bit builds) */
    const GLsizeiptr offsetPtrSized = attribute.offset;

    /* Use currently bound VBO for VertexAttribPointer functions */
    if (!attribute.conversion && dataType != DataType::Float && dataType != DataType::Double)
    {
        if (!HasExtension(GLExt::EXT_gpu_shader4))
            ThrowNotSupportedExcept(__FUNCTION__, "integral vertex attributes");

        glVertexAttribIPointer(
            index,
            components,
            GLTypes::Map(dataType),
            stride,
            reinterpret_cast<const void*>(offsetPtrSized)
        );
    }
    else
    {
        glVertexAttribPointer(
            index,
            components,
            GLTypes::Map(dataType),
            GL_FALSE,
            stride,
            reinterpret_cast<const void*>(offsetPtrSized)
        );
    }
}


} // /namespace LLGL



// ================================================================================
