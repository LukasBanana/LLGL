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
#include "../../GLCommon/GLCore.h"


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
    SplitFormat(attribute.format, dataType, components);

    auto isNormalizedFormat = IsNormalizedFormat(attribute.format);
    auto isFloatFormat      = IsFloatFormat(attribute.format);

    /* Convert offset to pointer sized type (for 32- and 64 bit builds) */
    const GLsizeiptr offsetPtrSized = attribute.offset;

    /* Use currently bound VBO for VertexAttribPointer functions */
    if (!isNormalizedFormat && !isFloatFormat)
    {
        if (HasExtension(GLExt::EXT_gpu_shader4))
        {
            glVertexAttribIPointer(
                index,
                components,
                GLTypes::Map(dataType),
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
            components,
            GLTypes::Map(dataType),
            GLBoolean(isNormalizedFormat),
            stride,
            reinterpret_cast<const void*>(offsetPtrSized)
        );
    }
}


} // /namespace LLGL



// ================================================================================
