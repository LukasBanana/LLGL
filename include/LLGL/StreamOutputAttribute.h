/*
 * StreamOutputAttribute.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_STREAM_OUTPUT_ATTRIBUTE_H
#define LLGL_STREAM_OUTPUT_ATTRIBUTE_H


#include "Export.h"
#include "Format.h"
#include <string>


namespace LLGL
{


//! Stream-output attribute structure.
struct LLGL_EXPORT StreamOutputAttribute
{
    StreamOutputAttribute() = default;
    StreamOutputAttribute(const StreamOutputAttribute&) = default;
    StreamOutputAttribute& operator = (const StreamOutputAttribute&) = default;

    //! Vertex attribute name (for GLSL) or semantic name (for HLSL).
    std::string     name;

    //! Zero-based stream number. By default 0.
    unsigned int    stream          = 0;

    //! Start vector component index, which is to be written. Must be 0, 1, 2, or 3. By default 0.
    unsigned char   startComponent  = 0;

    /**
    \brief Number of vector components, which are to be written. Must be 1, 2, 3, or 4.
    \remarks The number of components plus the start component index (see 'startComponent') must not be larger than 4.
    \see startComponent
    */
    unsigned char   components      = 4;

    /**
    \brief Semantic index.
    \note Only supported with: Direct3D 11, Direct3D 12.
    */
    unsigned int    semanticIndex   = 0;

    /**
    \brief Stream-output buffer output slot.
    \remarks This is used when multiple stream-output buffers are used simultaneously.
    */
    unsigned char   outputSlot      = 0;
};


LLGL_EXPORT bool operator == (const StreamOutputAttribute& lhs, const StreamOutputAttribute& rhs);
LLGL_EXPORT bool operator != (const StreamOutputAttribute& lhs, const StreamOutputAttribute& rhs);


} // /namespace LLGL


#endif



// ================================================================================
