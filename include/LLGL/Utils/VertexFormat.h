/*
 * VertexFormat.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VERTEX_FORMAT_H
#define LLGL_VERTEX_FORMAT_H


#include <LLGL/Export.h>
#include <LLGL/Constants.h>
#include <LLGL/VertexAttribute.h>
#include <vector>
#include <cstdint>
#include <algorithm>


namespace LLGL
{


/**
\brief Utility structure to store vertex attributes.
\remarks A vertex format is required to describe how the vertex attributes are layed out inside a vertex buffer.
\see BufferDescriptor::VertexBuffer::format
*/
struct VertexFormat
{
    /**
    \brief Appends the specified vertex attribute to this vertex format.
    \param[in] attrib Specifies the new attribute which is appended to this vertex format.
    \remarks This function modifies the \c offset member of specified attribute before adding it to the \c attributes list,
    and sets the \c stride member to the sum of the size of all attributes.
    \see VertexAttribute::offset
    */
    inline void AppendAttribute(const VertexAttribute& attrib)
    {
        /* Append attribute to the list */
        attributes.push_back(attrib);
        auto& last = attributes.back();

        /* Overwrite attribute location */
        if (attributes.size() > 1)
        {
            /* Set location and offset after previous attribute */
            const auto& prev = attributes[attributes.size() - 2];
            last.location   = prev.location + 1;
            last.offset     = prev.offset + prev.GetSize();
        }
        else
        {
            last.location   = 0;
            last.offset     = 0;
        }

        /* Update stride for each attribute */
        std::uint32_t stride = 0;

        for (const auto& attr : attributes)
            stride = (std::max)(stride, attr.offset + attr.GetSize());

        SetStride(stride);
    }

    /**
    \brief Returns the stride (in bytes) of the first vertex.
    \remarks It is expected that all vertices with the same buffer binding slot have the same stride.
    \see VertexAttribute::GetSize
    */
    inline std::uint32_t GetStride() const
    {
        return (attributes.empty() ? 0 : attributes.front().stride);
    }

    /**
    \brief Returns the stride (in bytes) of the first vertex with the specified buffer binding slot.
    \remarks It is expected that all vertices with the same buffer binding slot have the same stride.
    \see VertexAttribute::GetSize
    */
    inline std::uint32_t GetStride(std::uint32_t slot) const
    {
        for (const auto& attr : attributes)
        {
            if (attr.slot == slot)
                return attr.stride;
        }
        return 0;
    }

    /**
    \brief Set the \c stride member for all vertex attributes to the specified value.
    \see VertexAttribute::stride
    */
    inline void SetStride(std::uint32_t stride)
    {
        for (auto& attr : attributes)
            attr.stride = stride;
    }

    /**
    \brief Set the \c stride member for all vertex attributes with the specified buffer binding slot to the new value specified by \c stride.
    \see VertexAttribute::stride
    */
    inline void SetStride(std::uint32_t stride, std::uint32_t slot)
    {
        for (auto& attr : attributes)
        {
            if (attr.slot == slot)
                attr.stride = stride;
        }
    }

    /**
    \brief Sets the \c slot member for all vertex attributes to the specified value.
    \see VertexAttribute::slot
    */
    inline void SetSlot(std::uint32_t slot)
    {
        for (auto& attr : attributes)
            attr.slot = slot;
    }

    /**
    \brief Specifies the list of vertex attributes.
    \see AppendAttribute
    */
    std::vector<VertexAttribute> attributes;
};


} // /namespace LLGL


#endif



// ================================================================================
