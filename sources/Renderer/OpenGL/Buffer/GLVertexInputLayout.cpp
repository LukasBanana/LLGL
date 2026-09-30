/*
 * GLVertexInputLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLVertexInputLayout.h"
#include "../../../Core/MacroUtils.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


void GLVertexInputLayout::Reset()
{
    attribs_.clear();
}

void GLVertexInputLayout::SetAttribs(ArrayView<GLVertexAttribute> attributes)
{
    attribs_ = std::vector<GLVertexAttribute>(attributes.begin(), attributes.end());
}

void GLVertexInputLayout::SetAttribs(ArrayView<VertexAttribute> attributes)
{
    /* Convert to GLVertexAttribute */
    attribs_.resize(attributes.size());
    for_range(i, attributes.size())
        GLConvertVertexAttrib(attribs_[i], attributes[i], attributes[i].slot);
}

int GLVertexInputLayout::CompareSWO(const GLVertexInputLayout& lhs, const GLVertexInputLayout& rhs)
{
    LLGL_COMPARE_SEPARATE_MEMBERS_SWO( lhs.attribs_.size(), rhs.attribs_.size() );
    for_range(i, lhs.attribs_.size())
    {
        int cmpAttrib = LLGL::CompareSWO(lhs.attribs_[i], rhs.attribs_[i]);
        if (cmpAttrib != 0)
            return cmpAttrib;
    }
    return 0;
}


} // /namespace LLGL



// ================================================================================
