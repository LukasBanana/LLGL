/*
 * VertexFormat.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/VertexFormat.h>
#include <LLGL/RenderSystemFlags.h>
#include <stdexcept>
#include <sstream>


namespace LLGL
{


static void UpdateStride(VertexFormat& vertexFormat)
{
    /* Update stride for each attribute */
    std::uint32_t stride = 0;

    for (const auto& attr : vertexFormat.attributes)
        stride = std::max(stride, attr.offset + attr.GetSize());

    vertexFormat.SetStride(stride);
}

void VertexFormat::AppendAttribute(
    const VertexAttribute&  attrib,
    bool                    customLocation,
    std::uint32_t           customOffset)
{
    /* Append attribute to the list */
    attributes.push_back(attrib);
    auto& attr = attributes.back();

    /* Overwrite attribute location */
    if (!customLocation && attr.location == 0)
    {
        if (attributes.size() > 1)
        {
            const auto& prevAttr = attributes[attributes.size() - 2];
            attr.location = prevAttr.location + 1;
        }
        else
            attr.location = 0;
    }

    /* Overwrite attribute offset */
    if (customOffset == Constants::ignoreOffset)
    {
        /* Set offset after the previous attribute */
        if (attributes.size() > 1)
        {
            const auto& prevAttr = attributes[attributes.size() - 2];
            attr.offset = prevAttr.offset + prevAttr.GetSize();
        }
        else
            attr.offset = 0;
    }
    else
    {
        /* Set custom offset */
        attr.offset = customOffset;
    }

    /* Update vertex stride */
    UpdateStride(*this);
}

std::uint32_t VertexFormat::GetStride() const
{
    if (attributes.empty())
        return 0;
    else
        return attributes.front().stride;
}

std::uint32_t VertexFormat::GetStride(std::uint32_t slot) const
{
    for (const auto& attr : attributes)
    {
        if (attr.slot == slot)
            return attr.stride;
    }
    return 0;
}

void VertexFormat::SetStride(std::uint32_t stride)
{
    for (auto& attr : attributes)
        attr.stride = stride;
}

void VertexFormat::SetStride(std::uint32_t stride, std::uint32_t slot)
{
    for (auto& attr : attributes)
    {
        if (attr.slot == slot)
            attr.stride = stride;
    }
}

void VertexFormat::SetSlot(std::uint32_t slot)
{
    for (auto& attr : attributes)
        attr.slot = slot;
}


} // /namespace LLGL



// ================================================================================
