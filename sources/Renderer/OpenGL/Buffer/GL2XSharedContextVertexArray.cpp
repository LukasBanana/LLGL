/*
 * GL2XSharedContextVertexArray.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GL2XSharedContextVertexArray.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../RenderState/GLStateManager.h"
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/Exception.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


void GL2XSharedContextVertexArray::BuildVertexLayout(GLuint bufferID, const ArrayView<VertexAttribute>& attributes)
{
    /* Convert vertex attributes into GL attributes and verify parameters for GL 2.x */
    const std::size_t startOffset = attribs_.size();
    attribs_.resize(startOffset + attributes.size());

    for_range(i, attributes.size())
    {
        const VertexAttribute& inAttrib = attributes[i];

        /* Check if instance divisor is used */
        if (inAttrib.instanceDivisor > 0)
            LLGL_TRAP_FEATURE_NOT_SUPPORTED("per-instance vertex attributes");

        /* Check if integral vertex attribute is used */
        bool isNormalizedFormat = IsNormalizedFormat(inAttrib.format);
        bool isFloatFormat      = IsFloatFormat(inAttrib.format);

        if (!isNormalizedFormat && !isFloatFormat)
            LLGL_TRAP_FEATURE_NOT_SUPPORTED("integral vertex attributes");

        /* Get data type and components of vector type */
        const FormatAttributes& formatAttribs = GetFormatAttribs(inAttrib.format);
        if ((formatAttribs.flags & FormatFlags::SupportsVertex) == 0)
            LLGL_TRAP_FEATURE_NOT_SUPPORTED("specified vertex attribute");

        /* Convert to GLVertexAttribute */
        GLConvertVertexAttrib(attribs_[startOffset + i], inAttrib, bufferID);
    }
}

void GL2XSharedContextVertexArray::Finalize()
{
    if (attribs_.empty())
        return;

    /* Validate attribute indices are unique and fill the entire range [0, N) */
    std::vector<bool> locationsTaken;
    locationsTaken.resize(attribs_.size(), false);

    for (const GLVertexAttribute& attr : attribs_)
    {
        LLGL_ASSERT(
            attr.index < locationsTaken.size() && !locationsTaken[attr.index],
            "vertex attribute locations must fill the entire half-open range [0, N) for OpenGL 2.X"
        );
        locationsTaken[attr.index] = true;
    }

    /* Store upper bound for attribute indices */
    attribIndexEnd_ = attribs_.back().index + 1;

    /* Sort attributes by buffer binding and index in ascending order */
    std::sort(
        attribs_.begin(),
        attribs_.end(),
        [](const GLVertexAttribute& lhs, const GLVertexAttribute& rhs)
        {
            if (lhs.buffer < rhs.buffer)
                return true;
            if (lhs.buffer > rhs.buffer)
                return false;
            return (lhs.index < rhs.index);
        }
    );
}

void GL2XSharedContextVertexArray::Bind(GLStateManager& stateMngr)
{
    /* Enable required vertex arrays */
    for (const GLVertexAttribute& attr : attribs_)
    {
        stateMngr.BindBuffer(GLBufferTarget::ArrayBuffer, attr.buffer);
        glVertexAttribPointer(attr.index, attr.size, attr.type, attr.normalized, attr.stride, reinterpret_cast<const GLvoid*>(attr.offsetPtrSized));
        glEnableVertexAttribArray(attr.index);
    }

    //TODO: add case for disabling attrib arrays inbetween, e.g. when only index 0 and 2 is used (rare case probably)
    /* Disable remaining vertex arrays */
    stateMngr.DisableVertexAttribArrays(attribIndexEnd_);
}

void GL2XSharedContextVertexArray::SetDebugName(const char* name)
{
    // dummy (only implemented for GL 3+)
}


} // /namespace LLGL



// ================================================================================
