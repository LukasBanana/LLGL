/*
 * VertexFormat.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VERTEX_FORMAT_H
#define LLGL_VERTEX_FORMAT_H


#include "Export.h"
#include "VertexAttribute.h"
#include <vector>


namespace LLGL
{


/**
\brief Vertex format descriptor structure.
\remarks A vertex format is required to describe how the vertex attributes are supported inside a vertex buffer.
*/
struct LLGL_EXPORT VertexFormat
{
    /**
    \brief Offset value to determine the offset automatically, so that a vertex attribute is appended at the end of a vertex format.
    \see AppendAttribute
    */
    static const unsigned int OffsetAppend = ~0;

    /**
    \brief Appends the specified vertex attribute to this vertex format.
    \param[in] attrib Specifies the new attribute which is appended to this vertex format.
    \param[in] offset Specifies the optional offset (in bytes) for this attribute.
    If this is 'OffsetAppend', the offset is determined by the previous vertex attribute offset plus its size.
    If there is no previous vertex attribute, the determined offset is 0. By default OffsetAppend.
    \remarks This function will always overwrite the 'offset' and 'inputSlot' members before the attribute is appended to this vertex format.
    The 'inputSlot' member will be set to the input slot value of the previous vertex attribute and is increased by one,
    if the new offset of the new vertex attribute is less than the offset plus size of the previous vertex attribute.
    \throws std::invalid_argument If 'attrib.components' is neither 1, 2, 3, nor 4.
    \see VertexAttribute::offset
    \see VertexAttribute::inputSlot
    */
    void AppendAttribute(const VertexAttribute& attrib, unsigned int offset = OffsetAppend);

    /**
    \brief Append all attributes of the specified vertex format.
    \remarks This can be used to build a vertex format for vertex buffer arrays.
    */
    void AppendAttributes(const VertexFormat& format);

    /**
    \brief Specifies the list of vertex attributes.
    \remarks Use "AppendAttribute" or "AppendAttributes" to append new attributes.
    */
    std::vector<VertexAttribute>    attributes;

    /**
    \brief Specifies the vertex data stride (or format size) which describes the byte offset between consecutive vertices.
    \remarks This is updated automatically evertime "AppendAttribute" or "AppendAttributes" is called,
    but it can also modified manually. It is commonly the size of all vertex attributes.
    */
    unsigned int                    stride = 0;
};


} // /namespace LLGL


#endif



// ================================================================================
