/*
 * VertexFormat.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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
\remarks A vertex format is required to describe how the vertex attributes are supported inside a vertex buffer.
\see BufferDescriptor::VertexBuffer::format
\see ShaderProgram::BuildInputLayout
*/
struct LLGL_EXPORT VertexFormat
{
    /**
    \brief Appends the specified vertex attribute to this vertex format.
    \param[in] attrib Specifies the new attribute which is appended to this vertex format.
    \param[in] offset Specifies the optional offset (in bytes) for this attribute.
    If this is 'Constants::ignoreOffset', the offset is determined by the previous vertex attribute offset plus its size.
    If there is no previous vertex attribute, the determined offset is 0. By default Constants::ignoreOffset.
    \remarks This function will always overwrite the 'offset' and 'inputSlot' members before the attribute is appended to this vertex format.
    The 'inputSlot' member will be set to the input slot value of the previous vertex attribute and is increased by one,
    if the new offset of the new vertex attribute is less than the offset plus size of the previous vertex attribute.
    \throws std::invalid_argument If 'attrib.components' is neither 1, 2, 3, nor 4.
    \see VertexAttribute::offset
    \see VertexAttribute::inputSlot
    \see Constants::ignoreOffset
    */
    void AppendAttribute(const VertexAttribute& attrib, std::uint32_t offset = Constants::ignoreOffset);

    /**
    \brief Specifies the list of vertex attributes.
    \remarks Use "AppendAttribute" or "AppendAttributes" to append new attributes.
    */
    std::vector<VertexAttribute>    attributes;

    /**
    \brief Specifies the vertex data stride (or format size) which describes the byte offset between consecutive vertices.
    \remarks This is updated automatically everytime "AppendAttribute" or "AppendAttributes" is called,
    but it can also be modified manually. It is commonly the size of all vertex attributes.
    */
    std::uint32_t                   stride      = 0;

    /**
    \brief Vertex buffer input slot. By default 0.
    \remarks This is used when multiple vertex buffers are used simultaneously.
    \note Only supported with: Direct3D 11, Direct3D 12, Vulkan.
    \note For OpenGL, the input slots are automatically generated in ascending order and beginning with zero.
    */
    std::uint32_t                   inputSlot   = 0;
};


} // /namespace LLGL


#endif



// ================================================================================
