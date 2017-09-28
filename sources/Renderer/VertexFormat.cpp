/*
 * VertexFormat.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
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

void VertexFormat::AppendAttribute(const VertexAttribute& attrib, std::uint32_t offset)
{
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
}


} // /namespace LLGL



// ================================================================================
