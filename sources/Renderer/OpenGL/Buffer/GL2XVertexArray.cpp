/*
 * GL2XVertexArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GL2XVertexArray.h"
#include "../Ext/GLExtensions.h"
#include "../RenderState/GLStateManager.h"
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../../../Core/Exception.h"


namespace LLGL
{


void GL2XVertexArray::BuildVertexAttribute(GLuint bufferID, const VertexAttribute& attribute)
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
    const auto& formatAttribs = GetFormatAttribs(attribute.format);
    if ((formatAttribs.flags & FormatFlags::SupportsVertex) == 0)
        ThrowNotSupportedExcept(__FUNCTION__, "specified vertex attribute");

    /* Convert offset to pointer sized type (for 32- and 64 bit builds) */
    auto dataType       = GLTypes::Map(formatAttribs.dataType);
    auto components     = static_cast<GLint>(formatAttribs.components);
    auto attribIndex    = static_cast<GLuint>(attribute.location);
    auto stride         = static_cast<GLsizei>(attribute.stride);
    auto offsetPtrSized = static_cast<GLsizeiptr>(attribute.offset);

    attribs_.push_back(
        {
            bufferID,
            attribIndex,
            components,
            dataType,
            GLBoolean(isNormalizedFormat),
            stride,
            reinterpret_cast<const GLvoid*>(offsetPtrSized)
        }
    );
}

void GL2XVertexArray::Finalize()
{
    if (attribs_.empty())
        return;

    /* Validate attribute indices are unique and fill the entire range [0, N) */
    std::vector<bool> locationsTaken;
    locationsTaken.resize(attribs_.size(), false);

    for (const auto& attr : attribs_)
    {
        if (attr.index > locationsTaken.size() || locationsTaken[attr.index])
            throw std::runtime_error("vertex attribute locations must fill the entire half-open range [0, N) for OpenGL 2.X");
        locationsTaken[attr.index] = true;
    }

    /* Store upper bound for attribute indices */
    attribIndexEnd_ = attribs_.back().index + 1;

    /* Sort attributes by buffer binding and index in ascending order */
    std::sort(
        attribs_.begin(),
        attribs_.end(),
        [](const GL2XVertexAttrib& lhs, const GL2XVertexAttrib& rhs)
        {
            if (lhs.buffer < rhs.buffer)
                return true;
            if (lhs.buffer > rhs.buffer)
                return false;
            return (lhs.index < rhs.index);
        }
    );
}

void GL2XVertexArray::Bind(GLStateManager& stateMngr) const
{
    /* Enable all vertex arrays */
    for (const auto& attr : attribs_)
    {
        stateMngr.BindBuffer(GLBufferTarget::ARRAY_BUFFER, attr.buffer);
        glVertexAttribPointer(attr.index, attr.size, attr.type, attr.normalized, attr.size, attr.pointer);
        glEnableVertexAttribArray(attr.index);
    }

    /* Disable remaining vertex arrays */
    stateMngr.DisableVertexAttribArrays(attribIndexEnd_);
}


} // /namespace LLGL



// ================================================================================
