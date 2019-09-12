/*
 * VertexFormat.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VERTEX_FORMAT_H
#define LLGL_VERTEX_FORMAT_H


#include "Export.h"
#include "Constants.h"
#include "VertexAttribute.h"
#include <vector>
#include <cstdint>


namespace LLGL
{


/**
\brief Vertex format structure.
\remarks A vertex format is required to describe how the vertex attributes are layed out inside a vertex buffer.
\see BufferDescriptor::VertexBuffer::format
\see ShaderProgramDescriptor::vertexFormats
\todo Move this to \c Utility.h header.
*/
struct LLGL_EXPORT VertexFormat
{
    /**
    \brief Appends the specified vertex attribute to this vertex format.
    \param[in] attrib Specifies the new attribute which is appended to this vertex format.
    \param[in] customLocation Specifies whether the attribute location is to be adopted.
    Otherwise, the the location will be set to the previous attribute's location plus one.
    \param[in] customOffset Specifies the optional offset (in bytes) for this attribute.
    If this is equal to <code>Constants::ignoreOffset</code>, the offset is determined by the previous vertex attribute offset plus its size.
    If there is no previous vertex attribute, the determined offset is 0. By default <code>Constants::ignoreOffset</code>.
    \remarks This function modifies the \c offset member of specified attribute before adding it to the \c attributes list,
    and sets the \c stride member to the sum of the size of all attributes.
    \see VertexAttribute::offset
    \see Constants::ignoreOffset
    */
    void AppendAttribute(
        const VertexAttribute&  attrib,
        bool                    customLocation  = false,
        std::uint32_t           customOffset    = Constants::ignoreOffset
    );

    /**
    \brief Returns the stride (in bytes) of the first vertex.
    \remarks It is expected that all vertices with the same buffer binding slot have the same stride.
    \see VertexAttribute::GetSize
    */
    std::uint32_t GetStride() const;

    /**
    \brief Returns the stride (in bytes) of the first vertex with the specified buffer binding slot.
    \remarks It is expected that all vertices with the same buffer binding slot have the same stride.
    \see VertexAttribute::GetSize
    */
    std::uint32_t GetStride(std::uint32_t slot) const;

    /**
    \brief Set the \c stride member for all vertex attributes to the specified value.
    \see VertexAttribute::stride
    */
    void SetStride(std::uint32_t stride);

    /**
    \brief Set the \c stride member for all vertex attributes with the specified buffer binding slot to the new value specified by \c stride.
    \see VertexAttribute::stride
    */
    void SetStride(std::uint32_t stride, std::uint32_t slot);

    /**
    \brief Sets the \c slot member for all vertex attributes to the specified value.
    \see VertexAttribute::slot
    */
    void SetSlot(std::uint32_t slot);

    /**
    \brief Specifies the list of vertex attributes.
    \see AppendAttribute
    */
    std::vector<VertexAttribute> attributes;
};


} // /namespace LLGL


#endif



// ================================================================================
