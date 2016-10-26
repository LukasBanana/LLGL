/*
 * StreamOutputFormat.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_STREAM_OUTPUT_FORMAT_H
#define LLGL_STREAM_OUTPUT_FORMAT_H


#include "Export.h"
#include "StreamOutputAttribute.h"
#include <vector>


namespace LLGL
{


/**
\brief Stream-output format descriptor structure.
\remarks A vertex format is required to describe how the vertex attributes are supported inside a vertex buffer.
*/
struct LLGL_EXPORT StreamOutputFormat
{
    /**
    \brief Appends the specified stream-output attribute to this stream-output format.
    \param[in] attrib Specifies the new attribute which is appended to this stream-output format.
    */
    void AppendAttribute(const StreamOutputAttribute& attrib);

    /**
    \brief Append all attributes of the specified stream-output format.
    \remarks This can be used to build a stream-output format for stream-output buffer arrays.
    */
    void AppendAttributes(const StreamOutputFormat& format);

    /**
    \brief Specifies the list of vertex attributes.
    \remarks Use "AppendAttribute" or "AppendAttributes" to append new attributes.
    */
    std::vector<StreamOutputAttribute>  attributes;

    #if 0
    /**
    \brief Specifies the stream-output data stride (or format size) which describes the byte offset between consecutive output vertices.
    \remarks This is updated automatically evertime "AppendAttribute" or "AppendAttributes" is called,
    but it can also modified manually. It is commonly the size of all stream-output attributes.
    */
    unsigned int                        stride = 0;
    #endif
};


} // /namespace LLGL


#endif



// ================================================================================
