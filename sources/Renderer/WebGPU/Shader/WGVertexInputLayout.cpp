/*
 * WGVertexInputLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGVertexInputLayout.h"
#include "../WGTypes.h"
//#include "../../../Core/Assertion.h"
//#include "../../../Core/StringUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


WGVertexInputLayout::WGVertexInputLayout(std::vector<VertexAttribute> attribs, Report* outReport)
{
    (void)BuildVertexBufferLayouts(attribs, outReport);
}

static void ConvertWebGPUVertexAttrib(WGPUVertexAttribute& dst, const VertexAttribute& src)
{
    dst.nextInChain     = nullptr;
    dst.format          = WGTypes::ToWGVertexFormat(src.format);
    dst.offset          = src.offset;
    dst.shaderLocation  = src.location;
}

bool WGVertexInputLayout::BuildVertexBufferLayouts(std::vector<VertexAttribute> attribs, Report* outReport)
{
    bool validationFailed = false;

    /* Sort vertex attributes so their buffer slots are consecutive */
    std::sort(
        attribs.begin(),
        attribs.end(),
        [](const VertexAttribute& lhs, const VertexAttribute& rhs) -> bool
        {
            return (lhs.slot < rhs.slot);
        }
    );

    /* Convert all vertex attributes to WebGPU types */
    vertexAttribs_.resize(attribs.size());
    std::uint32_t numAttribsPerLayout = 0;

    for_range(i, static_cast<std::uint32_t>(attribs.size()))
    {
        ConvertWebGPUVertexAttrib(vertexAttribs_[i], attribs[i]);
        ++numAttribsPerLayout;

        /* When we reached the end or if the slot for attributes has changed, we record a new buffer layout */
        if (i + 1 == for_range_end(i) || (attribs[i].slot != attribs[i + 1].slot))
        {
            const std::uint32_t firstVertexAttrib       = i + 1 - numAttribsPerLayout;
            const std::uint32_t expectedBufferSlot      = static_cast<std::uint32_t>(vertexBufferLayouts_.size());
            const std::uint32_t expectedAttribStride    = attribs[firstVertexAttrib].stride;
            const std::uint32_t expectedInstanceDivisor = attribs[firstVertexAttrib].instanceDivisor;

            /* Validate input attributes */
            for_range(j, numAttribsPerLayout)
            {
                const VertexAttribute& attr = attribs[firstVertexAttrib + j];
                if (attr.instanceDivisor > 1)
                {
                    if (outReport != nullptr)
                    {
                        outReport->Errorf(
                            "unsupported instance divisor in vertex attribute '%s' (%u); must be 0 or 1 for WebGPU",
                            attr.name.c_str(), attr.instanceDivisor
                        );
                    }
                    validationFailed = true;
                }
                if (j > 0 && attr.instanceDivisor != expectedInstanceDivisor)
                {
                    if (outReport != nullptr)
                    {
                        outReport->Errorf(
                            "mismatch between expected vertex instance divisor (%u) and vertex attribute '%s' instance divisor (%u)",
                            expectedInstanceDivisor, attr.name.c_str(), attr.instanceDivisor
                        );
                    }
                    validationFailed = true;
                }
                if (j > 0 && attr.stride != expectedAttribStride)
                {
                    if (outReport != nullptr)
                    {
                        outReport->Errorf(
                            "mismatch between expected vertex stride (%u) and vertex attribute '%s' stride (%u)",
                            expectedAttribStride, attr.name.c_str(), attr.stride
                        );
                    }
                    validationFailed = true;
                }
                if (attr.slot != expectedBufferSlot)
                {
                    if (outReport != nullptr)
                    {
                        outReport->Errorf(
                            "mismatch between expected vertex buffer slot (%u) and vertex attribute '%s' slot (%u)",
                            expectedBufferSlot, attr.name.c_str(), attr.slot
                        );
                    }
                    validationFailed = true;
                }
            }

            /* Add new vertex buffer layout */
            WGPUVertexBufferLayout vbufferLayout;
            {
                vbufferLayout.nextInChain       = nullptr;
                vbufferLayout.stepMode          = (attribs[i].instanceDivisor > 0 ? WGPUVertexStepMode_Instance : WGPUVertexStepMode_Vertex);
                vbufferLayout.arrayStride       = attribs[i].stride;
                vbufferLayout.attributeCount    = numAttribsPerLayout;
                vbufferLayout.attributes        = &(vertexAttribs_[firstVertexAttrib]);
            }
            vertexBufferLayouts_.push_back(vbufferLayout);
            numAttribsPerLayout = 0;
        }
    }

    return !validationFailed;
}


} // /namespace LLGL



// ================================================================================
