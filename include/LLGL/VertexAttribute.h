/*
 * VertexAttribute.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_VERTEX_ATTRIBUTE_H__
#define __LLGL_VERTEX_ATTRIBUTE_H__


#include "RenderSystemFlags.h"
#include <string>


namespace LLGL
{


//! Vertex attribute class.
struct VertexAttribute
{
    //! Data type of the vertex attribute components. By default DataType::Float.
    DataType        dataType    = DataType::Float;

    //! Specifies whether non-floating-point data types are to be converted to floating-points. By default false.
    bool            conversion  = false;

    //! Number of components: 1, 2, 3, or 4. By default 4.
    unsigned int    components  = 4;

    //! Byte offset for within each vertex. By default 0.
    unsigned int    offset      = 0;

    //! Vertex attribute name (for GLSL) or semantic name (for HLSL).
    std::string     name;
};


} // /namespace LLGL


#endif



// ================================================================================
