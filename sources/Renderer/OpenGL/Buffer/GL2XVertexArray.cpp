/*
 * GL2XVertexArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GL2XVertexArray.h"
#include "../Ext/GLExtensions.h"
#include "../RenderState/GLStateManager.h"
#include "../../GLCommon/GLTypes.h"
#include "../../GLCommon/GLCore.h"
#include "../../../Core/Exception.h"


namespace LLGL
{


void GL2XVertexArray::BuildVertexAttribute(GLuint bufferID, const VertexAttribute& attribute, GLuint attribIndex)
{
    /* Check if instance divisor is used */
    if (attribute.instanceDivisor > 0)
        ThrowNotSupportedExcept(__FUNCTION__, "per-instance vertex attributes");

    /* Check if integral vertex attribute is used */
    auto isNormalizedFormat = IsNormalizedFormat(attribute.format);
    auto isFloatFormat      = IsFloatFormat(attribute.format);

    if (!isNormalizedFormat && !isFloatFormat)
        ThrowNotSupportedExcept(__FUNCTION__, "integral vertex attributes");

    /* Get data type and components of vector type */
    const auto& formatDesc = GetFormatAttribs(attribute.format);
    if (formatDesc.bitSize == 0)
        ThrowNotSupportedExcept(__FUNCTION__, "specified vertex attribute");

    /* Convert offset to pointer sized type (for 32- and 64 bit builds) */
    const GLsizei       stride          = static_cast<GLsizei>(attribute.stride);
    const GLsizeiptr    offsetPtrSized  = static_cast<GLsizeiptr>(attribute.offset);

    attribs_.push_back(
        {
            bufferID,
            attribIndex,
            static_cast<GLint>(formatDesc.components),
            GLTypes::Map(formatDesc.dataType),
            GLBoolean(isNormalizedFormat),
            stride,
            reinterpret_cast<const GLvoid*>(offsetPtrSized)
        }
    );
}

void GL2XVertexArray::Bind(GLStateManager& stateMngr) const
{
    if (!attribs_.empty())
    {
        /* Enable all vertex arrays */
        for (const auto& attr : attribs_)
        {
            stateMngr.BindBuffer(GLBufferTarget::ARRAY_BUFFER, attr.buffer);
            glVertexAttribPointer(attr.index, attr.size, attr.type, attr.normalized, attr.size, attr.pointer);
            glEnableVertexAttribArray(attr.index);
        }

        /* Disable remaining vertex arrays */
        stateMngr.DisableVertexAttribArrays(attribs_.back().index + 1);
    }
    else
        stateMngr.DisableVertexAttribArrays(0);
}


} // /namespace LLGL



// ================================================================================
