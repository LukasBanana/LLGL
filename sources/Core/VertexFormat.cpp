/*
 * VertexFormat.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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
    vertexFormat.stride = 0;
    for (const auto& attr : vertexFormat.attributes)
        vertexFormat.stride = std::max(vertexFormat.stride, attr.offset + attr.GetSize());
}

void VertexFormat::AppendAttribute(const VertexAttribute& attrib, unsigned int offset)
{
    /* Validate input arguments */
    if (attrib.components < 1 || attrib.components > 4)
    {
        std::stringstream s;
        s << __FUNCTION__ << ": 'attrib.components' must be 1, 2, 3, or 4 (but " << attrib.components << " is specified)";
        throw std::invalid_argument(s.str());
    }

    /* Append attribute to the list */
    attributes.push_back(attrib);

    /* Overwrite attribute offset */
    auto& attr = attributes.back();

    if (offset == OffsetAppend)
    {
        /* Set offset after the previous attribute */
        if (attributes.size() > 1)
        {
            const auto& prevAttr = attributes[attributes.size() - 2];
            attr.offset = prevAttr.offset + prevAttr.GetSize();
        }
        else
            attr.offset = 0;

        /* Increase stride */
        stride += attr.GetSize();
    }
    else
    {
        /* Set custom offset */
        attr.offset = offset;

        /* Update stride */
        UpdateStride(*this);
    }

    /* Overwrite attribute input slot */
    if (attributes.size() > 1)
    {
        /* Check if input slot must be increased */
        const auto& prevAttr = attributes[attributes.size() - 2];
        attr.inputSlot = prevAttr.inputSlot;
        if (attr.offset < prevAttr.offset + prevAttr.GetSize())
            ++attr.inputSlot;
    }
    else
        attr.inputSlot = 0;
}

void VertexFormat::AppendAttributes(const VertexFormat& vertexFormat)
{
    for (const auto& attr : vertexFormat.attributes)
        AppendAttribute(attr, attr.offset);
}


} // /namespace LLGL



// ================================================================================
